/* Copyright 2013 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_
#define MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/common/node_id.h"


namespace maidsafe {

namespace network_viewer {

extern const std::string kMessageQueueName;

enum class ChildType : int32_t { kGroup, kClosest, kMatrix, kNotConnected };

struct ViewableNode {
  ViewableNode();
  ViewableNode(const std::string& id_in, const std::string& distance_in, ChildType type_in);
  ViewableNode(const ViewableNode& other);
  ViewableNode(ViewableNode&& other);
  ViewableNode& operator=(ViewableNode other);

  std::string id, distance;
  ChildType type;
};

class MatrixRecord {
 public:
  typedef std::map<NodeId, ChildType, std::function<bool(const NodeId&, const NodeId&)>> MatrixIds;

  MatrixRecord();
  explicit MatrixRecord(const NodeId& owner_id);
  explicit MatrixRecord(const std::string& serialised_matrix_record);
  std::string Serialise() const;
  MatrixRecord(const MatrixRecord& other);
  MatrixRecord(MatrixRecord&& other);
  MatrixRecord& operator=(MatrixRecord other);

  // If element already exists, replaces existing value of 'type'.
  void AddElement(const NodeId& element_id, ChildType child_type);
  NodeId owner_id() const;
  MatrixIds matrix_ids() const;

 private:
  NodeId owner_id_;
  MatrixIds matrix_ids_;
};

void SetUpdateFunctor(std::function<void(int /*state_id*/)> functor);

std::vector<std::string> GetNodesInNetwork(int state_id);

std::vector<ViewableNode> GetCloseNodes(int state_id, const std::string& hex_encoded_id);

void SetNotifyInterval(const std::chrono::milliseconds& notify_interval);

void Run(const std::chrono::milliseconds& notify_interval);

void Stop();

}  // namespace network_viewer

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_
