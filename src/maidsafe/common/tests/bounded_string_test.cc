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

#include <cstdint>
#include <string>
#include <vector>

#include "maidsafe/common/bounded_string.h"

#include "maidsafe/common/convert.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace detail {

namespace test {

template <typename T>
class BoundedStringTest : public testing::Test {
 protected:
  using OneOne = BoundedString<1, 1, T>;
  using OneTwo = BoundedString<1, 2, T>;
  using OneThree = BoundedString<1, 3, T>;
  using OneMax = BoundedString<1, static_cast<std::size_t>(-1), T>;
  using TwoTwo = BoundedString<2, 2, T>;
  using TwoThree = BoundedString<2, 3, T>;
  using TwoFour = BoundedString<2, 4, T>;
  using TwoMax = BoundedString<2, static_cast<std::size_t>(-1), T>;

  T RandomData(std::size_t size) const;
  T RandomData(std::uint32_t min, std::uint32_t max) const;
  std::string ToString(const T& input) const;
};

template <>
std::vector<byte> BoundedStringTest<std::vector<byte>>::RandomData(std::uint32_t min,
  std::uint32_t max) const {
  return RandomBytes(min, max);
}

template <>
std::string BoundedStringTest<std::string>::RandomData(std::uint32_t min, std::uint32_t max) const {
  return RandomString(min, max);
}

template <>
std::vector<byte> BoundedStringTest<std::vector<byte>>::RandomData(std::size_t size) const {
  return RandomBytes(size);
}

template <>
std::string BoundedStringTest<std::string>::RandomData(std::size_t size) const {
  return RandomString(size);
}

template <>
std::string BoundedStringTest<std::vector<byte>>::ToString(const std::vector<byte>& input) const {
  return convert::ToString(input);
}

template <>
std::string BoundedStringTest<std::string>::ToString(const std::string& input) const {
  return input;
}



using TestTypes = testing::Types<std::vector<byte>, std::string>;
TYPED_TEST_CASE(BoundedStringTest, TestTypes);

TYPED_TEST(BoundedStringTest, BEH_DefaultConstructor) {
  typename TestFixture::OneOne a;
  EXPECT_FALSE(a.IsInitialised());
  typename TestFixture::OneMax b;
  EXPECT_FALSE(b.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_Getters) {
  const typename TestFixture::TwoTwo a;
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_THROW(a.string(), common_error);
  EXPECT_THROW(a.data(), common_error);
  EXPECT_THROW(typename TestFixture::TwoTwo()[0], common_error);
  EXPECT_THROW(a[0], common_error);
  EXPECT_THROW(a.size(), common_error);

  auto random(this->RandomData(1, 1024));
  typename TestFixture::OneMax b(random);
  EXPECT_TRUE(b.IsInitialised());
  EXPECT_EQ(random, b.string());
  std::string copied(reinterpret_cast<const char*>(b.data()), b.size());
  std::string original(random.begin(), random.end());
  ASSERT_EQ(original, copied);
  const typename TestFixture::OneMax c(random);
  for (std::size_t i(0); i < random.size(); ++i) {
    EXPECT_EQ(random[i], b[i]);
    EXPECT_EQ(random[i], c[i]);
  }
}

TYPED_TEST(BoundedStringTest, BEH_StringConstructor) {
  // Empty (invalid)
  EXPECT_THROW(typename TestFixture::OneOne a(""), common_error);
  // Valid
  std::string random(RandomString(1));
  typename TestFixture::OneOne b(random);
  EXPECT_EQ(random, this->ToString(b.string()));
  // Too big
  EXPECT_THROW(typename TestFixture::OneOne c(RandomString(2)), common_error);

  // Empty (invalid)
  EXPECT_THROW(typename TestFixture::OneMax d(""), common_error);
  // Valid
  random = RandomString(1, 1024);
  typename TestFixture::OneMax e(random);
  EXPECT_EQ(random, this->ToString(e.string()));
}

TYPED_TEST(BoundedStringTest, BEH_Swap) {
  // Swap with initialised
  auto random1(this->RandomData(1));
  auto random2(this->RandomData(2));
  typename TestFixture::OneTwo a(random1);
  typename TestFixture::OneTwo b(random2);
  swap(a, b);
  EXPECT_EQ(random2, a.string());
  EXPECT_EQ(random1, b.string());

  // Swap with uninitialised
  typename TestFixture::OneTwo c;
  swap(a, c);
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_EQ(random2, c.string());
}

TYPED_TEST(BoundedStringTest, BEH_CopyConstruction) {
  // Copy from initialised
  auto random(this->RandomData(1, 1024));
  typename TestFixture::OneMax a(random);
  typename TestFixture::OneMax b(a);
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Copy from uninitialised
  typename TestFixture::OneMax c;
  typename TestFixture::OneMax d(c);
  EXPECT_FALSE(d.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_MoveConstruction) {
  // Move from initialised
  auto random(this->RandomData(1, 1024));
  typename TestFixture::OneMax a(std::move(typename TestFixture::OneMax(random)));
  EXPECT_EQ(random, a.string());

  // Move from uninitialised
  typename TestFixture::OneMax b(std::move(typename TestFixture::OneMax()));
  EXPECT_FALSE(b.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_CopyAssignment) {
  // Assign from initialised
  auto random(this->RandomData(1, 1024));
  typename TestFixture::OneMax a(random);
  typename TestFixture::OneMax b("1");
  b = a;
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Assign from self
  b = b;
  EXPECT_EQ(random, b.string());

  // Assign from uninitialised
  typename TestFixture::OneMax c;
  b = c;
  EXPECT_FALSE(b.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_MoveAssignment) {
  // Assign from initialised
  auto random(this->RandomData(1, 1024));
  typename TestFixture::OneMax a(random);
  typename TestFixture::OneMax b("1");
  b = std::move(a);
  EXPECT_EQ(random, b.string());

  // Assign from uninitialised
  typename TestFixture::OneMax c;
  b = std::move(c);
  EXPECT_FALSE(b.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_ConstructionFromDifferentType) {
  // Valid copy
  auto random(this->RandomData(2));
  typename TestFixture::TwoThree a(random);
  typename TestFixture::OneMax b(a);
  EXPECT_EQ(random, b.string());
  typename TestFixture::TwoFour c(std::move(a));
  EXPECT_EQ(random, c.string());

  // Copy from uninitialised
  typename TestFixture::TwoThree d;
  typename TestFixture::OneThree e(d);
  EXPECT_FALSE(e.IsInitialised());
  typename TestFixture::TwoFour f(std::move(d));
  EXPECT_FALSE(f.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_AssignmentFromDifferentType) {
  // Valid assignment
  auto random(this->RandomData(2));
  typename TestFixture::TwoThree a(random);
  typename TestFixture::OneMax b("1");
  b = a;
  EXPECT_EQ(random, b.string());
  typename TestFixture::TwoFour c("02");
  c = std::move(a);
  EXPECT_EQ(random, c.string());

  // Assign from uninitialised
  typename TestFixture::TwoThree d;
  typename TestFixture::OneThree e("1");
  e = d;
  EXPECT_FALSE(e.IsInitialised());
  typename TestFixture::TwoFour f("02");
  f = std::move(d);
  EXPECT_FALSE(f.IsInitialised());
}

TYPED_TEST(BoundedStringTest, BEH_Serialization) {
  // Invalid Serialisation
  typename TestFixture::OneThree a;
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_THROW(Serialise(a), common_error);

  // Valid Serialisation
  typename TestFixture::OneThree b(this->RandomData(1));
  EXPECT_TRUE(b.IsInitialised());

  SerialisedData serialised_str;
  EXPECT_TRUE(serialised_str.empty());
  EXPECT_NO_THROW(serialised_str = Serialise(b));
  EXPECT_FALSE(serialised_str.empty());

  // Invalid Deserialisation
  typename TestFixture::TwoThree c;
  EXPECT_FALSE(c.IsInitialised());
  EXPECT_THROW(Parse(serialised_str, c), common_error);

  // Valid Deserialisation
  typename TestFixture::OneTwo d;
  EXPECT_FALSE(d.IsInitialised());
  EXPECT_NO_THROW(Parse(serialised_str, d));
  EXPECT_EQ(b.string(), d.string());
}

TYPED_TEST(BoundedStringTest, BEH_StreamOperator) {
  std::stringstream ss;
  typename TestFixture::OneMax a(this->RandomData(1, 1000));
  ss << a;
  EXPECT_EQ(ss.str(), hex::Substr(a));

  ss.str("");
  ss << typename TestFixture::OneMax();
  EXPECT_EQ(ss.str(), "Invalid string");
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
