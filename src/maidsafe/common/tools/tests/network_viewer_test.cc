/*  Copyright 2014 MaidSafe.net limited

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

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

namespace {  // anonymous

using Mr_t = network_viewer::MatrixRecord;

bool operator==(const Mr_t& ref_lhs, const Mr_t& ref_rhs) {
  return ref_lhs.owner_id() == ref_rhs.owner_id() && ref_lhs.matrix_ids() == ref_rhs.matrix_ids();
}

}  // anonymous namespace

TEST(NetworkViewerTest, BEH_MatrixRecordSerialisation) {
  NodeId node_id_0{NodeId::IdType::kRandomId};
  NodeId node_id_1{NodeId::IdType::kRandomId};
  Mr_t a{node_id_0}, b{node_id_1};

  // Serialisation
  EXPECT_FALSE(a == b);
  auto serialised_data_0(a.Serialise());

  // Deserialisation
  Mr_t c{serialised_data_0};
  EXPECT_FALSE(b == c);
  EXPECT_TRUE(a == c);

  // Reserialise
  auto serialised_data_1(c.Serialise());
  EXPECT_TRUE(serialised_data_0 == serialised_data_1);
}

}  // namespace test

}  // namespace maidsafe
