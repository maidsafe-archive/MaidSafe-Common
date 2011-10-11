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
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/asymmetric_crypto.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1003
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace maidsafe {

typedef std::function<void(const std::string&, const std::string&)>
        GetPublicKeyAndValidationCallback;

/// Base class used to cryptographically secure and validate values and
///  messages.
template <typename Keys>
class Securifier {
 public:


  Securifier(std::shared_ptr<AsymmetricCrypto<Keys> >asymm) : asymm_(asymm) {}

  virtual ~Securifier();

  virtual int Sign(const PlainText &value,
                   Signature *signature) const;

  virtual int AsymmetricEncrypt(
                                const PlainText &value,
                                const typename Keys::PublicKey &public_key,
                                CipherText *cipher) const;

  virtual void GetPublicKeyAndValidation(const typename Keys::Identity &id,
                                  typename Keys::PublicKey *public_key,
                                  typename  Keys::ValidationToken *validation);

  virtual void GetPublicKeyAndValidation(
      const  typename Keys::Identity &public_key_id,
      GetPublicKeyAndValidationCallback callback) const;

  virtual bool Validate(const PlainText &plain_text,
                        const Signature &signature,
                        const typename Keys::PublicKey &public_key
                        ) const;
                                       
  virtual int AsymmetricDecrypt(
      const CipherText &encrypted_value,
      PlainText *data) const;

 private:
  Securifier& operator=(Securifier const&);
  AsymmetricCrypto<Keys> asymm_;
};

template<typename Keys>
int Securifier<Keys>::Sign(const PlainText &value, Signature *result)
                                                               const {
  return asymm_->Sign(value, result, asymm_->Keys.PublicKey);
}

template<typename Keys>
int Securifier<Keys>::AsymmetricEncrypt(const PlainText &plain_text,
                                        const typename Keys::PublicKey &key,
                                        CipherText *cipher_text) const {
  return asymm_->Encrypt(plain_text, key, cipher_text);
}

template<typename Keys>
int Securifier<Keys>::AsymmetricDecrypt(const CipherText &encrypted_value,
                                        PlainText *data) const {
  return  asymm_->Decrypt(encrypted_value, data);
}


template<typename Keys>
void Securifier<Keys>::GetPublicKeyAndValidation(
    const typename Keys::Identity &id,
    GetPublicKeyAndValidationCallback callback) const {
  callback("", "");
}

template<typename Keys>
bool Securifier<Keys>::Validate(const PlainText &plain_text,
                                const Signature &signature,
                                const typename Keys::PublicKey &public_key
                               ) const {
  if (0 == asymm_->CheckSignature(plain_text, signature, public_key))
    return true;
  else
    return false;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SECURIFIER_H_
