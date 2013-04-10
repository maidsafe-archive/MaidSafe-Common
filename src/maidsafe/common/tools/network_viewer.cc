/* Copyright (c) 2013 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/tools/network_viewer.h"

#include <signal.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <set>

#include "boost/interprocess/ipc/message_queue.hpp"

#include "maidsafe/common/utils.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/tools/network_viewer.pb.h"


namespace bi = boost::interprocess;

namespace maidsafe {

namespace network_viewer {

const std::string kMessageQueueName("matrix_messages");

namespace {

std::function<void(int /*state_id*/)> g_functor;
std::mutex g_mutex;
std::condition_variable g_cond_var;
bool g_ctrlc_pressed(false);
int g_state_id(0);

void SigHandler(int signum) {
  LOG(kInfo) << " Signal received: " << signum;
  {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ctrlc_pressed = true;
  }
  g_cond_var.notify_one();
}

struct NodeInfo;

typedef std::set<NodeInfo, std::function<bool(const NodeInfo&, const NodeInfo&)>> NodeSet;
typedef std::map<NodeSet::iterator, ChildType,
                 std::function<bool(NodeSet::iterator, NodeSet::iterator)>> MatrixMap;

struct NodeInfo {
  explicit NodeInfo(const NodeId& id_in)
      : id(id_in),
        matrix([id_in](NodeSet::iterator lhs, NodeSet::iterator rhs) {
                   return NodeId::CloserToTarget(lhs->id, rhs->id, id_in);
               }) {}
  NodeInfo(const NodeInfo& other) : id(other.id), matrix(other.matrix) {}
  NodeInfo(NodeInfo&& other) : id(std::move(other.id)), matrix(std::move(other.matrix)) {}
  NodeInfo& operator=(NodeInfo other) {
    using std::swap;
    swap(id, other.id);
    swap(matrix, other.matrix);
    return *this;
  }
  NodeId id;
  mutable MatrixMap matrix;
};

NodeSet g_nodes([](const NodeInfo& lhs, const NodeInfo& rhs) { return lhs.id < rhs.id; });

std::map<int, NodeSet> g_snapshots;

}  // unnamed namespace


ViewableNode::ViewableNode() : id(), distance(), type(static_cast<ChildType>(-1)) {}

ViewableNode::ViewableNode(const std::string& id_in,
                           const std::string& distance_in,
                           ChildType type_in)
    : id(id_in),
      distance(distance_in),
      type(type_in) {
  assert(id.size() == 2 * NodeId::kSize);
  assert(distance.size() == 2 * NodeId::kSize);
}

ViewableNode::ViewableNode(const ViewableNode& other)
    : id(other.id),
      distance(other.distance),
      type(other.type) {}

ViewableNode::ViewableNode(ViewableNode&& other)
    : id(std::move(other.id)),
      distance(std::move(other.distance)),
      type(std::move(other.type)) {}

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
      matrix_ids_([owner_id](const NodeId& lhs, const NodeId& rhs) {
                      return NodeId::CloserToTarget(lhs, rhs, owner_id);
                  }) {}

MatrixRecord::MatrixRecord(const std::string& serialised_matrix_record)
    : owner_id_(),
      matrix_ids_([](const NodeId&, const NodeId&) { return true; }) {
  protobuf::MatrixRecord proto_matrix_record;
  if (!proto_matrix_record.ParseFromString(serialised_matrix_record)) {
    LOG(kError) << "Failed to construct matrix_record.";
    ThrowError(CommonErrors::invalid_parameter);
  }

  owner_id_ = NodeId(proto_matrix_record.owner_id());
  matrix_ids_ = MatrixIds([this](const NodeId& lhs, const NodeId& rhs) {
                              return NodeId::CloserToTarget(lhs, rhs, owner_id_);
                          });

  for (int i(0); i != proto_matrix_record.matrix_ids_size(); ++i) {
    AddElement(NodeId(proto_matrix_record.matrix_ids(i).id()),
               static_cast<ChildType>(proto_matrix_record.matrix_ids(i).type()));
  }
}

