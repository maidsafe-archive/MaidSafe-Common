/*  Copyright 2012 MaidSafe.net limited

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

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/data_name_variant.h"

namespace maidsafe {

namespace test {

TEST(DataTypeTest, BEH_OutputOperator) {
  LOG(kInfo) << DataTagValue::kAnmaidValue;
  LOG(kInfo) << DataTagValue::kMaidValue;
  LOG(kInfo) << DataTagValue::kAnpmidValue;
  LOG(kInfo) << DataTagValue::kPmidValue;
  LOG(kInfo) << DataTagValue::kAnmpidValue;
  LOG(kInfo) << DataTagValue::kMpidValue;
  LOG(kInfo) << DataTagValue::kImmutableDataValue;
  LOG(kInfo) << DataTagValue::kMutableDataValue;
  LOG(kInfo) << static_cast<DataTagValue>(-9);
}

TEST(DataTypeTest, BEH_ConstructType) {
  ASSERT_TRUE(is_short_term_cacheable<MutableData>::value);
  ASSERT_TRUE(!is_long_term_cacheable<MutableData>::value);
  ASSERT_TRUE(!is_short_term_cacheable<ImmutableData>::value);
  ASSERT_TRUE(is_long_term_cacheable<ImmutableData>::value);
}

TEST(DataTypeTest, BEH_RetrieveType) {
  ASSERT_TRUE(
      (std::is_same<passport::PublicAnmaid, passport::PublicAnmaid::Name::data_type>::value));
  ASSERT_TRUE((std::is_same<passport::PublicMaid, passport::PublicMaid::Name::data_type>::value));
  ASSERT_TRUE(
      (std::is_same<passport::PublicAnpmid, passport::PublicAnpmid::Name::data_type>::value));
  ASSERT_TRUE((std::is_same<passport::PublicPmid, passport::PublicPmid::Name::data_type>::value));
  ASSERT_TRUE(
      (std::is_same<passport::PublicAnmpid, passport::PublicAnmpid::Name::data_type>::value));
  ASSERT_TRUE((std::is_same<passport::PublicMpid, passport::PublicMpid::Name::data_type>::value));
  ASSERT_TRUE((std::is_same<ImmutableData, ImmutableData::Name::data_type>::value));
  ASSERT_TRUE((std::is_same<MutableData, MutableData::Name::data_type>::value));
}

}  // namespace test

}  // namespace maidsafe
