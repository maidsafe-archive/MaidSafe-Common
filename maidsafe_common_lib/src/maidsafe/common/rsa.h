/* Copyright (c) 2009 maidsafe.net limited
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

#ifndef MAIDSAFE_COMMON_RSA_H_
#define MAIDSAFE_COMMON_RSA_H_

#include <string>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif
#include "cryptopp/rsa.h"
#ifdef __MSVC__
#  pragma warning(pop)
#  pragma warning(disable: 4505)
#endif

#include "maidsafe/common/version.h"
#include "maidsafe/common/asymmetric_crypto.h"
#if MAIDSAFE_COMMON_VERSION != 1003
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif

namespace maidsafe {
namespace rsa {
  
typedef CryptoPP::RSA::PrivateKey PrivateKey;
typedef CryptoPP::RSA::PublicKey PublicKey;
typedef std::string ValidationToken, Identity, PlainText, Signature, CipherText;
typedef std::function<void(const std::string&, const std::string&)>
        GetPublicKeyAndValidationCallback;
  
struct Keys {
 public:
  enum { KeySize = 4096 };
  Keys() : identity(), priv_key(), pub_key(), validation_token() {}
  Identity identity;
  PrivateKey priv_key;
  PublicKey pub_key;
  ValidationToken validation_token;  // certificate, additional signature etc.
};

int GenerateKeyPair(Keys *keypair);

int Sign(const PlainText &data,
          const PrivateKey &priv_key,
          Signature *signature);

int CheckSignature(const PlainText &plain_text,
                   const Signature &signature,
                   const PublicKey &pub_key);

int Encrypt(const PlainText &plain_text,
            const PublicKey &pub_key,
              CipherText *result);

int Decrypt(const CipherText &data,
            const PrivateKey &priv_key,
            PlainText *result);

void EncodePrivateKey(const PrivateKey& key, std::string *priv_key);

void EncodePublicKey(const PublicKey& key, std::string *pub_key);

void DecodePrivateKey(std::string& priv_key, PrivateKey *key);

void DecodePublicKey(const std::string& pub_key, PublicKey *key);

// check decoded keys were the same as encoded and pub key not replaced
bool CheckRoundtrip(PublicKey &public_key, PrivateKey &priv_key);

bool ValidateKey(PrivateKey &priv_key);

bool ValidateKey(PublicKey &pub_key);

void GetPublicKeyAndValidation(
    const Identity &public_key_id,
    GetPublicKeyAndValidationCallback callback);

bool Validate(const PlainText &plain_text,
              const Signature &signature,
              const PublicKey &public_key);

}  // namespace rsa 
}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_RSA_H_