std::string MatrixRecord::Serialise() const {
  protobuf::MatrixRecord proto_matrix_record;
  proto_matrix_record.set_owner_id(owner_id_.string());
  for (const auto& matrix_id : matrix_ids_) {
    auto proto_element(proto_matrix_record.add_matrix_ids());
    proto_element->set_id(matrix_id.first.string());
    proto_element->set_type(static_cast<int32_t>(matrix_id.second));
  }
  return proto_matrix_record.SerializeAsString();
}

MatrixRecord::MatrixRecord(const MatrixRecord& other)
    : owner_id_(other.owner_id_),
      matrix_ids_(other.matrix_ids_) {}

MatrixRecord::MatrixRecord(MatrixRecord&& other)
    : owner_id_(std::move(other.owner_id_)),
      matrix_ids_(std::move(other.matrix_ids_)) {}

MatrixRecord& MatrixRecord::operator=(MatrixRecord other) {
  using std::swap;
  swap(owner_id_, other.owner_id_);
  swap(matrix_ids_, other.matrix_ids_);
  return *this;
}

void MatrixRecord::AddElement(const NodeId& element_id, ChildType child_type) {
  matrix_ids_[element_id] = child_type;
}

NodeId MatrixRecord::owner_id() const {
  return owner_id_;
}

MatrixRecord::MatrixIds MatrixRecord::matrix_ids() const {
  return matrix_ids_;
}

void PrintDetails(const NodeInfo& node_info) {
  static int count(0);
  std::string printout(std::to_string(count++));
  printout += "\tReceived: Owner: " + DebugId(node_info.id) + "\n";
  for (const auto& node : node_info.matrix)
    printout += "\t\t" + DebugId(node.first->id) + "\n";
  LOG(kInfo) << printout << '\n';
}

MatrixMap::iterator FindInMatrix(const NodeInfo& node_info, MatrixMap& matrix) {
  NodeId node_id(node_info.id);
  return std::find_if(std::begin(matrix),
                      std::end(matrix),
                      [node_id](const MatrixMap::value_type& child) {
                          return child.first->id == node_id;
                      });
}

// void EraseThisNodeFromReferreesMatrix(const NodeInfo& node_info, MatrixMap::value_type referee) {
//   auto referred_entry(FindInMatrix(node_info, referee.first->matrix));
//   assert(referred_entry != std::end(referee.first->matrix));
//   referee.first->matrix.erase(referred_entry);
//   if (referee.first->matrix.empty())
//     g_nodes.erase(referee);
// }

void EraseNode(const NodeInfo& node_info) {
  auto itr(g_nodes.find(node_info));
  if (itr != std::end(g_nodes)) {
//  // Remove from all referees (if referee's matrix becomes empty, erase referee from global set)
//  // then erase this node from global set.
//  for (auto referee : itr->matrix)
//    EraseThisNodeFromReferreesMatrix(*itr, referee);
    g_nodes.erase(itr);
  }
}

void InsertNode(const MatrixRecord& matrix_record) {
  // Insert or find node
  auto itr(g_nodes.insert(NodeInfo(matrix_record.owner_id())).first);
  std::set<NodeId> sent_matrix;
  for (const auto& child : matrix_record.matrix_ids()) {
    // If matrix entry doesn't already exist in this node's matrix, add entry or find entry in
    // global set, then add it to this node's matrix.
    NodeInfo matrix_entry(child.first);
    if (matrix_entry.id == itr->id)
      continue;
    sent_matrix.insert(matrix_entry.id);
    auto matrix_itr(FindInMatrix(matrix_entry, itr->matrix));
    if (matrix_itr != std::end(itr->matrix))
      continue;
    auto global_itr(g_nodes.insert(matrix_entry).first);
//    global_itr->matrix.insert(itr);
    itr->matrix.insert(std::make_pair(global_itr, child.second));
  }
  // Check all pre-existing matrix entries are still entries.  Any that aren't, remove this node
  // from its matrix
  auto referee_itr(std::begin(itr->matrix));
  while (referee_itr != std::end(itr->matrix)) {
    if (sent_matrix.find((*(*referee_itr).first).id) == std::end(sent_matrix)) {
//      EraseThisNodeFromReferreesMatrix(*itr, *referee_itr);
      referee_itr = itr->matrix.erase(referee_itr);
    } else {
      ++referee_itr;
    }
  }
  PrintDetails(*itr);
}

