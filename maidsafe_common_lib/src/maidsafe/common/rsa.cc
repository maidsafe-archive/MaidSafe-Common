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

#include <memory>

#include "maidsafe/common/return_codes.h"

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif
#include "boost/scoped_array.hpp"
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

namespace rsa {

namespace {

CryptoPP::RandomNumberGenerator &rng() {
  static CryptoPP::AutoSeededRandomPool random_number_generator;
  return random_number_generator;
}

void EncodeKey(const CryptoPP::BufferedTransformation& bt, std::string *key) {
    CryptoPP::StringSink name(*key);
    bt.CopyTo(name);
    name.MessageEnd();
}

void Decode(const std::string& key, CryptoPP::BufferedTransformation *bt) {
    CryptoPP::StringSource file(key, true /*pumpAll*/);
    file.TransferTo(*bt);
    bt->MessageEnd();
}

}  // Unnamed namespace

int GenerateKeyPair(Keys *keypair) {
  CryptoPP::InvertibleRSAFunction parameters;
  try {
  parameters.GenerateRandomWithKeySize(rng(), Keys::KeySize);
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed Generating keypair: " << e.what();
    return kKeyGenerationError;
  }
  PrivateKey priv_key(parameters);
  PublicKey pub_key(parameters);
  keypair->priv_key = priv_key;
  keypair->pub_key = pub_key;
  if (keypair->priv_key.Validate(rng(), 2) &&
          keypair->pub_key.Validate(rng(), 2))
    return kSuccess;
  else
    return kGeneralError;
}


int Encrypt(const PlainText &data,
        const PublicKey &pub_key,
        CipherText *result
        ) {
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return kDataEmpty;
  }
  if (!pub_key.Validate(rng(), 0)) {
    DLOG(ERROR) << " Bad public key ";
    return kInvalidPublicKey;
  }
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(pub_key);
  CryptoPP::StringSource(data, true,
    new CryptoPP::PK_EncryptorFilter(rng(), encryptor,
      new CryptoPP::StringSink(*result)
    )  // PK_EncryptorFilterint GenerateKeyPair(RSAkeys* keypair) const NOLINT
      );  // StringSource NOLINT
  if (data == *result) {
    DLOG(ERROR) << " failed encryption ";
    return kRSAEncryptError;
  }
  return kSuccess;
}

int Decrypt(const CipherText &data,
        const PrivateKey& priv_key,
        PlainText *result) {
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return kDataEmpty;
  }
  if (!priv_key.Validate(rng(), 0)) {
    DLOG(ERROR) << " Bad private key ";
    return kInvalidPrivateKey;
  }
  CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(priv_key);
  std::string recovered;
  try {
  CryptoPP::StringSource(data, true,
  new CryptoPP::PK_DecryptorFilter(rng(), decryptor,
    new CryptoPP::StringSink(*result)
    )  // PK_DecryptorFilter NOLINT
    );  // StringSource NOLINT
  } catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "decryption failed";
    e.what();
    return kRSADecryptError;
  }
  if (data == *result) {
  DLOG(ERROR) << " failed decryption ";
  return kRSADecryptError;
  }
  return kSuccess;
}

int Sign(const std::string& data,
    const PrivateKey& priv_key,
    std::string  *signature) {
  if (!priv_key.Validate(rng(), 0)) {
    DLOG(ERROR) << " Bad private key ";
    return kInvalidPrivateKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return kDataEmpty;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(priv_key);
  try {
    CryptoPP::StringSource(data, true, new CryptoPP::SignerFilter(
                               rng(), signer,
                      new CryptoPP::StringSink(*signature)));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed asymmetric signing: " << e.what();
    return kRSASigningError;
  }
  return kSuccess;
}

int CheckSignature(const PlainText &data,
              const Signature &signature,
              const PublicKey &pub_key) {
  if (!pub_key.Validate(rng(), 0)) {
    DLOG(ERROR) << " Bad public key ";
    return kInvalidPublicKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << " No data ";
    return kDataEmpty;
  }
  if (signature.empty()) {
    DLOG(ERROR) << " No Signature ";
    return kRSASignatureEmpty;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(pub_key);
  try {
    if (verifier.VerifyMessage(reinterpret_cast<const byte *>(data.c_str()),
                             data.size(),
                             reinterpret_cast<const byte *>(signature.c_str()),
                             signature.size()))
      return kSuccess;
     else
       return kRSAInvalidsignature;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed signature check: " << e.what();
    return kRSAInvalidsignature;
  }
}

void EncodePrivateKey(const PrivateKey& key, std::string *priv_key) {
    CryptoPP::ByteQueue queue;
    key.DEREncodePrivateKey(queue);
    EncodeKey(queue, priv_key);
}

void EncodePublicKey(const PublicKey& key, std::string *pub_key) {
    CryptoPP::ByteQueue queue;
    key.DEREncodePublicKey(queue);
    EncodeKey(queue, pub_key);
}

void DecodePrivateKey(const std::string& priv_key, PrivateKey *key) {
    CryptoPP::ByteQueue queue;
    Decode(priv_key, &queue);
    key->BERDecodePrivateKey(queue,
                             false /*paramsPresent*/,
                             static_cast<size_t>(queue.MaxRetrievable()));
}

void DecodePublicKey(const std::string& pub_key, PublicKey *key) {
    CryptoPP::ByteQueue queue;
    Decode(pub_key, &queue);
    key->BERDecodePublicKey(queue,
                             false /*paramsPresent*/,
                             static_cast<size_t>(queue.MaxRetrievable()));
}

bool CheckRoundtrip(const maidsafe::rsa::PublicKey& public_key,
                    const maidsafe::rsa::PrivateKey& priv_key) {
  return (public_key.GetModulus() != priv_key.GetModulus() ||
          public_key.GetPublicExponent() != priv_key.GetPrivateExponent());
}

bool ValidateKey(const PrivateKey &priv_key) {
  return priv_key.Validate(rng(), 2);
}

bool ValidateKey(const PublicKey &pub_key) {
  return pub_key.Validate(rng(), 2);
}

void GetPublicKeyAndValidation(
    const Identity &/*id*/,
    GetPublicKeyAndValidationCallback callback) {
  callback("", "");
}

bool Validate(const PlainText &plain_text,
                                const Signature &signature,
                                const PublicKey &public_key
                               ) {
  if (0 == CheckSignature(plain_text, signature, public_key))
    return true;
  else
    return false;
}

bool MatchingPublicKeys(const PublicKey &pub_key_one,
                        const PublicKey &pub_key_two) {
  std::string enc_key_one;
  std::string enc_key_two;
  EncodePublicKey(pub_key_one, &enc_key_one);
  EncodePublicKey(pub_key_two, &enc_key_two);
  if (!enc_key_one.empty() && !enc_key_two.empty())
    return enc_key_one == enc_key_two;
  return false;
}

}  // namespace rsa

}  // namespace maidsafe
