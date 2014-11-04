/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#include "maidsafe/common/tools/network_viewer.h"

#include <csignal>

#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>

#include "boost/interprocess/ipc/message_queue.hpp"

#include "maidsafe/common/utils.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"

#include "maidsafe/common/serialisation.h"

namespace bi = boost::interprocess;

namespace maidsafe {

namespace network_viewer {

const std::string kMessageQueueName("matrix_messages");

namespace {

std::function<void(int /*state_id*/)> g_functor;
std::mutex g_mutex;
std::condition_variable g_cond_var;
bool g_stop(false);
std::thread g_thread;
int g_state_id(0);
std::chrono::milliseconds g_notify_interval(1000);
const size_t kMaxSnapshotCount(1000);
bool g_last_notified_state_current(true);

struct NodeInfo;

typedef std::set<NodeInfo, std::function<bool(const NodeInfo&, const NodeInfo&)>> NodeSet;
typedef std::map<NodeSet::iterator, ChildType,
                 std::function<bool(NodeSet::iterator, NodeSet::iterator)>> MatrixMap;

struct NodeInfo {
  explicit NodeInfo(const NodeId& id_in)
      : id(id_in),
        matrix(new MatrixMap([id_in](NodeSet::iterator lhs, NodeSet::iterator rhs) {
          return NodeId::CloserToTarget(lhs->id, rhs->id, id_in);
        })) {}
  NodeInfo(const NodeInfo& other) : id(other.id), matrix(other.matrix) {}
  NodeInfo(NodeInfo&& other) : id(std::move(other.id)), matrix(std::move(other.matrix)) {}
  NodeInfo& operator=(NodeInfo other) {
    using std::swap;
    swap(id, other.id);
    swap(matrix, other.matrix);
    return *this;
  }
  NodeId id;
  mutable std::shared_ptr<MatrixMap> matrix;
};

NodeSet g_nodes([](const NodeInfo& lhs, const NodeInfo& rhs) { return lhs.id < rhs.id; });

std::map<int, NodeSet> g_snapshots;

void PrintDetails(const NodeInfo& node_info) {
  static int count(0);
  std::string printout(std::to_string(count++));
  printout += "\tReceived: Owner: " + DebugId(node_info.id) + "\n";
  for (const auto& node : *node_info.matrix) {
    switch (node.second) {
      case ChildType::kGroup:
        printout += "\t\t" + DebugId(node.first->id) + ": kGroup\n";
        break;
      case ChildType::kClosest:
        printout += "\t\t" + DebugId(node.first->id) + ": kClosest\n";
        break;
      case ChildType::kMatrix:
        printout += "\t\t" + DebugId(node.first->id) + ": kMatrix\n";
        break;
      case ChildType::kNotConnected:
        printout += "\t\t" + DebugId(node.first->id) + ": kNotConnected\n";
        break;
      default:
        assert(false);
    }
  }
  LOG(kInfo) << printout << '\n';
}

MatrixMap::iterator FindInMatrix(const NodeInfo& node_info, MatrixMap& matrix) {
  NodeId node_id(node_info.id);
  return std::find_if(
      std::begin(matrix), std::end(matrix),
      [node_id](const MatrixMap::value_type & child) { return child.first->id == node_id; });
}

void InsertNode(const MatrixRecord& matrix_record) {
  // Insert or find node
  auto itr(g_nodes.insert(NodeInfo(matrix_record.owner_id())).first);
  itr->matrix->clear();
  for (const auto& child : matrix_record.matrix_ids()) {
    // If matrix entry doesn't already exist in this node's matrix, add entry or find entry in
    // global set, then add it to this node's matrix.
    NodeInfo matrix_entry(child.first);
    if (matrix_entry.id == itr->id)
      continue;
    auto matrix_itr(FindInMatrix(matrix_entry, *itr->matrix));
    if (matrix_itr != std::end(*itr->matrix))
      continue;
    auto global_itr(g_nodes.insert(matrix_entry).first);
    //    global_itr->matrix.insert(itr);
    itr->matrix->insert(std::make_pair(global_itr, child.second));
  }
  PrintDetails(*itr);
}

void TakeSnapshotAndNotify() {
  static std::chrono::steady_clock::time_point last_notified(std::chrono::steady_clock::now());

  if (g_last_notified_state_current || !g_functor ||
      std::chrono::steady_clock::now() - last_notified < g_notify_interval) {
    return;
  }

  auto& snapshot(g_snapshots.insert(std::make_pair(++g_state_id, NodeSet([](const NodeInfo & lhs,
                                                                            const NodeInfo & rhs) {
                                                                   return lhs.id < rhs.id;
                                                                 }))).first->second);
  // TODO(Fraser#5#): 2013-04-10 - Optimise this
  // Construct deep copy of g_nodes using NodeInfo with empty matrix.
  for (const auto& node : g_nodes)
    snapshot.insert(NodeInfo(node.id));
  // Apply the links in each entry's matrix
  auto main_itr(std::begin(g_nodes));
  auto snapshot_itr(std::begin(snapshot));
  while (main_itr != std::end(g_nodes)) {
    for (const auto& child : *main_itr->matrix) {
      // Find the child in the new NodeSet
      auto g_itr(std::find_if(
          std::begin(snapshot), std::end(snapshot),
          [&child](const NodeInfo & node_info) { return child.first->id == node_info.id; }));
      snapshot_itr->matrix->insert(std::make_pair(g_itr, child.second));
    }
    ++main_itr;
    ++snapshot_itr;
  }

  if (g_snapshots.size() > kMaxSnapshotCount)
    g_snapshots.erase(std::begin(g_snapshots));

  g_functor(g_state_id);
  last_notified = std::chrono::steady_clock::now();
  g_last_notified_state_current = true;
  LOG(kInfo) << "Increased state version to " << g_state_id << '\n';
}

void UpdateNodeInfo(const std::string& serialised_matrix_record) {
  MatrixRecord matrix_record(serialised_matrix_record);
  if (matrix_record.matrix_ids().empty())
    g_nodes.erase(NodeInfo(matrix_record.owner_id()));
  else
    InsertNode(matrix_record);

  g_last_notified_state_current = false;
  TakeSnapshotAndNotify();
}

}  // unnamed namespace

ViewableNode::ViewableNode() : id(), distance(), type(static_cast<ChildType>(-1)) {}

ViewableNode::ViewableNode(std::string id_in, std::string distance_in, ChildType type_in)
    : id(std::move(id_in)), distance(std::move(distance_in)), type(type_in) {
  assert(id.size() == 2 * NodeId::kSize);
  assert(distance.size() == 2 * NodeId::kSize);
}

ViewableNode::ViewableNode(const ViewableNode& other)
    : id(other.id), distance(other.distance), type(other.type) {}

ViewableNode::ViewableNode(ViewableNode&& other)
    : id(std::move(other.id)), distance(std::move(other.distance)), type(std::move(other.type)) {}

ViewableNode& ViewableNode::operator=(ViewableNode other) {
  using std::swap;
  swap(id, other.id);
  swap(distance, other.distance);
  swap(type, other.type);
  return *this;
}

MatrixRecord::MatrixRecord()
    : owner_id_(),
      matrix_ids_([](const NodeId&, const NodeId&)->bool {
        // Ensure this is never actually invoked
        assert(false);
        return true;
      }) {}

MatrixRecord::MatrixRecord(const NodeId& owner_id)
    : owner_id_(owner_id),
      matrix_ids_([owner_id](const NodeId & lhs, const NodeId & rhs) {
        return NodeId::CloserToTarget(lhs, rhs, owner_id);
      }) {}

MatrixRecord::MatrixRecord(const std::string& serialised_matrix_record)
    : owner_id_(), matrix_ids_([](const NodeId&, const NodeId&) { return true; }) {
  try { ConvertFromString(serialised_matrix_record, *this); }
  catch(...) {
    LOG(kError) << "Failed to construct matrix_record.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
}

std::string MatrixRecord::Serialise() const {
  return ConvertToString(*this);
}

MatrixRecord::MatrixRecord(const MatrixRecord& other)
    : owner_id_(other.owner_id_), matrix_ids_(other.matrix_ids_) {}

MatrixRecord::MatrixRecord(MatrixRecord&& other)
    : owner_id_(std::move(other.owner_id_)), matrix_ids_(std::move(other.matrix_ids_)) {}

MatrixRecord& MatrixRecord::operator=(MatrixRecord other) {
  using std::swap;
  swap(owner_id_, other.owner_id_);
  swap(matrix_ids_, other.matrix_ids_);
  return *this;
}

NodeId MatrixRecord::owner_id() const { return owner_id_; }

MatrixRecord::MatrixIds MatrixRecord::matrix_ids() const { return matrix_ids_; }

void SetUpdateFunctor(std::function<void(int /*state_id*/)> functor) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_functor = functor;
}

std::vector<std::string> GetNodesInNetwork(int state_id) {
  LOG(kInfo) << "Handling GetNodesInNetwork request for state " << state_id << '\n';
  std::vector<std::string> hex_encoded_ids;
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_snapshots.empty())
    return hex_encoded_ids;

