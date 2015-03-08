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

#include "maidsafe/common/data_types/data.h"

#include <limits>

#include "cereal/types/base_class.hpp"
#include "cereal/types/polymorphic.hpp"

#include "maidsafe/common/identity.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/serialisation/binary_archive.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace test {

TEST(DataTest, BEH_ConstructAndAssignNameAndTypeId) {
  // Default c'tor
  const Data::NameAndTypeId default_name_and_type_id;
  EXPECT_FALSE(default_name_and_type_id.name.IsInitialised());
  EXPECT_EQ(std::numeric_limits<std::uint32_t>::max(), default_name_and_type_id.type_id);

  // C'tor taking name and type ID
  const Identity name(MakeIdentity());
  const DataTypeId type_id(RandomUint32());
  const Data::NameAndTypeId name_and_type_id(name, type_id);
  EXPECT_EQ(name, name_and_type_id.name);
  EXPECT_EQ(type_id, name_and_type_id.type_id);

  // Copy c'tor
  Data::NameAndTypeId copied(name_and_type_id);
  EXPECT_EQ(name, copied.name);
  EXPECT_EQ(type_id, copied.type_id);

  // Move c'tor
  Data::NameAndTypeId moved(std::move(copied));
  EXPECT_EQ(name, moved.name);
  EXPECT_EQ(type_id, moved.type_id);

  // Copy assignment
  copied.name = MakeIdentity();
  copied.type_id = DataTypeId(RandomUint32());
  EXPECT_NE(name, copied.name);
  EXPECT_NE(type_id, copied.type_id);
  copied = name_and_type_id;
  EXPECT_EQ(name, copied.name);
  EXPECT_EQ(type_id, copied.type_id);

  // Move assignment
  moved.name = MakeIdentity();
  moved.type_id = DataTypeId(RandomUint32());
  EXPECT_NE(name, moved.name);
  EXPECT_NE(type_id, moved.type_id);
  moved = std::move(copied);
  EXPECT_EQ(name, moved.name);
  EXPECT_EQ(type_id, moved.type_id);
}

TEST(DataTest, BEH_NameAndTypeIdComparisonOperators) {
  Identity name1(MakeIdentity());
  Identity name2(MakeIdentity());
  ASSERT_NE(name1, name2);
  const DataTypeId type_id1(RandomUint32());
  const DataTypeId type_id2(RandomUint32());
  ASSERT_NE(type_id1, type_id2);

  const Identity name(name1 < name2 ? name2 : name1);
  const Identity lower_name(name1 < name2 ? name1 : name2);
  const DataTypeId type_id(type_id1 < type_id2 ? type_id2 : type_id1);
  const DataTypeId lower_type_id(type_id1 < type_id2 ? type_id1 : type_id2);

  const Data::NameAndTypeId name_and_type_id(name, type_id);
  const Data::NameAndTypeId lower_name_only(lower_name, type_id);
  const Data::NameAndTypeId lower_type_id_only(name, lower_type_id);
  const Data::NameAndTypeId lower_name_and_type_id(lower_name, lower_type_id);

  EXPECT_TRUE(lower_name_only < name_and_type_id);
  EXPECT_TRUE(lower_type_id_only < name_and_type_id);
  EXPECT_TRUE(lower_name_and_type_id < name_and_type_id);
  EXPECT_FALSE(name_and_type_id < name_and_type_id);

  EXPECT_FALSE(lower_name_only > name_and_type_id);
  EXPECT_FALSE(lower_type_id_only > name_and_type_id);
  EXPECT_FALSE(lower_name_and_type_id > name_and_type_id);
  EXPECT_FALSE(name_and_type_id > name_and_type_id);

  EXPECT_TRUE(lower_name_only <= name_and_type_id);
  EXPECT_TRUE(lower_type_id_only <= name_and_type_id);
  EXPECT_TRUE(lower_name_and_type_id <= name_and_type_id);
  EXPECT_TRUE(name_and_type_id <= name_and_type_id);

  EXPECT_FALSE(lower_name_only >= name_and_type_id);
  EXPECT_FALSE(lower_type_id_only >= name_and_type_id);
  EXPECT_FALSE(lower_name_and_type_id >= name_and_type_id);
  EXPECT_TRUE(name_and_type_id >= name_and_type_id);

  EXPECT_FALSE(lower_name_only == name_and_type_id);
  EXPECT_FALSE(lower_type_id_only == name_and_type_id);
  EXPECT_FALSE(lower_name_and_type_id == name_and_type_id);
  EXPECT_TRUE(name_and_type_id == name_and_type_id);

  EXPECT_TRUE(lower_name_only != name_and_type_id);
  EXPECT_TRUE(lower_type_id_only != name_and_type_id);
  EXPECT_TRUE(lower_name_and_type_id != name_and_type_id);
  EXPECT_FALSE(name_and_type_id != name_and_type_id);
}

