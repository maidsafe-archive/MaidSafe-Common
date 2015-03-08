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

#include <cstdint>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/authentication/user_credentials.h"

namespace maidsafe {

namespace authentication {

crypto::SecurePassword CreateSecurePassword(const UserCredentials& user_credentials) {
  if (!user_credentials.pin || !user_credentials.password) {
    LOG(kError) << "UserCredentials is not initialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  crypto::Salt salt{crypto::Hash<crypto::SHA512>(user_credentials.pin->Hash<crypto::SHA512>() +
                                                 user_credentials.password->string())};
  return crypto::CreateSecurePassword(*user_credentials.password, salt,
                                      static_cast<uint32_t>(user_credentials.pin->Value()));
}

NonEmptyString Obfuscate(const UserCredentials& user_credentials, const NonEmptyString& data) {
  if (!user_credentials.keyword || !user_credentials.pin || !user_credentials.password) {
    LOG(kError) << "UserCredentials is not initialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  crypto::Salt salt(crypto::Hash<crypto::SHA512>(user_credentials.password->string() +
                                                 user_credentials.pin->Hash<crypto::SHA512>()));
  auto obfuscation_str(crypto::CreateSecurePassword(
                           *user_credentials.keyword, salt,
                           static_cast<uint32_t>(user_credentials.pin->Value() * 2))->string());

  // make the obfuscation_str of same size for XOR
  const std::size_t data_size(data.string().size());
  if (data_size < obfuscation_str.size()) {
    obfuscation_str.resize(data_size);
  } else if (data_size > obfuscation_str.size()) {
    obfuscation_str.reserve(data_size);
    while (data_size > obfuscation_str.size())
      obfuscation_str.insert(obfuscation_str.end(), obfuscation_str.begin(), obfuscation_str.end());
    obfuscation_str.resize(data_size);
  }

  // XOR the data and the obfuscation string
  std::vector<byte> result(data_size, 0);
  for (std::size_t i(0); i < data_size; ++i)
    result[i] = data.string()[i] ^ obfuscation_str[i];

  return NonEmptyString(result);
}

}  // namespace authentication

}  // namespace maidsafe
