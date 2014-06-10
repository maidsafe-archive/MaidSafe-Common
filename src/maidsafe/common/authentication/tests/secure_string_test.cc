/*  Copyright 2013 MaidSafe.net limited

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

#include "maidsafe/common/authentication/detail/secure_string.h"

#include "boost/regex.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/bounded_string.h"

namespace maidsafe {

namespace authentication {

namespace detail {

namespace test {

typedef SecureInputString<std::greater_equal<SecureString::size_type>, 1, PasswordTag> Password;
typedef SecureInputString<std::greater_equal<SecureString::size_type>, 1, PinTag> Pin;

TEST_CASE("Create SecureString", "[SecureString][Unit]") {
  SecureString secure_string;

  CHECK_NOTHROW(secure_string.Append('p'));
  CHECK_NOTHROW(secure_string.Append('a'));
  CHECK_NOTHROW(secure_string.Append('s'));
  CHECK_NOTHROW(secure_string.Append('s'));
  CHECK_NOTHROW(secure_string.Append('w'));
  CHECK_NOTHROW(secure_string.Append('o'));
  CHECK_NOTHROW(secure_string.Append('r'));
  CHECK_NOTHROW(secure_string.Append('d'));
  CHECK_NOTHROW(secure_string.Finalise());

  REQUIRE(SafeString("password") == secure_string.string());
}

TEST_CASE("Hash SecureString string", "[SecureString][Unit]") {
//  typedef maidsafe::detail::BoundedString<crypto::SHA512::DIGESTSIZE, crypto::SHA512::DIGESTSIZE>
//      BoundedString;
  SafeString string("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  CHECK_NOTHROW(crypto::Hash<crypto::SHA512>(string));
}

TEST_CASE("Create Password", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Remove(2, 3));
  CHECK_NOTHROW(password.Insert(2, 'l'));
  CHECK_NOTHROW(password.Insert(2, 'y'));
  CHECK_NOTHROW(password.Remove(5, 1));
  CHECK_NOTHROW(password.Insert(5, 'a'));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("payload") == password.string());
}

TEST_CASE("Create Password string", "[SecureString][Unit]") {
  SafeString safe_password("password");
  CHECK_NOTHROW(Password password(safe_password));
  std::string std_password("drowssap");
  CHECK_NOTHROW(Password password(std_password));

  {
    Password password(safe_password);
    REQUIRE(SafeString("password") == password.string());

    CHECK_NOTHROW(password.Insert(safe_password.size(), std_password));

    CHECK_NOTHROW(password.Finalise());

    REQUIRE(SafeString("passworddrowssap") == password.string());
  }

  {
    Password password;
    CHECK_NOTHROW(password.Insert(0, safe_password));
    CHECK_NOTHROW(password.Insert(1, std_password));

    CHECK_NOTHROW(password.Finalise());

    REQUIRE(SafeString("passworddrowssap") == password.string());
  }
}


TEST_CASE("Pass empty Password string", "[SecureString][Unit]") {
  // Password's are currently defined to have length at least 1 character.
  SafeString safe_password;
  CHECK_THROWS_AS(Password password(safe_password), std::exception);
  std::string std_password;
  CHECK_THROWS_AS(Password password(std_password), std::exception);
}

TEST_CASE("Remove first Password character", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Remove(0, 1));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("assword") == password.string());
}

TEST_CASE("Remove last Password character", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Remove(7, 1));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("passwor") == password.string());
}

TEST_CASE("Insert and remove after Password is finalised", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Finalise());

  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Remove(0, 1));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("password") == password.string());
}

TEST_CASE("Create Password with missing index", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(8, 'd'));
  CHECK_NOTHROW(password.Insert(5, 'w'));
  CHECK_NOTHROW(password.Insert(7, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(6, 'o'));

  CHECK_THROWS_AS(password.Finalise(), std::exception);

  CHECK_NOTHROW(password.Insert(4, 'D'));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("passDword") == password.string());
}

TEST_CASE("Create invalid length Password", "[SecureString][Unit]") {
  Password password;

  CHECK_THROWS_AS(password.Finalise(), std::exception);
}

TEST_CASE("Clear Password then redo", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Clear());

  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));
  CHECK_NOTHROW(password.Insert(4, 'w'));

  CHECK_NOTHROW(password.Finalise());

  CHECK_NOTHROW(password.Remove(7, 1));
  CHECK_NOTHROW(password.Remove(2, 1));
  CHECK_NOTHROW(password.Remove(4, 1));
  CHECK_NOTHROW(password.Remove(4, 1));
  CHECK_NOTHROW(password.Remove(1, 1));
  CHECK_NOTHROW(password.Remove(2, 1));
  CHECK_NOTHROW(password.Remove(1, 1));
  CHECK_NOTHROW(password.Remove(0, 1));

  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));
  CHECK_NOTHROW(password.Insert(4, 'w'));

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("password") == password.string());
}

TEST_CASE("Clear Password after it's finalised", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_NOTHROW(password.Finalise());

  CHECK_NOTHROW(password.Clear());

  CHECK_THROWS_AS(password.Finalise(), std::exception);
  CHECK_THROWS_AS(password.string(), std::exception);
}

TEST_CASE("Get Password text before it's finalised", "[SecureString][Unit]") {
  Password password;

  CHECK_NOTHROW(password.Insert(3, 's'));
  CHECK_NOTHROW(password.Insert(7, 'd'));
  CHECK_NOTHROW(password.Insert(4, 'w'));
  CHECK_NOTHROW(password.Insert(6, 'r'));
  CHECK_NOTHROW(password.Insert(1, 'a'));
  CHECK_NOTHROW(password.Insert(0, 'p'));
  CHECK_NOTHROW(password.Insert(2, 's'));
  CHECK_NOTHROW(password.Insert(5, 'o'));

  CHECK_THROWS_AS(password.string(), std::exception);

  CHECK_NOTHROW(password.Finalise());

  REQUIRE(SafeString("password") == password.string());
}

TEST_CASE("Check Password is valid for all chars", "[SecureString][Unit]") {
  Password password;
  for (size_t i(0); i != 23; ++i)
    CHECK_NOTHROW(password.Insert(i, static_cast<char>(RandomInt32())));

  REQUIRE(password.IsValid(boost::regex(".")));

  CHECK_NOTHROW(password.Finalise());
}

TEST_CASE("Create PIN", "[SecureString][Unit]") {
  Pin pin;

  CHECK_NOTHROW(pin.Insert(1, '1'));
  CHECK_NOTHROW(pin.Insert(3, '3'));
  CHECK_NOTHROW(pin.Insert(0, '0'));
  CHECK_NOTHROW(pin.Insert(2, '2'));

  CHECK_NOTHROW(pin.Finalise());

  REQUIRE(SafeString("0123") == pin.string());
  REQUIRE(123 == pin.Value());
}

TEST_CASE("Create invalid length PIN", "[SecureString][Unit]") {
  Pin pin;

  CHECK_THROWS_AS(pin.Finalise(), std::exception);

  CHECK_NOTHROW(pin.Insert(0, '0'));

  CHECK_NOTHROW(pin.Finalise());

  REQUIRE(SafeString("0") == pin.string());
}

TEST_CASE("Insert invalid PIN value", "[SecureString][Unit]") {
  Pin pin;

  CHECK_NOTHROW(pin.Insert(1, '1'));
  CHECK_NOTHROW(pin.Insert(3, '3'));
  CHECK_NOTHROW(pin.Insert(0, 'a'));
  CHECK_NOTHROW(pin.Insert(2, '2'));

  CHECK_NOTHROW(pin.Finalise());

  REQUIRE(SafeString("a123") == pin.string());
  CHECK(pin.IsValid(boost::regex(".")));
  CHECK_THROWS_AS(pin.Value(), std::exception);

  CHECK_NOTHROW(pin.Remove(0, 1));
  CHECK_NOTHROW(pin.Insert(0, '0'));
  CHECK_NOTHROW(pin.Finalise());
  CHECK(pin.IsValid(boost::regex(".")));

  CHECK_NOTHROW(pin.Finalise());
  REQUIRE(123 == pin.Value());
}

}  // namespace test

}  // namespace detail

}  // namespace authentication

}  // namespace maidsafe