void UpdateNodeInfo(const std::string& serialised_matrix_record) {
  MatrixRecord matrix_record(serialised_matrix_record);
  if (matrix_record.matrix_ids().empty())
    EraseNode(NodeInfo(matrix_record.owner_id()));
  else
    InsertNode(matrix_record);

  ++g_state_id;
  NodeSet snapshot([](const NodeInfo& lhs, const NodeInfo& rhs) { return lhs.id < rhs.id; });
  // TODO(Fraser#5#): 2013-04-10 - Optimise this
  // Construct deep copy of g_nodes using NodeInfo with empty matrix.
  for (const auto& node : g_nodes)
    snapshot.emplace(node.id);
  // Apply the links in each entry's matrix
  auto main_itr(std::begin(g_nodes));
  auto snapshot_itr(std::begin(snapshot));
  while (main_itr != std::end(g_nodes)) {
    for (const auto& child : main_itr->matrix) {
      // Find the child in the new NodeSet
      auto g_itr(std::find_if(std::begin(snapshot),
                              std::end(snapshot),
                              [&child](const NodeInfo& node_info) {
                                  return child.first->id == node_info.id;
                              }));
      snapshot_itr->matrix.insert(std::make_pair(g_itr, child.second));
    }
    ++main_itr;
    ++snapshot_itr;
  }

  if (g_functor)
    g_functor(g_state_id);
}

void SetUpdateFunctor(std::function<void(int /*state_id*/)> functor) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_functor = functor;
}

std::vector<std::string> GetNodesInNetwork(int state_id) {
  auto snapshot_itr(g_snapshots.find(state_id));
  if (snapshot_itr == std::end(g_snapshots))
    --snapshot_itr;
  std::vector<std::string> hex_encoded_ids;
  hex_encoded_ids.reserve(snapshot_itr->second.size());
  for (const auto& node : snapshot_itr->second)
    hex_encoded_ids.push_back(node.id.ToStringEncoded(NodeId::kHex));
  return hex_encoded_ids;
}

std::vector<ViewableNode> GetCloseNodes(int state_id, const std::string& hex_encoded_id) {
  auto snapshot_itr(g_snapshots.find(state_id));
  if (snapshot_itr == std::end(g_snapshots))
    --snapshot_itr;

  NodeId node_id(hex_encoded_id, NodeId::kHex);
  auto itr(std::find_if(std::begin(snapshot_itr->second),
                        std::end(snapshot_itr->second),
                        [&node_id](const NodeInfo& node_info) { return node_id == node_info.id; }));

  std::vector<ViewableNode> children;
  if (itr != std::end(snapshot_itr->second)) {
    for (const auto& child : itr->matrix)
      children.emplace_back((*child.first).id.ToStringEncoded(NodeId::kHex),
                            (itr->id ^ (*child.first).id).ToStringEncoded(NodeId::kHex),
                            child.second);
  }

  return children;
}

void EraseSnapshot(int state_id) {
  g_snapshots.erase(state_id);
}

int Run(int argc, char **argv) {
#ifndef MAIDSAFE_WIN32
  signal(SIGHUP, SigHandler);
#endif
  signal(SIGINT, SigHandler);
  signal(SIGTERM, SigHandler);

  log::Logging::Instance().Initialise(argc, argv);
  try {
    bi::message_queue::remove(kMessageQueueName.c_str());
    on_scope_exit cleanup([]() { bi::message_queue::remove(kMessageQueueName.c_str()); });

    bi::message_queue matrix_messages(bi::create_only, kMessageQueueName.c_str(), 1000, 10000);
    LOG(kSuccess) << "Running...";
    unsigned int priority;
    bi::message_queue::size_type received_size;
    char input[10000];
    for (;;) {
      if (matrix_messages.try_receive(&input[0], 10000, received_size, priority)) {
        std::string received(&input[0], received_size);
        UpdateNodeInfo(received);
      }
      std::unique_lock<std::mutex> lock(g_mutex);
      if (g_cond_var.wait_for(lock, std::chrono::milliseconds(20), [] { return g_ctrlc_pressed; }))
        return 0;
    }
  }
  catch(bi::interprocess_exception &ex) {
    LOG(kError) << ex.what();
    return 1;
  }
}

}  // namespace network_viewer

}  // namespace maidsafe
