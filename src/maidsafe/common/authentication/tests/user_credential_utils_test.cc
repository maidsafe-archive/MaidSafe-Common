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

#include "maidsafe/common/authentication/user_credential_utils.h"

#include "maidsafe/common/authentication/user_credentials.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"

namespace maidsafe {

namespace authentication {

namespace test {

TEST_CASE("CreateSecurePassword", "[UserCredentialUtils][Unit]") {
  UserCredentials user_credentials;
  const std::string kKeywordStr{ RandomAlphaNumericString((RandomUint32() % 100) +1) };
  const uint32_t kPinValue{ RandomUint32() };
  const std::string kPasswordStr{ RandomAlphaNumericString((RandomUint32() % 100) +1) };

  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(kPasswordStr);

  const crypto::SecurePassword kSecurePassword{ CreateSecurePassword(user_credentials) };
  REQUIRE(kSecurePassword->IsInitialised());
  CHECK(kSecurePassword->string() != kPasswordStr);

  // Modify keyword and check secure password is same
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr + 'a');
  crypto::SecurePassword copy_of_password{ CreateSecurePassword(user_credentials) };
  REQUIRE(copy_of_password->IsInitialised());
  CHECK(copy_of_password == kSecurePassword);

  // Modify pin and check secure password is different
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue + 1));
  crypto::SecurePassword modified_password0{ CreateSecurePassword(user_credentials) };
  REQUIRE(modified_password0->IsInitialised());
  CHECK(modified_password0 != kSecurePassword);

  // Modify password and check secure password is different
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(kPasswordStr + 'c');
  crypto::SecurePassword modified_password1{ CreateSecurePassword(user_credentials) };
  REQUIRE(modified_password1->IsInitialised());
  CHECK(modified_password1 != kSecurePassword);
  CHECK(modified_password1 != modified_password0);
}

TEST_CASE("ObfuscateData", "[UserCredentialUtils][Unit]") {
  UserCredentials user_credentials;
  const std::string kKeywordStr{ RandomAlphaNumericString((RandomUint32() % 100) + 1) };
  const uint32_t kPinValue{ RandomUint32() };
  const std::string kPasswordStr{ RandomAlphaNumericString((RandomUint32() % 100) + 1) };
  const NonEmptyString kData{ RandomString(1024 * 1024) };

  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(kPasswordStr);

  const NonEmptyString kObfuscated{ Obfuscate(user_credentials, kData) };
  REQUIRE(kObfuscated.IsInitialised());
  CHECK(kObfuscated.string().size() == kData.string().size());
  CHECK(kObfuscated != kData);
  CHECK(Obfuscate(user_credentials, kObfuscated) == kData);

  // Modify keyword and check obfuscated data is different
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr + 'a');
  NonEmptyString modified_obfuscated0{ Obfuscate(user_credentials, kData) };
  REQUIRE(modified_obfuscated0.IsInitialised());
  CHECK(modified_obfuscated0.string().size() == kData.string().size());
  CHECK(modified_obfuscated0 != kData);
  CHECK(Obfuscate(user_credentials, modified_obfuscated0) == kData);
  CHECK(modified_obfuscated0 != kObfuscated);

  // Modify pin and check obfuscated data is different
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue + 1));
  NonEmptyString modified_obfuscated1{ Obfuscate(user_credentials, kData) };
  REQUIRE(modified_obfuscated1.IsInitialised());
  CHECK(modified_obfuscated1.string().size() == kData.string().size());
  CHECK(modified_obfuscated1 != kData);
  CHECK(Obfuscate(user_credentials, modified_obfuscated1) == kData);
  CHECK(modified_obfuscated1 != kObfuscated);
  CHECK(modified_obfuscated1 != modified_obfuscated0);

  // Modify password and check obfuscated data is different
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(kPasswordStr + 'c');
  NonEmptyString modified_obfuscated2{ Obfuscate(user_credentials, kData) };
  REQUIRE(modified_obfuscated2.IsInitialised());
  CHECK(modified_obfuscated2.string().size() == kData.string().size());
  CHECK(modified_obfuscated2 != kData);
  CHECK(Obfuscate(user_credentials, modified_obfuscated2) == kData);
  CHECK(modified_obfuscated2 != kObfuscated);
  CHECK(modified_obfuscated2 != modified_obfuscated0);
  CHECK(modified_obfuscated2 != modified_obfuscated1);
}

}  // namespace test

}  // namespace authentication

}  // namespace maidsafe
