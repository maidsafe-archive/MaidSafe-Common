/* Copyright (c) 2011 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_COMMON_SECURIFIER_H_
#define MAIDSAFE_COMMON_SECURIFIER_H_

#include <functional>
#include <string>
#include <vector>
#include "maidsafe/common/asymmetric_crypto.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1003
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace maidsafe {

typedef std::function<void(const std::string&, const std::string&)>
        GetPublicKeyAndValidationCallback;

/** Base class used to cryptographically secure and validate values and
 *  messages. */
template <typename Keys>

class Securifier {
 public:


  Securifier(std::shared_ptr<AsymmetricCrypto<Keys>> asymm);

  virtual ~Securifier();

  virtual int Sign(const std::string &value,
                   std::string *signature) const;

  virtual int AsymmetricEncrypt(
      const std::string &value,
      std::string *cipher,
      const std::string &recipient_public_key) const;

  virtual void GetPublicKeyAndValidation(const Keys::Identity &public_key_id,
                              Keys::PublicKey *public_key,
                              Keys::ValidationToken *public_key_validation);

  virtual void GetPublicKeyAndValidation(
      const std::string &public_key_id,
      GetPublicKeyAndValidationCallback callback);

  virtual bool Validate(const std::string &value,
                        const std::string &value_signature,
                        const std::string &public_key_id,
                        const std::string &public_key,
                        const std::string &public_key_validation,
                        const std::string &kademlia_key) const;

  
  virtual int AsymmetricDecrypt(
      const std::string &encrypted_value,
      std::string *data) const;


 private:
  Securifier& operator=(Securifier const&);
  AsymmetricCrypto Asymm_;
};

template<typename Keys>
int Securifier::Sign(const std::string &value, std::string *result) const {
  return Asymm_->Sign(value, result, sign_keys_.priv_key);
}







}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SECURIFIER_H_
