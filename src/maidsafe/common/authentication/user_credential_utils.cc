
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
  crypto::Salt salt{crypto::Hash<crypto::SHA512>(user_credentials.pin->Hash<crypto::SHA512>() + user_credentials.password->string())};
  return crypto::CreateSecurePassword(*user_credentials.password, salt,
                                      static_cast<uint32_t>(user_credentials.pin->Value()));
}

NonEmptyString Obfuscate(const UserCredentials& user_credentials, const NonEmptyString& data) {
  if (!user_credentials.keyword || !user_credentials.pin || !user_credentials.password) {
    LOG(kError) << "UserCredentials is not initialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  crypto::Salt salt{crypto::Hash<crypto::SHA512>(user_credentials.password->string() +
                                                 user_credentials.pin->Hash<crypto::SHA512>())};
  std::string obfuscation_str{
      crypto::CreateSecurePassword(*user_credentials.keyword, salt,
                                   static_cast<uint32_t>(user_credentials.pin->Value() * 2))
          ->string()};

  // make the obfuscation_str of same size for XOR
  if (data.string().size() < obfuscation_str.size()) {
    obfuscation_str.resize(data.string().size());
  } else if (data.string().size() > obfuscation_str.size()) {
    obfuscation_str.reserve(data.string().size());
    while (data.string().size() > obfuscation_str.size())
      obfuscation_str += obfuscation_str;
    obfuscation_str.resize(data.string().size());
  }
  return NonEmptyString{crypto::XOR(data.string(), obfuscation_str)};
}

crypto::AES256Key DeriveSymmEncryptKey(const crypto::SecurePassword& secure_password) {
  return crypto::AES256Key{secure_password->string().substr(0, crypto::AES256_KeySize)};
}

crypto::AES256InitialisationVector DeriveSymmEncryptIv(
    const crypto::SecurePassword& secure_password) {
  return crypto::AES256InitialisationVector{
      secure_password->string().substr(crypto::AES256_KeySize, crypto::AES256_IVSize)};
}

}  // namespace authentication

}  // namespace maidsafe
