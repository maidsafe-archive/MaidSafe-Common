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

#include "maidsafe/common/platform_config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/safe_enc.pb.h"


namespace maidsafe {

namespace rsa {

namespace {

CryptoPP::RandomNumberGenerator &rng() {
  static CryptoPP::AutoSeededRandomPool random_number_generator;
  return random_number_generator;
}

void EncodeKey(const CryptoPP::BufferedTransformation &bt, std::string *key) {
  CryptoPP::StringSink name(*key);
  bt.CopyTo(name);
  name.MessageEnd();
}

void DecodeKey(const std::string &key, CryptoPP::BufferedTransformation *bt) {
  CryptoPP::StringSource file(key, true /*pumpAll*/);
  file.TransferTo(*bt);
  bt->MessageEnd();
}

}  // Unnamed namespace

int GenerateKeyPair(Keys *keypair) {
  CryptoPP::InvertibleRSAFunction parameters;
  try {
    parameters.GenerateRandomWithKeySize(rng(), Keys::kKeySize);
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed generating keypair: " << e.what();
    return kKeyGenerationError;
  }
  PrivateKey private_key(parameters);
  PublicKey public_key(parameters);
  keypair->private_key = private_key;
  keypair->public_key = public_key;
  if (keypair->private_key.Validate(rng(), 2) &&
      keypair->public_key.Validate(rng(), 2))
    return kSuccess;
  else
    return kGeneralError;
}

int Encrypt(const PlainText &data,
            const PublicKey &public_key,
            CipherText *result) {
  if (data.empty()) {
    DLOG(ERROR) << "No data";
    return kDataEmpty;
  }
  if (!public_key.Validate(rng(), 0)) {
    DLOG(ERROR) << "Bad public key";
    return kInvalidPublicKey;
  }
  SafeEncrypt safe_enc;
  std::string symm_encryption_key(RandomString(crypto::AES256_KeySize));
  std::string symm_encryption_iv(RandomString(crypto::AES256_IVSize));
  safe_enc.set_data(crypto::SymmEncrypt(data,
                                        symm_encryption_key,
                                        symm_encryption_iv));
  std::string encryption_key_encrypted;
  
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(public_key);
  try {
    CryptoPP::StringSource(symm_encryption_key + symm_encryption_iv , true,
        new CryptoPP::PK_EncryptorFilter(rng(), encryptor,
            new CryptoPP::StringSink(encryption_key_encrypted)));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed encryption: " << e.what();
    return kRSAEncryptError;
  }
  safe_enc.set_key(encryption_key_encrypted);
  safe_enc.SerializeToString(result);
  return kSuccess;
}

int Decrypt(const CipherText &data,
            const PrivateKey &private_key,
            PlainText *result) {
  if (data.empty()) {
    DLOG(ERROR) << "No data";
    return kDataEmpty;
  }
  if (!private_key.Validate(rng(), 0)) {
    DLOG(ERROR) << "Bad private key";
    return kInvalidPrivateKey;
  }
  SafeEncrypt safe_enc;
  
  if (!safe_enc.ParseFromString(data)) {
        DLOG(ERROR) << "Cannot parse PB";
        return kRSADecryptError;
  }
  
  CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key);
  std::string sym_enc_data;
  std::string temp_data = safe_enc.key();
  try {
    CryptoPP::StringSource(temp_data, true,
        new CryptoPP::PK_DecryptorFilter(rng(), decryptor,
            new CryptoPP::StringSink(sym_enc_data)));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed decryption: " << e.what();
    return kRSADecryptError;
  }

  *result = (crypto::SymmDecrypt(safe_enc.data(),
                              sym_enc_data.substr(0, crypto::AES256_KeySize),
                              sym_enc_data.substr(crypto::AES256_KeySize,
                                                  crypto::AES256_IVSize)));
  if (*result == "")
    return kRSADecryptError;
  return kSuccess;
}

int Sign(const std::string &data,
         const PrivateKey &private_key,
         std::string *signature) {
  if (!private_key.Validate(rng(), 0)) {
    DLOG(ERROR) << "Bad private key";
    return kInvalidPrivateKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << "No data";
    return kDataEmpty;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::StringSource(data, true,
        new CryptoPP::SignerFilter(rng(), signer,
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
                   const PublicKey &public_key) {
  if (!public_key.Validate(rng(), 0)) {
    DLOG(ERROR) << "Bad public key";
    return kInvalidPublicKey;
  }
  if (data.empty()) {
    DLOG(ERROR) << "No data";
    return kDataEmpty;
  }
  if (signature.empty()) {
    DLOG(ERROR) << "No signature";
    return kRSASignatureEmpty;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier
      verifier(public_key);
  try {
    if (verifier.VerifyMessage(reinterpret_cast<const byte*>(data.c_str()),
                               data.size(),
                               reinterpret_cast<const byte*>(signature.c_str()),
                               signature.size()))
       return kSuccess;
     else
       return kRSAInvalidSignature;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed signature check: " << e.what();
    return kRSAInvalidSignature;
  }
}

void EncodePrivateKey(const PrivateKey &key, std::string *private_key) {
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePrivateKey(queue);
    EncodeKey(queue, private_key);
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << e.what();
    private_key->clear();
  }
}

void EncodePublicKey(const PublicKey &key, std::string *public_key) {
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePublicKey(queue);
    EncodeKey(queue, public_key);
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << e.what();
    public_key->clear();
  }
}

void DecodePrivateKey(const std::string &private_key, PrivateKey *key) {
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(private_key, &queue);
    key->BERDecodePrivateKey(queue,
                              false /*paramsPresent*/,
                              static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << e.what();
    *key = PrivateKey();
  }
}

void DecodePublicKey(const std::string &public_key, PublicKey *key) {
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(public_key, &queue);
    key->BERDecodePublicKey(queue,
                            false /*paramsPresent*/,
                            static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << e.what();
    *key = PublicKey();
  }
}

bool CheckRoundtrip(const PublicKey &public_key,
                    const PrivateKey &private_key) {
  return (public_key.GetModulus() != private_key.GetModulus() ||
          public_key.GetPublicExponent() != private_key.GetPrivateExponent());
}

bool ValidateKey(const PrivateKey &private_key) {
  return private_key.Validate(rng(), 2);
}

bool ValidateKey(const PublicKey &public_key) {
  return public_key.Validate(rng(), 2);
}

bool Validate(const PlainText &data,
              const Signature &signature,
              const PublicKey &public_key) {
  return (kSuccess == CheckSignature(data, signature, public_key));
}

bool MatchingPublicKeys(const PublicKey &public_key1,
                        const PublicKey &public_key2) {
  std::string encoded_key1, encoded_key2;
  EncodePublicKey(public_key1, &encoded_key1);
  EncodePublicKey(public_key2, &encoded_key2);
  if (!encoded_key1.empty() && !encoded_key2.empty())
    return encoded_key1 == encoded_key2;
  return false;
}

}  // namespace rsa

}  // namespace maidsafe
