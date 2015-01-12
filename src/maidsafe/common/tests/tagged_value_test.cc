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

#include "maidsafe/common/tagged_value.h"

#include <string>

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/serialisation/serialisation.h"

std::ostream& operator<<(std::ostream& ostream, const maidsafe::Identity& identity) {
  ostream << identity.string();
  return ostream;
}

namespace maidsafe {

namespace detail {

namespace test {

struct TestTag;

template <typename T>
class TaggedValueTest : public testing::Test {
 protected:
  // For numeric types
  T RandomValue() { return static_cast<T>(RandomUint32()); }
  testing::AssertionResult Matches(T expected, T actual) {
    if (expected == actual) {
      return testing::AssertionSuccess();
    } else {
      return testing::AssertionFailure() << "\n\tExpected: " << expected
                                         << "\n\tActual: " << actual;
    }
  }
  using TestValue = TaggedValue<T, TestTag>;
};

template <>
std::string TaggedValueTest<std::string>::RandomValue() {
  return RandomAlphaNumericString(10);
}

template <>
Identity TaggedValueTest<Identity>::RandomValue() {
  return Identity{RandomAlphaNumericString(64)};
}

using TestTypes =
    testing::Types<char, int, int8_t, int16_t, int32_t, int64_t, unsigned, uint8_t, uint16_t,
                   uint32_t, uint64_t, double, float, std::string, Identity>;

TYPED_TEST_CASE(TaggedValueTest, TestTypes);

TYPED_TEST(TaggedValueTest, BEH_ConstructAndAssign) {
  // Default c'tor
  ASSERT_NO_THROW(TestValue{});

  // C'tor taking value
  ASSERT_NO_THROW(TestValue{RandomValue()});
  const TestValue tagged_value1{RandomValue()};
  const TestValue tagged_value2{RandomValue()};

  // Copy and move
  TestValue copied{tagged_value1};
  EXPECT_TRUE(tagged_value1 == copied);

  TestValue moved{std::move(copied)};
  EXPECT_TRUE(tagged_value1 == moved);

  copied = tagged_value2;
  EXPECT_TRUE(tagged_value2 == copied);

  moved = std::move(copied);
  EXPECT_TRUE(tagged_value2 == moved);
}

TYPED_TEST(TaggedValueTest, BEH_Observers) {
  const auto raw_data1(RandomValue());
  const auto raw_data2(RandomValue());
  ASSERT_NE(raw_data1, raw_data2);
  const TestValue tagged_value1{raw_data1};
  TestValue tagged_value2{raw_data2};

  // operator T() const
  EXPECT_TRUE(Matches(raw_data1, tagged_value1));

  // T const* operator->() const
  EXPECT_EQ(raw_data1, *tagged_value1.operator->());

  // T* operator->()
  EXPECT_EQ(raw_data2, *tagged_value2.operator->());
}

TYPED_TEST(TaggedValueTest, BEH_Comparisons) {
  const auto raw_data1(RandomValue());
  const auto raw_data2(RandomValue());
  ASSERT_NE(raw_data1, raw_data2);
  const TestValue tagged_value1{raw_data1 < raw_data2 ? raw_data1 : raw_data2};
  const TestValue tagged_value2{raw_data1 < raw_data2 ? raw_data2 : raw_data1};

  EXPECT_TRUE(tagged_value1 == tagged_value1);
  EXPECT_FALSE(tagged_value1 == tagged_value2);

  EXPECT_FALSE(tagged_value1 != tagged_value1);
  EXPECT_TRUE(tagged_value1 != tagged_value2);

  EXPECT_FALSE(tagged_value1 < tagged_value1);
  EXPECT_TRUE(tagged_value1 < tagged_value2);
  EXPECT_FALSE(tagged_value2 < tagged_value1);

  EXPECT_FALSE(tagged_value1 > tagged_value1);
  EXPECT_FALSE(tagged_value1 > tagged_value2);
  EXPECT_TRUE(tagged_value2 > tagged_value1);

  EXPECT_TRUE(tagged_value1 <= tagged_value1);
  EXPECT_TRUE(tagged_value1 <= tagged_value2);
  EXPECT_FALSE(tagged_value2 <= tagged_value1);

  EXPECT_TRUE(tagged_value1 >= tagged_value1);
  EXPECT_FALSE(tagged_value1 >= tagged_value2);
  EXPECT_TRUE(tagged_value2 >= tagged_value1);
}

TYPED_TEST(TaggedValueTest, BEH_Serialisation) {
  const TestValue tagged_value{RandomValue()};
  auto serialised(Serialise(tagged_value));
  TestValue parsed{Parse<TestValue>(serialised)};
  EXPECT_TRUE(tagged_value == parsed);
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
