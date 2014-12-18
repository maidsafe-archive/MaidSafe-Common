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

#include <string>

#include "maidsafe/common/data_types/mutable_data.h"

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

namespace {  // anonymous

bool operator==(const MutableData& ref_lhs, const MutableData& ref_rhs) {
  return ref_lhs.data() == ref_rhs.data() && ref_lhs.name() == ref_rhs.name();
}

}  // anonymous namespace

TEST(MutableDataTest, BEH_Serialisation) {
  const std::uint32_t size{64};
  auto value_0 = NonEmptyString(RandomAlphaNumericString(size));

  MutableData::Name key_0{Identity(crypto::Hash<crypto::SHA512>(value_0))};
  MutableData a{key_0, value_0};

  std::string serialised_str;

  // Serialisation
  EXPECT_EQ(0, serialised_str.size());
  EXPECT_NO_THROW(serialised_str = maidsafe::ConvertToString(a));
  EXPECT_NE(0, serialised_str.size());

  // Deserialisation
  auto value_1 = NonEmptyString(RandomAlphaNumericString(size));
  MutableData::Name key_1{Identity(crypto::Hash<crypto::SHA512>(value_0))};
  MutableData b{key_1, value_1};

  EXPECT_FALSE(a == b);
  EXPECT_NO_THROW(maidsafe::ConvertFromString(serialised_str, b));
  EXPECT_TRUE(a == b);
}

}  // namespace test

}  // namespace maidsafe
