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

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/immutable_data.h"
#include "maidsafe/common/data_types/mutable_data.h"
#include "maidsafe/common/data_types/tests/test_utils.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace test {

template <typename T>
class DerivedDataTest : public testing::Test {
 protected:
  DerivedDataTest()
      : value_(RandomBytes(1, 1000)),
        name_(GetName()),
        data_(GetData()),
        type_id_(data_.TypeId()) {}

  testing::AssertionResult Equal(const T* const lhs, const T* const rhs) {
    testing::AssertionResult result(
        maidsafe::test::Equal(dynamic_cast<const Data*>(lhs), dynamic_cast<const Data*>(rhs)));
    if (!result)
      return result;

    if (lhs->Value() != rhs->Value()) {
      return testing::AssertionFailure() << "lhs->Value() [" << lhs->Value()
                                         << "] != rhs->Value() [" << lhs->Value() << "].";
    } else {
      return testing::AssertionSuccess();
    }
  }

  const NonEmptyString value_;
  Identity name_;
  T data_;
  const DataTypeId type_id_;

 private:
  Identity GetName();
  T GetData();
};

template <>
Identity DerivedDataTest<ImmutableData>::GetName() {
  return crypto::Hash<crypto::SHA512>(this->value_);
}

template <>
Identity DerivedDataTest<MutableData>::GetName() {
  return MakeIdentity();
}

template <>
ImmutableData DerivedDataTest<ImmutableData>::GetData() {
  return ImmutableData(this->value_);
}

template <>
MutableData DerivedDataTest<MutableData>::GetData() {
  return MutableData(this->name_, this->value_);
}

using TestTypes = testing::Types<ImmutableData, MutableData>;
TYPED_TEST_CASE(DerivedDataTest, TestTypes);

TYPED_TEST(DerivedDataTest, BEH_ConstructAndAssign) {
  // Default c'tor
  const TypeParam default_test_data;
  EXPECT_FALSE(default_test_data.IsInitialised());
  EXPECT_THROW(default_test_data.Name(), common_error);
  EXPECT_THROW(default_test_data.TypeId(), common_error);
  EXPECT_THROW(default_test_data.NameAndType(), common_error);
  EXPECT_THROW(Serialise(default_test_data), common_error);

  // C'tor taking value
  EXPECT_TRUE(this->data_.IsInitialised());
  EXPECT_EQ(this->name_, this->data_.Name());
  EXPECT_EQ(this->type_id_, this->data_.TypeId());
  EXPECT_EQ(this->name_, this->data_.NameAndType().name);
  EXPECT_EQ(this->type_id_, this->data_.NameAndType().type_id);
  EXPECT_EQ(this->value_, this->data_.Value());

  // Copy c'tor
  TypeParam copied(this->data_);
  EXPECT_TRUE(Equal(&this->data_, &copied));

  // Move c'tor
  TypeParam moved(std::move(copied));
  EXPECT_TRUE(Equal(&this->data_, &moved));

  // Copy assignment
  TypeParam copy_assigned;
  EXPECT_FALSE(Equal(&this->data_, &copy_assigned));
  copy_assigned = this->data_;
  EXPECT_TRUE(Equal(&this->data_, &copy_assigned));

  // Move assignment
  TypeParam move_assigned;
  EXPECT_FALSE(Equal(&this->data_, &move_assigned));
  move_assigned = std::move(copy_assigned);
  EXPECT_TRUE(Equal(&this->data_, &move_assigned));
}

TYPED_TEST(DerivedDataTest, BEH_SerialiseParse) {
  // Serialise/parse as derived type
  SerialisedData serialised(Serialise(this->data_));
  TypeParam parsed(Parse<TypeParam>(serialised));
  EXPECT_TRUE(Equal(&this->data_, &parsed));

  // Serialise/parse as base type
  const std::unique_ptr<Data> data_ptr(new TypeParam(this->data_));
  serialised = Serialise(data_ptr);
  std::unique_ptr<Data> parsed_ptr(Parse<std::unique_ptr<Data>>(serialised));
  ASSERT_NE(nullptr, parsed_ptr.get());
  ASSERT_NE(nullptr, dynamic_cast<TypeParam*>(parsed_ptr.get()));
  EXPECT_TRUE(Equal(&this->data_, dynamic_cast<TypeParam*>(parsed_ptr.get())));
}

}  // namespace test

}  // namespace maidsafe
