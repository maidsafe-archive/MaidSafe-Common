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

#include <sstream>

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/serialisation.h"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace detail {

namespace test {

typedef BoundedString<1, 1> OneOne;
typedef BoundedString<1, 2> OneTwo;
typedef BoundedString<1, 3> OneThree;
typedef BoundedString<1> OneMax;
typedef BoundedString<2, 2> TwoTwo;
typedef BoundedString<2, 3> TwoThree;
typedef BoundedString<2, 4> TwoFour;
typedef BoundedString<2> TwoMax;

TEST(BoundedStringTest, BEH_DefaultConstructor) {
  OneOne a;
  EXPECT_FALSE(a.IsInitialised());
  OneMax b;
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_Getters) {
  TwoTwo a;
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_THROW(a.string(), std::exception);

  for (int i(0); i < 1000; ++i) {
    std::string random(RandomString((RandomUint32() % 1024) + 1));
    OneMax b(random);
    EXPECT_TRUE(b.IsInitialised());
    EXPECT_EQ(random, b.string());
  }
}

TEST(BoundedStringTest, BEH_StringConstructor) {
  // Empty (invalid)
  EXPECT_THROW(OneOne a(""), std::exception);
  // Valid
  std::string random(RandomString(1));
  OneOne b(random);
  EXPECT_EQ(random, b.string());
  // Too big
  EXPECT_THROW(OneOne c(RandomString(2)), std::exception);

  // Empty (invalid)
  EXPECT_THROW(OneMax d(""), std::exception);
  // Valid
  random = RandomString((RandomUint32() % 1024) + 1);
  OneMax e(random);
  EXPECT_EQ(random, e.string());
}

TEST(BoundedStringTest, BEH_Swap) {
  // Swap with initialised
  std::string random1(RandomString(1));
  std::string random2(RandomString(2));
  OneTwo a(random1);
  OneTwo b(random2);
  swap(a, b);
  EXPECT_EQ(random2, a.string());
  EXPECT_EQ(random1, b.string());

  // Swap with uninitialised
  OneTwo c;
  swap(a, c);
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_EQ(random2, c.string());
}

TEST(BoundedStringTest, BEH_Copy) {
  // Copy from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b(a);
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Copy from uninitialised
  OneMax c;
  OneMax d(c);
  EXPECT_FALSE(d.IsInitialised());
}

TEST(BoundedStringTest, BEH_Move) {
  // Move from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(std::move(OneMax(random)));
  EXPECT_EQ(random, a.string());

  // Move from uninitialised
  OneMax b(std::move(OneMax()));
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_Assignment) {
  // Assign from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b("1");
  b = a;
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Assign from self
  b = b;
  EXPECT_EQ(random, b.string());

  // Assign from uninitialised
  OneMax c;
  b = c;
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_CopyConstructor) {
  // Valid copy
  std::string random(RandomString(2));
  TwoThree a(random);
  OneMax b(a);
  EXPECT_EQ(random, b.string());
  TwoFour c(a);
  EXPECT_EQ(random, c.string());

  // Copy from uninitialised
  TwoThree d;
  OneThree e(d);
  EXPECT_FALSE(e.IsInitialised());
  TwoFour f(d);
  EXPECT_FALSE(f.IsInitialised());
}

TEST(BoundedStringTest, BEH_CopyAssignment) {
  // Valid assignment
  std::string random(RandomString(2));
  TwoThree a(random);
  OneMax b("1");
  b = a;
  EXPECT_EQ(random, b.string());
  TwoFour c("02");
  c = a;
  EXPECT_EQ(random, c.string());

  // Assign from uninitialised
  TwoThree d;
  OneThree e("1");
  e = d;
  EXPECT_FALSE(e.IsInitialised());
  TwoFour f("02");
  f = d;
  EXPECT_FALSE(f.IsInitialised());
}

TEST(BoundedStringTest, BEH_Concatenation) {
  OneOne a(RandomString(1));
  std::string a_before_throw = a.string();
  OneTwo b(RandomString(1));
  std::string b_before_throw = b.string();

  EXPECT_THROW(a + b, std::exception);
  EXPECT_EQ(a_before_throw, a.string());
  EXPECT_EQ(b_before_throw, b.string());

  EXPECT_THROW(a += b, std::exception);
  EXPECT_EQ(a_before_throw, a.string());
  EXPECT_EQ(b_before_throw, b.string());

  EXPECT_THROW(a + a, std::exception);
  EXPECT_EQ(a_before_throw, a.string());

  EXPECT_THROW(a += a, std::exception);
  EXPECT_EQ(a_before_throw, a.string());

  OneTwo c(b + a);
  EXPECT_EQ(a_before_throw, a.string());
  EXPECT_EQ(b_before_throw, b.string());
  auto concat(b_before_throw + a_before_throw);
  EXPECT_EQ(concat, c.string());

  b += a;
  EXPECT_EQ(a_before_throw, a.string());
  EXPECT_EQ(c.string(), b.string());

  b = OneTwo(b_before_throw);

  OneTwo d(b + b);
  EXPECT_EQ(b_before_throw, b.string());
  concat = b_before_throw + b_before_throw;
  EXPECT_EQ(concat, d.string());

  b += b;
  EXPECT_EQ(d.string(), b.string());

  b = OneTwo(b_before_throw);
  OneThree e(RandomString(1));
  std::string e_before_throw = e.string();

  OneThree f(e + b + b);
  EXPECT_EQ(b_before_throw, b.string());
  concat = e_before_throw + b_before_throw + b_before_throw;
  EXPECT_EQ(concat, f.string());

  e += (b + b);
  EXPECT_EQ(b_before_throw, b.string());
  EXPECT_EQ(f.string(), e.string());

  e = OneThree(e_before_throw);

  EXPECT_THROW(e + b + b + a, std::exception);
  EXPECT_EQ(a_before_throw, a.string());
  EXPECT_EQ(b_before_throw, b.string());
  EXPECT_EQ(e_before_throw, e.string());

  OneOne g;
  EXPECT_THROW(e + g, std::exception);
  EXPECT_EQ(e_before_throw, e.string());
  EXPECT_FALSE(g.IsInitialised());

  EXPECT_THROW(g + e, std::exception);
  EXPECT_EQ(e_before_throw, e.string());
  EXPECT_FALSE(g.IsInitialised());

  EXPECT_THROW(e += g, std::exception);
  EXPECT_EQ(e_before_throw, e.string());
  EXPECT_FALSE(g.IsInitialised());

  EXPECT_THROW(g += e, std::exception);
  EXPECT_EQ(e_before_throw, e.string());
  EXPECT_FALSE(g.IsInitialised());
}

TEST(BoundedStringTest, BEH_Serialization) {
  // Invalid Serialisation
  OneThree a;
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_THROW(maidsafe::ConvertToString(a), std::exception);

  // Valid Serialisation
  OneThree b {RandomString(1)};
  EXPECT_TRUE(b.IsInitialised());

  std::string serialised_str;
  EXPECT_EQ(0, serialised_str.size());
  EXPECT_NO_THROW(serialised_str = maidsafe::ConvertToString(b));
  EXPECT_NE(0, serialised_str.size());

  // Invalid Deserialisation
  TwoThree c;
  EXPECT_FALSE(c.IsInitialised());
  EXPECT_THROW(maidsafe::ConvertFromString(serialised_str, c), std::exception);

  // Valid Deserialisation
  OneTwo d;
  EXPECT_FALSE(d.IsInitialised());
  EXPECT_NO_THROW(maidsafe::ConvertFromString(serialised_str, d));
  EXPECT_EQ(b.string(), d.string());
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
