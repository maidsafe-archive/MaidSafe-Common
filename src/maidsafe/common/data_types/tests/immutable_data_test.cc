/*  Copyright 2015 MaidSafe.net limited

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

#include "maidsafe/common/data_types/immutable_data.h"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/tests/test_utils.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace test {

inline testing::AssertionResult Equal(const ImmutableData* const lhs,
                                      const ImmutableData* const rhs) {
  testing::AssertionResult result(
      Equal(dynamic_cast<const Data*>(lhs), dynamic_cast<const Data*>(rhs)));
  if (!result)
    return result;

  if (lhs->Value() != rhs->Value()) {
    return testing::AssertionFailure() << "lhs->Value() [" << hex::Substr(lhs->Value())
                                       << "] != rhs->Value() [" << hex::Substr(lhs->Value())
                                       << "].";
  } else {
    return testing::AssertionSuccess();
  }
}


TEST(ImmutableDataTest, BEH_ConstructAndAssign) {
  // Default c'tor
  const ImmutableData default_test_data;
  EXPECT_FALSE(default_test_data.IsInitialised());
  EXPECT_THROW(default_test_data.Name(), common_error);
  EXPECT_THROW(default_test_data.TypeId(), common_error);
  EXPECT_THROW(default_test_data.NameAndType(), common_error);
  EXPECT_THROW(Serialise(default_test_data), common_error);

  // C'tor taking value
  const DataTypeId type_id(0);
  const NonEmptyString value(RandomBytes(1, 1000));
  const Identity name(crypto::Hash<crypto::SHA512>(value));
  const ImmutableData immutable_data(value);
  EXPECT_TRUE(immutable_data.IsInitialised());
  EXPECT_EQ(name, immutable_data.Name());
  EXPECT_EQ(type_id, immutable_data.TypeId());
  EXPECT_EQ(name, immutable_data.NameAndType().name);
  EXPECT_EQ(type_id, immutable_data.NameAndType().type_id);
  EXPECT_EQ(value, immutable_data.Value());

  // Copy c'tor
  ImmutableData copied(immutable_data);
  EXPECT_TRUE(Equal(&immutable_data, &copied));

  // Move c'tor
  ImmutableData moved(std::move(copied));
  EXPECT_TRUE(Equal(&immutable_data, &moved));

  // Copy assignment
  ImmutableData copy_assigned;
  EXPECT_FALSE(Equal(&immutable_data, &copy_assigned));
  copy_assigned = immutable_data;
  EXPECT_TRUE(Equal(&immutable_data, &copy_assigned));

  // Move assignment
  ImmutableData move_assigned;
  EXPECT_FALSE(Equal(&immutable_data, &move_assigned));
  move_assigned = std::move(copy_assigned);
  EXPECT_TRUE(Equal(&immutable_data, &move_assigned));
}

TEST(ImmutableDataTest, BEH_SerialiseParse) {
  const NonEmptyString value(RandomBytes(1, 1000));

  // Serialise/parse as derived type
  const ImmutableData immutable_data(value);
  SerialisedData serialised(Serialise(immutable_data));
  ImmutableData parsed(Parse<ImmutableData>(serialised));
  EXPECT_TRUE(Equal(&immutable_data, &parsed));

  // Serialise/parse as base type
  const std::unique_ptr<Data> data_ptr(new ImmutableData(value));
  serialised = Serialise(data_ptr);
  std::unique_ptr<Data> parsed_ptr(Parse<std::unique_ptr<Data>>(serialised));
  ASSERT_NE(nullptr, parsed_ptr.get());
  ASSERT_NE(nullptr, dynamic_cast<ImmutableData*>(parsed_ptr.get()));
  EXPECT_TRUE(Equal(&immutable_data, dynamic_cast<ImmutableData*>(parsed_ptr.get())));
}

}  // namespace test

}  // namespace maidsafe
