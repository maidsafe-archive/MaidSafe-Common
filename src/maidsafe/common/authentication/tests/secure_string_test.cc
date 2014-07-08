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

TEST(SecureStringTest, BEH_Construct) {
  SecureString secure_string;

  EXPECT_NO_THROW(secure_string.Append('p'));
  EXPECT_NO_THROW(secure_string.Append('a'));
  EXPECT_NO_THROW(secure_string.Append('s'));
  EXPECT_NO_THROW(secure_string.Append('s'));
  EXPECT_NO_THROW(secure_string.Append('w'));
  EXPECT_NO_THROW(secure_string.Append('o'));
  EXPECT_NO_THROW(secure_string.Append('r'));
  EXPECT_NO_THROW(secure_string.Append('d'));
  EXPECT_NO_THROW(secure_string.Finalise());

  ASSERT_TRUE(SafeString("password") == secure_string.string());
}

TEST(SecureStringTest, BEH_Hash) {
//  typedef maidsafe::detail::BoundedString<crypto::SHA512::DIGESTSIZE, crypto::SHA512::DIGESTSIZE>
//      BoundedString;
  SafeString string("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_NO_THROW(crypto::Hash<crypto::SHA512>(string));
}

TEST(SecureStringTest, BEH_CreatePassword) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Remove(2, 3));
  EXPECT_NO_THROW(password.Insert(2, 'l'));
  EXPECT_NO_THROW(password.Insert(2, 'y'));
  EXPECT_NO_THROW(password.Remove(5, 1));
  EXPECT_NO_THROW(password.Insert(5, 'a'));

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("payload") == password.string());
}

TEST(SecureStringTest, BEH_CreatePasswordString) {
  SafeString safe_password("password");
  EXPECT_NO_THROW(Password password(safe_password));
  std::string std_password("drowssap");
  EXPECT_NO_THROW(Password password(std_password));

  {
    Password password(safe_password);
    ASSERT_TRUE(SafeString("password") == password.string());

    EXPECT_NO_THROW(password.Insert(safe_password.size(), std_password));

    EXPECT_NO_THROW(password.Finalise());

    ASSERT_TRUE(SafeString("passworddrowssap") == password.string());
  }

  {
    Password password;
    EXPECT_NO_THROW(password.Insert(0, safe_password));
    EXPECT_NO_THROW(password.Insert(1, std_password));

    EXPECT_NO_THROW(password.Finalise());

    ASSERT_TRUE(SafeString("passworddrowssap") == password.string());
  }
}


TEST(SecureStringTest, BEH_PassEmptyPasswordString) {
  // Passwords are currently defined to have length at least 1 character.
  SafeString safe_password;
  EXPECT_THROW(Password password(safe_password), std::exception);
  std::string std_password;
  EXPECT_THROW(Password password(std_password), std::exception);
}

TEST(SecureStringTest, BEH_RemoveFirstPasswordCharacter) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Remove(0, 1));

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("assword") == password.string());
}

TEST(SecureStringTest, BEH_RemoveLastPasswordCharacter) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Remove(7, 1));

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("passwor") == password.string());
}

TEST(SecureStringTest, BEH_InsertAndRemoveAfterPasswordFinalised) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Finalise());

  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Remove(0, 1));

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("password") == password.string());
}

TEST(SecureStringTest, BEH_CreatePasswordWithMissingIndex) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(8, 'd'));
  EXPECT_NO_THROW(password.Insert(5, 'w'));
  EXPECT_NO_THROW(password.Insert(7, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(6, 'o'));

  EXPECT_THROW(password.Finalise(), std::exception);
  EXPECT_NO_THROW(password.Insert(4, 'D'));
  EXPECT_NO_THROW(password.Finalise());
  ASSERT_TRUE(SafeString("passDword") == password.string());
}

TEST(SecureStringTest, BEH_CreateInvalidLengthPassword) {
  Password password;
  EXPECT_THROW(password.Finalise(), std::exception);
}

