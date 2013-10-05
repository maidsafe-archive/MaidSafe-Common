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

#include "maidsafe/common/bounded_string.h"

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

TEST_CASE("BoundedString default constructor", "[Unit]") {
  OneOne a;
  CHECK_FALSE(a.IsInitialised());
  OneMax b;
  CHECK_FALSE(b.IsInitialised());
}

TEST_CASE("BoundedString getters", "[Unit]") {
  TwoTwo a;
  CHECK_FALSE(a.IsInitialised());
  CHECK_THROWS_AS(a.string(), std::exception);

  for (int i(0); i < 1000; ++i) {
    std::string random(RandomString((RandomUint32() % 1024) + 1));
    OneMax b(random);
    CHECK(b.IsInitialised());
    CHECK(random == b.string());
  }
}

TEST_CASE("BoundedString string constructor", "[Unit]") {
  // Empty (invalid)
  CHECK_THROWS_AS(OneOne a(""), std::exception);
  // Valid
  std::string random(RandomString(1));
  OneOne b(random);
  CHECK(random == b.string());
  // Too big
  CHECK_THROWS_AS(OneOne c(RandomString(2)), std::exception);

  // Empty (invalid)
  CHECK_THROWS_AS(OneMax d(""), std::exception);
  // Valid
  random = RandomString((RandomUint32() % 1024) + 1);
  OneMax e(random);
  CHECK(random == e.string());
}

TEST_CASE("BoundedString swap", "[Unit]") {
  // Swap with initialised
  std::string random1(RandomString(1));
  std::string random2(RandomString(2));
  OneTwo a(random1);
  OneTwo b(random2);
  swap(a, b);
  CHECK(random2 == a.string());
  CHECK(random1 == b.string());

  // Swap with uninitialised
  OneTwo c;
  swap(a, c);
  CHECK_FALSE(a.IsInitialised());
  CHECK(random2 == c.string());
}

TEST_CASE("BoundedString copy", "[Unit]") {
  // Copy from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b(a);
  CHECK(random == a.string());
  CHECK(random == b.string());

  // Copy from uninitialised
  OneMax c;
  OneMax d(c);
  CHECK_FALSE(d.IsInitialised());
}

TEST_CASE("BoundedString move", "[Unit]") {
  // Move from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(std::move(OneMax(random)));
  CHECK(random == a.string());

  // Move from uninitialised
  OneMax b(std::move(OneMax()));
  CHECK_FALSE(b.IsInitialised());
}

TEST_CASE("BoundedString assignment", "[Unit]") {
  // Assign from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b("1");
  b = a;
  CHECK(random == a.string());
  CHECK(random == b.string());

  // Assign from self
  b = b;
  CHECK(random == b.string());

  // Assign from uninitialised
  OneMax c;
  b = c;
  CHECK_FALSE(b.IsInitialised());
}

TEST_CASE("BoundedString from other BoundedString constructor", "[Unit]") {
  // Valid copy
  std::string random(RandomString(2));
  TwoThree a(random);
  OneMax b(a);
  CHECK(random == b.string());
  TwoFour c(a);
  CHECK(random == c.string());

  // Copy from uninitialised
  TwoThree d;
  OneThree e(d);
  CHECK_FALSE(e.IsInitialised());
  TwoFour f(d);
  CHECK_FALSE(f.IsInitialised());
}

TEST_CASE("BoundedString assignment from other BoundedString", "[Unit]") {
  // Valid assignment
  std::string random(RandomString(2));
  TwoThree a(random);
  OneMax b("1");
  b = a;
  CHECK(random == b.string());
  TwoFour c("02");
  c = a;
  CHECK(random == c.string());

  // Assign from uninitialised
  TwoThree d;
  OneThree e("1");
  e = d;
  CHECK_FALSE(e.IsInitialised());
  TwoFour f("02");
  f = d;
  CHECK_FALSE(f.IsInitialised());
}

TEST_CASE("BoundedString concatenation operators", "[Unit]") {
  OneOne a(RandomString(1));
  std::string a_before_throw = a.string();
  OneTwo b(RandomString(1));
  std::string b_before_throw = b.string();

  CHECK_THROWS_AS(a + b, std::exception);
  CHECK(a_before_throw == a.string());
  CHECK(b_before_throw == b.string());

  CHECK_THROWS_AS(a += b, std::exception);
  CHECK(a_before_throw == a.string());
  CHECK(b_before_throw == b.string());

  CHECK_THROWS_AS(a + a, std::exception);
  CHECK(a_before_throw == a.string());

  CHECK_THROWS_AS(a += a, std::exception);
  CHECK(a_before_throw == a.string());

  OneTwo c(b + a);
  CHECK(a_before_throw == a.string());
  CHECK(b_before_throw == b.string());
  auto concat(b_before_throw + a_before_throw);
  CHECK(concat == c.string());

  b += a;
  CHECK(a_before_throw == a.string());
  CHECK(c.string() == b.string());

  b = OneTwo(b_before_throw);

  OneTwo d(b + b);
  CHECK(b_before_throw == b.string());
  concat = b_before_throw + b_before_throw;
  CHECK(concat == d.string());

  b += b;
  CHECK(d.string() == b.string());

  b = OneTwo(b_before_throw);
  OneThree e(RandomString(1));
  std::string e_before_throw = e.string();

  OneThree f(e + b + b);
  CHECK(b_before_throw == b.string());
  concat = e_before_throw + b_before_throw + b_before_throw;
  CHECK(concat == f.string());

  e += (b + b);
  CHECK(b_before_throw == b.string());
  CHECK(f.string() == e.string());

  e = OneThree(e_before_throw);

  CHECK_THROWS_AS(e + b + b + a, std::exception);
  CHECK(a_before_throw == a.string());
  CHECK(b_before_throw == b.string());
  CHECK(e_before_throw == e.string());

  OneOne g;
  CHECK_THROWS_AS(e + g, std::exception);
  CHECK(e_before_throw == e.string());
  CHECK_FALSE(g.IsInitialised());

  CHECK_THROWS_AS(g + e, std::exception);
  CHECK(e_before_throw == e.string());
  CHECK_FALSE(g.IsInitialised());

  CHECK_THROWS_AS(e += g, std::exception);
  CHECK(e_before_throw == e.string());
  CHECK_FALSE(g.IsInitialised());

  CHECK_THROWS_AS(g += e, std::exception);
  CHECK(e_before_throw == e.string());
  CHECK_FALSE(g.IsInitialised());
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
