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

#ifndef MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_
#define MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/common/node_id.h"


namespace maidsafe {

namespace network_viewer {

namespace protobuf { class MatrixRecord; }

extern const std::string kMessageQueueName;

enum class ChildType : int32_t { kGroup, kClosest, kMatrix };

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

void EraseSnapshot(int state_id);

int Run(int argc, char **argv);

}  // namespace network_viewer

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TOOLS_NETWORK_VIEWER_H_