  auto snapshot_itr(g_snapshots.find(state_id));
  if (snapshot_itr == std::end(g_snapshots))
    --snapshot_itr;
  hex_encoded_ids.reserve(snapshot_itr->second.size());
  for (const auto& node : snapshot_itr->second)
    hex_encoded_ids.push_back(node.id.ToStringEncoded(NodeId::EncodingType::kHex));

  TakeSnapshotAndNotify();
  return hex_encoded_ids;
}

std::vector<ViewableNode> GetCloseNodes(int state_id, const std::string& hex_encoded_id) {
  LOG(kInfo) << "Handling GetCloseNodes request for "
             << DebugId(NodeId(hex_encoded_id, NodeId::EncodingType::kHex)) << " at state "
             << state_id;
  std::vector<ViewableNode> children;
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_snapshots.empty())
    return children;

  auto snapshot_itr(g_snapshots.find(state_id));
  if (snapshot_itr == std::end(g_snapshots))
    --snapshot_itr;

  NodeId target_id(hex_encoded_id, NodeId::EncodingType::kHex);
  auto itr(
      std::find_if(std::begin(snapshot_itr->second), std::end(snapshot_itr->second),
                   [&target_id](const NodeInfo & node_info) { return target_id == node_info.id; }));

  if (itr == std::end(snapshot_itr->second)) {
    // Data / account request
    for (const auto& node : snapshot_itr->second) {
      bool needs_sorted(false);
      if (children.size() < 4) {
        children.emplace_back(node.id.ToStringEncoded(NodeId::EncodingType::kHex),
                              (target_id ^ node.id).ToStringEncoded(NodeId::EncodingType::kHex),
                              ChildType::kNotConnected);
        if (children.size() == 4)
          needs_sorted = true;
      } else if (NodeId::CloserToTarget(node.id, NodeId(children[3].id, NodeId::EncodingType::kHex),
                                        target_id)) {
        children[3] =
            ViewableNode(node.id.ToStringEncoded(NodeId::EncodingType::kHex),
                         (target_id ^ node.id).ToStringEncoded(NodeId::EncodingType::kHex),
                         ChildType::kNotConnected);
        needs_sorted = true;
      }
      if (needs_sorted) {
        std::sort(std::begin(children), std::end(children),
                  [&target_id](const ViewableNode & lhs, const ViewableNode & rhs) {
          return NodeId::CloserToTarget(NodeId(lhs.id, NodeId::EncodingType::kHex),
                                        NodeId(rhs.id, NodeId::EncodingType::kHex), target_id);
        });
      }
    }
  } else {
    // Node request
    for (const auto& child : *itr->matrix)
      children.emplace_back(
          (*child.first).id.ToStringEncoded(NodeId::EncodingType::kHex),
          (itr->id ^ (*child.first).id).ToStringEncoded(NodeId::EncodingType::kHex), child.second);
  }