TEST(SecureStringTest, BEH_ClearPasswordThenRedo) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Clear());

  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));

  EXPECT_NO_THROW(password.Finalise());

  EXPECT_NO_THROW(password.Remove(7, 1));
  EXPECT_NO_THROW(password.Remove(2, 1));
  EXPECT_NO_THROW(password.Remove(4, 1));
  EXPECT_NO_THROW(password.Remove(4, 1));
  EXPECT_NO_THROW(password.Remove(1, 1));
  EXPECT_NO_THROW(password.Remove(2, 1));
  EXPECT_NO_THROW(password.Remove(1, 1));
  EXPECT_NO_THROW(password.Remove(0, 1));

  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("password") == password.string());
}

TEST(SecureStringTest, BEH_ClearPasswordAfterFinalised) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_NO_THROW(password.Finalise());

  EXPECT_NO_THROW(password.Clear());

  EXPECT_THROW(password.Finalise(), std::exception);
  EXPECT_THROW(password.string(), std::exception);
}

TEST(SecureStringTest, BEH_GetPasswordTextBeforeFinalised) {
  Password password;

  EXPECT_NO_THROW(password.Insert(3, 's'));
  EXPECT_NO_THROW(password.Insert(7, 'd'));
  EXPECT_NO_THROW(password.Insert(4, 'w'));
  EXPECT_NO_THROW(password.Insert(6, 'r'));
  EXPECT_NO_THROW(password.Insert(1, 'a'));
  EXPECT_NO_THROW(password.Insert(0, 'p'));
  EXPECT_NO_THROW(password.Insert(2, 's'));
  EXPECT_NO_THROW(password.Insert(5, 'o'));

  EXPECT_THROW(password.string(), std::exception);

  EXPECT_NO_THROW(password.Finalise());

  ASSERT_TRUE(SafeString("password") == password.string());
}

TEST(SecureStringTest, BEH_CheckPasswordIsValidForAllChars) {
  Password password;
  for (size_t i(0); i != 23; ++i)
    EXPECT_NO_THROW(password.Insert(i, static_cast<char>(RandomInt32())));

  ASSERT_TRUE(password.IsValid(boost::regex(".")));

  EXPECT_NO_THROW(password.Finalise());
}

TEST(SecureStringTest, BEH_CreatePin) {
  Pin pin;

  EXPECT_NO_THROW(pin.Insert(1, '1'));
  EXPECT_NO_THROW(pin.Insert(3, '3'));
  EXPECT_NO_THROW(pin.Insert(0, '0'));
  EXPECT_NO_THROW(pin.Insert(2, '2'));

  EXPECT_NO_THROW(pin.Finalise());

  ASSERT_TRUE(SafeString("0123") == pin.string());
  ASSERT_TRUE(123 == pin.Value());
}

TEST(SecureStringTest, BEH_CreateInvalidLengthPin) {
  Pin pin;

  EXPECT_THROW(pin.Finalise(), std::exception);

  EXPECT_NO_THROW(pin.Insert(0, '0'));

  EXPECT_NO_THROW(pin.Finalise());

  ASSERT_TRUE(SafeString("0") == pin.string());
}

TEST(SecureStringTest, BEH_InsertInvalidPinValue) {
  Pin pin;

  EXPECT_NO_THROW(pin.Insert(1, '1'));
  EXPECT_NO_THROW(pin.Insert(3, '3'));
  EXPECT_NO_THROW(pin.Insert(0, 'a'));
  EXPECT_NO_THROW(pin.Insert(2, '2'));

  EXPECT_NO_THROW(pin.Finalise());

  ASSERT_TRUE(SafeString("a123") == pin.string());
  EXPECT_TRUE(pin.IsValid(boost::regex(".")));
  EXPECT_THROW(pin.Value(), std::exception);

  EXPECT_NO_THROW(pin.Remove(0, 1));
  EXPECT_NO_THROW(pin.Insert(0, '0'));
  EXPECT_NO_THROW(pin.Finalise());
  EXPECT_TRUE(pin.IsValid(boost::regex(".")));

  EXPECT_NO_THROW(pin.Finalise());
  ASSERT_TRUE(123 == pin.Value());
}

}  // namespace test

}  // namespace detail

}  // namespace authentication

}  // namespace maidsafe
