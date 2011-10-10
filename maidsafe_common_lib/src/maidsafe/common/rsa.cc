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

#include "maidsafe/common/rsa.h"
#include "maidsafe/common/asymmetric_crypto.h"
#include "maidsafe/common/return_codes.h"

#include <memory>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "boost/scoped_array.hpp"
#include "boost/thread/mutex.hpp"
#include "cryptopp/hex.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pssr.h"
#include "cryptopp/cryptlib.h"
#include "boost/assert.hpp"
#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/log.h"
#include "maidsafe/common/platform_config.h"


namespace maidsafe {


namespace {

CryptoPP::RandomNumberGenerator &rng() {
  static CryptoPP::AutoSeededRandomPool random_number_generator;
  return random_number_generator;
 }
}  // Unnamed namespace
  
template<>
int AsymmetricCrypto<RSAKeys>::GenerateKeyPair(RSAKeys *keypair) const
{
  CryptoPP::InvertibleRSAFunction parameters;
  try {
  parameters.GenerateRandomWithKeySize(rng(), RSAKeys::KeySize);
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed Generating keypair: " << e.what();
    return CommonReturnCode::kKeyGenerationError;
  }
  CryptoPP::RSA::PrivateKey priv_key(parameters);
  CryptoPP::RSA::PublicKey pub_key(parameters);
  keypair->priv_key = priv_key;
  keypair->pub_key = pub_key;
  if (keypair->priv_key.Validate(rng(), 2) &&
          keypair->pub_key.Validate(rng(), 2))
    return CommonReturnCode::kSuccess;
  else
    return CommonReturnCode::kGeneralError;
}


template<>
int AsymmetricCrypto<RSAKeys>::Encrypt(const std::string& data, std::string *result,
                 const PublicKey& pub_key) const
{
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return CommonReturnCode::kDataEmpty;
  }
  if (!pub_key.Validate(rng(),0)) {
    DLOG(ERROR) << " Bad public key ";
    return CommonReturnCode::kInvalidPublicKey;
  }
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(pub_key);
  CryptoPP::StringSource(data, true,
    new CryptoPP::PK_EncryptorFilter(rng(), encryptor,
      new CryptoPP::StringSink(*result)
    ) // PK_EncryptorFilterint GenerateKeyPair(RSAkeys* keypair) const
      ); // StringSource
  if (data == *result) {
    DLOG(ERROR) << " failed encryption ";
    return  CommonReturnCode::kRSAEncryptError;
  }
  return CommonReturnCode::kSuccess;
}

template<>
int AsymmetricCrypto<RSAKeys>::Decrypt(const std::string& data, std::string *result,
                 const PrivateKey& priv_key) const
{
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return CommonReturnCode::kDataEmpty;
  }
  if (!priv_key.Validate(rng(),0)) {
    DLOG(ERROR) << " Bad private key ";
    return CommonReturnCode::kInvalidPrivateKey;
  }
   CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(priv_key);
   std::string recovered;
   try {
   CryptoPP::StringSource(data, true,
    new CryptoPP::PK_DecryptorFilter(rng(), decryptor,
      new CryptoPP::StringSink(*result)
     ) // PK_DecryptorFilter
      ); // StringSource
   } catch (CryptoPP::Exception &e) {
     DLOG(ERROR) << "decryption failed";
     e.what();
     return CommonReturnCode::kRSADecryptError;
   }
   if (data == *result) {
    DLOG(ERROR) << " failed decryption ";
    return CommonReturnCode::kRSADecryptError;
   }
   return CommonReturnCode::kSuccess;
}

template<>
int AsymmetricCrypto<RSAKeys>::Sign(const std::string& data,
               std::string  *signature,
              const PrivateKey& priv_key) const
{
  if (!priv_key.Validate(rng(),0)) {
    DLOG(ERROR) << " Bad private key ";
    return CommonReturnCode::kInvalidPrivateKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return CommonReturnCode::kDataEmpty;
  }
  
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(priv_key);
  try {
    CryptoPP::StringSource(data, true, new CryptoPP::SignerFilter(
                               rng(), signer,
                      new CryptoPP::StringSink(*signature)));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed asymmetric signing: " << e.what();
    return CommonReturnCode::kRSASigningError;
  }
  return CommonReturnCode::kSuccess;
}


template<>
int AsymmetricCrypto<RSAKeys>::CheckSignature(const std::string& data,
                         const std::string& signature,
                        const PublicKey& pub_key) const
{
  if (!pub_key.Validate(rng(),0)) {
    DLOG(ERROR) << " Bad public key ";
    return CommonReturnCode::kInvalidPublicKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return CommonReturnCode::kDataEmpty;
  }
  if (signature.empty()) {
    DLOG(ERROR) << " No Signature ";
    return CommonReturnCode::kRSASignatureEmpty;
  }
  
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(pub_key);
  try {
    if (verifier.VerifyMessage(reinterpret_cast<const byte *>(data.c_str()),
                             data.size(),
                             reinterpret_cast<const byte *>(signature.c_str()),
                             signature.size()))
      return CommonReturnCode::kSuccess;
     else
       return CommonReturnCode::kRSAInvalidsignature;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed signature check: " << e.what();
    return CommonReturnCode::kRSAInvalidsignature;
  }
  return CommonReturnCode::kSuccess;
}

}  // namespace maidsafe