  TakeSnapshotAndNotify();
  return children;
}

void SetNotifyInterval(const std::chrono::milliseconds& notify_interval) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_notify_interval = notify_interval;
}

void Run(const std::chrono::milliseconds& notify_interval) {
  g_notify_interval = notify_interval;
  g_thread = std::move(std::thread([&]() {
    try {
      bi::message_queue::remove(kMessageQueueName.c_str());
      on_scope_exit cleanup([]() { bi::message_queue::remove(kMessageQueueName.c_str()); });

      bi::message_queue matrix_messages(bi::create_only, kMessageQueueName.c_str(), 1000, 10000);
      LOG(kSuccess) << "Running...";
      unsigned int priority;
      bi::message_queue::size_type received_size(0);
      char input[10000];
      for (;;) {
        std::string received;
        if (matrix_messages.try_receive(&input[0], 10000, received_size, priority))
          received = std::string(&input[0], received_size);
        std::unique_lock<std::mutex> lock(g_mutex);
        if (received.empty()) {
          if (g_cond_var.wait_for(lock, std::chrono::milliseconds(20), [] { return g_stop; }))
            return;
        } else {
          UpdateNodeInfo(received);
        }
      }
    }
    catch (bi::interprocess_exception& ex) {
      LOG(kError) << ex.what();
    }
  }));
}

void Stop() {
  {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_stop = true;
  }
  g_cond_var.notify_one();
  g_thread.join();
}

}  // namespace network_viewer

}  // namespace maidsafe