TEST(DataTest, BEH_SerialiseNameAndTypeId) {
  const Identity name(MakeIdentity());
  const DataTypeId type_id(RandomUint32());
  const Data::NameAndTypeId name_and_type_id(name, type_id);

  SerialisedData serialised(Serialise(name_and_type_id));
  Data::NameAndTypeId parsed(Parse<Data::NameAndTypeId>(serialised));

  EXPECT_EQ(name_and_type_id, parsed);
}

class TestData : public Data {
 public:
  TestData(Identity id, std::string value) : Data(std::move(id)), value_(std::move(value)) {}

  TestData() = default;
  TestData(const TestData&) = default;
  TestData(TestData&& other) : Data(std::move(other)), value_(std::move(other.value_)) {}
  TestData& operator=(const TestData&) = default;
  TestData& operator=(TestData&& other) {
    Data::operator=(std::move(other));
    value_ = std::move(other.value_);
    return *this;
  }
  virtual ~TestData() final = default;

  template <typename Archive>
  Archive& save(Archive& archive) const {
    return archive(cereal::base_class<Data>(this), value_);
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    try {
      archive(cereal::base_class<Data>(this), value_);
    } catch (const std::exception&) {
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    }
    return archive;
  }

  std::string value_;

 private:
  virtual std::uint32_t ThisTypeId() const final { return 123456; }
};

inline bool operator==(const TestData& lhs, const TestData& rhs) {
  if (!lhs.IsInitialised() && !rhs.IsInitialised())
    return true;

  if (!lhs.IsInitialised() || !rhs.IsInitialised())
    return false;

  return lhs.NameAndType() == rhs.NameAndType() && lhs.value_ == rhs.value_;
}

inline bool operator!=(const TestData& lhs, const TestData& rhs) { return !operator==(lhs, rhs); }

}  // namespace test

}  // namespace maidsafe

CEREAL_REGISTER_TYPE(maidsafe::test::TestData);

namespace maidsafe {

namespace test {

TEST(DataTest, BEH_ConstructAndAssignData) {
  // Default c'tor
  const TestData default_test_data;
  EXPECT_FALSE(default_test_data.IsInitialised());
  EXPECT_THROW(default_test_data.Name(), common_error);
  EXPECT_THROW(default_test_data.TypeId(), common_error);
  EXPECT_THROW(default_test_data.NameAndType(), common_error);
  EXPECT_THROW(Serialise(default_test_data), common_error);

  // C'tor taking Identity and value
  const Identity name(MakeIdentity());
  const DataTypeId type_id(123456);
  const std::string value(RandomString(1, 1000));
  const TestData test_data(name, value);
  EXPECT_TRUE(test_data.IsInitialised());
  EXPECT_EQ(name, test_data.Name());
  EXPECT_EQ(type_id, test_data.TypeId());
  EXPECT_EQ(name, test_data.NameAndType().name);
  EXPECT_EQ(type_id, test_data.NameAndType().type_id);
  EXPECT_EQ(value, test_data.value_);

  // Copy c'tor
  TestData copied(test_data);
  EXPECT_EQ(test_data, copied);

  // Move c'tor
  TestData moved(std::move(copied));
  EXPECT_EQ(test_data, moved);

  // Copy assignment
  TestData copy_assigned;
  EXPECT_NE(test_data, copy_assigned);
  copy_assigned = test_data;
  EXPECT_EQ(test_data, copy_assigned);

  // Move assignment
  TestData move_assigned;
  EXPECT_NE(test_data, move_assigned);
  move_assigned = std::move(copy_assigned);
  EXPECT_EQ(test_data, move_assigned);
}

TEST(DataTest, BEH_SerialiseData) {
  const Identity id(MakeIdentity());
  const std::string value(RandomString(0, 1000));

  // Serialise/parse as derived type
  const TestData test_data(id, value);
  SerialisedData serialised(Serialise(test_data));
  TestData parsed(Parse<TestData>(serialised));
  EXPECT_EQ(test_data, parsed);

  // Serialise/parse as base type
  const std::unique_ptr<Data> data_ptr(new TestData(id, value));
  serialised = Serialise(data_ptr);
  std::unique_ptr<Data> parsed_ptr(Parse<std::unique_ptr<Data>>(serialised));
  ASSERT_NE(nullptr, parsed_ptr.get());
  ASSERT_NE(nullptr, dynamic_cast<TestData*>(parsed_ptr.get()));
  EXPECT_EQ(test_data, *dynamic_cast<TestData*>(parsed_ptr.get()));
}

}  // namespace test

}  // namespace maidsafe
