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

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/common/serialisation/serialisation.h"
#include "maidsafe/common/tools/bootstrap_file_tool_cereal.h"

namespace maidsafe {

namespace detail {  // Provide ADL for this TU to std::vector comparisons

bool operator==(const EndpointCereal& ref_lhs, const EndpointCereal& ref_rhs) {
  return ref_lhs.port_ == ref_rhs.port_ && ref_lhs.ip_ == ref_rhs.ip_;
}

bool operator==(const BootstrapCereal& ref_lhs, const BootstrapCereal& ref_rhs) {
  return ref_lhs.bootstrap_contacts_ == ref_rhs.bootstrap_contacts_;
}

}  // namespace detail

namespace test {

TEST(BootstrapFileToolTest, BEH_Serialisation) {
  maidsafe::detail::BootstrapCereal a, b;
  maidsafe::detail::EndpointCereal c, d;

  c.ip_ = "192.168.12.45"; c.port_ = 64435;
  d.ip_ = "10.168.12.45"; d.port_ = 99;

  a.bootstrap_contacts_.push_back(c);
  a.bootstrap_contacts_.push_back(d);

  // Serialise
  EXPECT_FALSE(a == b);
  auto serialised_data_0(ConvertToString(a));

  // Deseriialise
  ConvertFromString(serialised_data_0, b);
  EXPECT_TRUE(a == b);
}

}  // namespace test

}  // namespace maidsafe
