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
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace authentication {

namespace test {

class UserCredentialsTest : public testing::Test {
 protected:
  UserCredentialsTest()
      : user_credentials(),
        kKeywordStr(RandomAlphaNumericBytes(1, 100)),
        kPinValue(RandomUint32()),
        kPasswordStr(RandomAlphaNumericBytes(1, 100)) {
    user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
    user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
    user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(kPasswordStr);
  }
  UserCredentials user_credentials;
  const std::vector<byte> kKeywordStr;
  const uint32_t kPinValue;
  const std::vector<byte> kPasswordStr;
};

TEST_F(UserCredentialsTest, BEH_CreateSecurePassword) {
  const crypto::SecurePassword kSecurePassword{CreateSecurePassword(user_credentials)};
  ASSERT_TRUE(kSecurePassword->IsInitialised());
  EXPECT_NE(kSecurePassword->string(), kPasswordStr);

  // Modify keyword and check secure password is same
  std::vector<byte> modified_bytes(kKeywordStr);
  modified_bytes.push_back(1);
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(modified_bytes);
  crypto::SecurePassword copy_of_password{CreateSecurePassword(user_credentials)};
  ASSERT_TRUE(copy_of_password->IsInitialised());
  EXPECT_EQ(copy_of_password, kSecurePassword);

  // Modify pin and check secure password is different
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue + 1));
  crypto::SecurePassword modified_password0{CreateSecurePassword(user_credentials)};
  ASSERT_TRUE(modified_password0->IsInitialised());
  EXPECT_NE(modified_password0, kSecurePassword);

  // Modify password and check secure password is different
  modified_bytes = kPasswordStr;
  modified_bytes.push_back(1);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(modified_bytes);
  crypto::SecurePassword modified_password1{CreateSecurePassword(user_credentials)};
  ASSERT_TRUE(modified_password1->IsInitialised());
  EXPECT_NE(modified_password1, kSecurePassword);
  EXPECT_NE(modified_password1, modified_password0);

  // Check validation of inputs
  user_credentials.keyword.reset();
  EXPECT_NO_THROW(CreateSecurePassword(user_credentials));
  user_credentials.pin.reset();
  EXPECT_THROW(CreateSecurePassword(user_credentials), common_error);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password.reset();
  EXPECT_THROW(CreateSecurePassword(user_credentials), common_error);
}

TEST_F(UserCredentialsTest, FUNC_ObfuscateData) {
  const NonEmptyString kData{RandomString(1024 * 1024)};
  const NonEmptyString kObfuscated{Obfuscate(user_credentials, kData)};
  ASSERT_TRUE(kObfuscated.IsInitialised());
  EXPECT_EQ(kObfuscated.string().size(), kData.string().size());
  EXPECT_NE(kObfuscated, kData);
  EXPECT_EQ(Obfuscate(user_credentials, kObfuscated), kData);

  // Modify keyword and check obfuscated data is different
  std::vector<byte> modified_bytes(kKeywordStr);
  modified_bytes.push_back(1);
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(modified_bytes);
  NonEmptyString modified_obfuscated0{Obfuscate(user_credentials, kData)};
  ASSERT_TRUE(modified_obfuscated0.IsInitialised());
  EXPECT_EQ(modified_obfuscated0.string().size(), kData.string().size());
  EXPECT_NE(modified_obfuscated0, kData);
  EXPECT_EQ(Obfuscate(user_credentials, modified_obfuscated0), kData);
  EXPECT_NE(modified_obfuscated0, kObfuscated);

  // Modify pin and check obfuscated data is different
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue + 1));
  NonEmptyString modified_obfuscated1{Obfuscate(user_credentials, kData)};
  ASSERT_TRUE(modified_obfuscated1.IsInitialised());
  EXPECT_EQ(modified_obfuscated1.string().size(), kData.string().size());
  EXPECT_NE(modified_obfuscated1, kData);
  EXPECT_EQ(Obfuscate(user_credentials, modified_obfuscated1), kData);
  EXPECT_NE(modified_obfuscated1, kObfuscated);
  EXPECT_NE(modified_obfuscated1, modified_obfuscated0);

  // Modify password and check obfuscated data is different
  modified_bytes = kPasswordStr;
  modified_bytes.push_back(1);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password = maidsafe::make_unique<UserCredentials::Password>(modified_bytes);
  NonEmptyString modified_obfuscated2{Obfuscate(user_credentials, kData)};
  ASSERT_TRUE(modified_obfuscated2.IsInitialised());
  EXPECT_EQ(modified_obfuscated2.string().size(), kData.string().size());
  EXPECT_NE(modified_obfuscated2, kData);
  EXPECT_EQ(Obfuscate(user_credentials, modified_obfuscated2), kData);
  EXPECT_NE(modified_obfuscated2, kObfuscated);
  EXPECT_NE(modified_obfuscated2, modified_obfuscated0);
  EXPECT_NE(modified_obfuscated2, modified_obfuscated1);

  // Check validation of inputs
  user_credentials.keyword.reset();
  EXPECT_THROW(Obfuscate(user_credentials, kData), common_error);
  user_credentials.keyword = maidsafe::make_unique<UserCredentials::Keyword>(kKeywordStr);
  user_credentials.pin.reset();
  EXPECT_THROW(Obfuscate(user_credentials, kData), common_error);
  user_credentials.pin = maidsafe::make_unique<UserCredentials::Pin>(std::to_string(kPinValue));
  user_credentials.password.reset();
  EXPECT_THROW(Obfuscate(user_credentials, kData), common_error);
}

}  // namespace test

}  // namespace authentication

}  // namespace maidsafe
