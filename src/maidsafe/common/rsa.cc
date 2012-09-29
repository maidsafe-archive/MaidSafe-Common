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

#include "maidsafe/common/error.h"

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

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/safe_encrypt_pb.h"


namespace maidsafe {

namespace rsa {

namespace {

CryptoPP::RandomNumberGenerator& rng() {
  static CryptoPP::AutoSeededRandomPool random_number_generator;
  return random_number_generator;
}

void EncodeKey(const CryptoPP::BufferedTransformation& bt, std::string& key) {
  CryptoPP::StringSink name(key);
  bt.CopyTo(name);
  name.MessageEnd();
}

void DecodeKey(const std::string& key, CryptoPP::BufferedTransformation& bt) {
  CryptoPP::StringSource file(key, true /*pumpAll*/);
  file.TransferTo(bt);
  bt.MessageEnd();
}


bool ParseSafeEncrypt(const std::string& serialised_safe_encrypt, SafeEncrypt& safe_encrypt) {
  return safe_encrypt.ParseFromString(serialised_safe_encrypt);
}

}  // Unnamed namespace

Keys GenerateKeyPair() {
  Keys keypair;
  CryptoPP::InvertibleRSAFunction parameters;
  try {
    parameters.GenerateRandomWithKeySize(rng(), Keys::kKeySize);
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed generating keypair: " << e.what();
    throw error_code::kKeyGenerationError;
  }
  PrivateKey private_key(parameters);
  PublicKey public_key(parameters);
  keypair.private_key = private_key;
  keypair.public_key = public_key;
  if (!(keypair.private_key.Validate(rng(), 2) && keypair.public_key.Validate(rng(), 2)))
    throw error_code::kGeneralError;
  return keypair;
}

std::string Encrypt(const PlainText& data,
            const PublicKey& public_key) {
  std::string result;
  if (data.empty()) {
    LOG(kError) << "No data";
    throw error_code::kDataEmpty;
  }
  if (!public_key.Validate(rng(), 0)) {
    LOG(kError) << "Bad public key";
    throw error_code::kInvalidPublicKey;
  }

  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(public_key);
  SafeEncrypt safe_enc;
  try {
    std::string symm_encryption_key(RandomString(crypto::AES256_KeySize));
    std::string symm_encryption_iv(RandomString(crypto::AES256_IVSize));
    safe_enc.set_data(crypto::SymmEncrypt(data, symm_encryption_key, symm_encryption_iv));
    BOOST_ASSERT(!safe_enc.data().empty());
    std::string encryption_key_encrypted;
    CryptoPP::StringSource(symm_encryption_key + symm_encryption_iv,
                            true,
                            new CryptoPP::PK_EncryptorFilter(rng(),
                                                             encryptor,
                                                             new CryptoPP::StringSink(
                                                               encryption_key_encrypted)));
    safe_enc.set_key(encryption_key_encrypted);
    if (!safe_enc.SerializeToString(&result)) {
      LOG(kError) << "Failed to serialise PB";
      result.clear();
      throw error_code::kRSASerialisationError;
    }
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encryption: " << e.what();
    throw error_code::kRSAEncryptError;
  }
   return result;
}

std::string Decrypt(const CipherText& data, const PrivateKey& private_key) {
  std::string result;
  try {
    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key);
    SafeEncrypt safe_enc;
    if (ParseSafeEncrypt(data, safe_enc)) {
      std::string out_data;
      CryptoPP::StringSource(safe_enc.key(),
                             true,
                             new CryptoPP::PK_DecryptorFilter(rng(),
                                                              decryptor,
                                                              new CryptoPP::StringSink(out_data)));
      result = crypto::SymmDecrypt(safe_enc.data(),
                                    out_data.substr(0, crypto::AES256_KeySize),
                                    out_data.substr(crypto::AES256_KeySize,
                                                    crypto::AES256_IVSize));
    } else {
      LOG(kError) << "Failed to ParseSafeEncrypt.";
      throw error_code::kRSADecryptError;
    }
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed decryption: " << e.what();
    throw error_code::kRSADecryptError;
  }

  if (result.empty()) {
    LOG(kError) << "Failed decryption";
    throw error_code::kRSADecryptError;
  }
  return result;
}

std::string Sign(const std::string& data, const PrivateKey& private_key) {
  std::string signature;
  if (data.empty())
    throw error_code::kEmptyData;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::StringSource(data,
                           true,
                           new CryptoPP::SignerFilter(rng(),
                                                      signer,
                                                      new CryptoPP::StringSink(signature)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    throw error_code::kRSASigningError;
  }
  return signature;
}

std::string SignFile(const boost::filesystem::path& filename,
                     const PrivateKey& private_key) {
  std::string signature;
  boost::system::error_code boost_error_code;
  if (boost::filesystem::file_size(filename, boost_error_code) == 0 || boost_error_code) {
    LOG(kError) << "File empty or inaccessible.";
    throw error_code::kRSAEmptyFileError;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::FileSource(filename.c_str(),
                         true,
                         new CryptoPP::SignerFilter(rng(),
                                                    signer,
                                                    new CryptoPP::StringSink(signature)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    throw error_code::kRSASigningError;
  }
  return signature;
}


bool CheckSignature(const PlainText& data, const Signature& signature, const PublicKey& public_key) {
  if (!public_key.Validate(rng(), 0)) {
    LOG(kError) << "Bad public key";
    return false;
  }
  if (data.empty()) {
    LOG(kError) << "No data";
    throw  error_code::kDataEmpty;
  }
  if (signature.empty()) {
    LOG(kError) << "No signature";
    throw  error_code::kRSASignatureEmpty;
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    if (verifier.VerifyMessage(reinterpret_cast<const byte*>(data.c_str()),
                               data.size(),
                               reinterpret_cast<const byte*>(signature.c_str()),
                               signature.size()))
       return true;
     else
       throw  error_code::kRSAInvalidSignature;
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed signature check: " << e.what();
    throw error_code::kRSAInvalidSignature;
  }
  return false;
}

bool CheckFileSignature(const boost::filesystem::path& filename,
                              const Signature& signature,
                              const PublicKey& public_key) {

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    CryptoPP::VerifierFilter* verifier_filter = new CryptoPP::VerifierFilter(verifier);
    verifier_filter->Put(reinterpret_cast<const byte*>(signature.c_str()),
                         verifier.SignatureLength());
    CryptoPP::FileSource file_source(filename.c_str(), true, verifier_filter);
    return verifier_filter->GetLastResult() ? true : false;
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed signature check: " << e.what();
    throw error_code::kRSAInvalidSignature;
  }
  return true;
}

std::string EncodePrivateKey(const PrivateKey& key)  {
  std::string private_key;
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePrivateKey(queue);
    EncodeKey(queue, private_key);
  }
  catch(const CryptoPP::Exception& e) {
  throw error_code::kInvalidPrivateKey;
}
  return private_key;
}

std::string EncodePublicKey(const PublicKey& key) {
  std::string public_key;
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePublicKey(queue);
    EncodeKey(queue, public_key);
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << e.what();
    public_key.clear();
  }
  return public_key;
}

PrivateKey DecodePrivateKey(const std::string& private_key) {
  PrivateKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(private_key, queue);
    key.BERDecodePrivateKey(queue,
                             false /*paramsPresent*/,
                             static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const std::exception& e) {
    LOG(kError) << e.what();

    throw error_code::kRSAParseError;
  }
  return key;
}

PublicKey DecodePublicKey(const std::string& public_key) {
  PublicKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(public_key, queue);
    key.BERDecodePublicKey(queue,
                            false /*paramsPresent*/,
                            static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const CryptoPP::Exception& e) {
    throw error_code::kInvalidPublicKey;
  }
  return key;
}

bool CheckRoundtrip(const PublicKey& public_key, const PrivateKey& private_key) {
  return (public_key.GetModulus() != private_key.GetModulus() ||
          public_key.GetPublicExponent() != private_key.GetPrivateExponent());
}

bool ValidateKey(const PrivateKey& private_key, unsigned int level) {
  return private_key.Validate(rng(), level);
}

bool ValidateKey(const PublicKey& public_key, unsigned int level) {
  return public_key.Validate(rng(), level);
}

bool Validate(const PlainText& data, const Signature& signature, const PublicKey& public_key) {
  return CheckSignature(data, signature, public_key);
}

bool MatchingPublicKeys(const PublicKey& public_key1, const PublicKey& public_key2) {
  return EncodePublicKey(public_key1) == EncodePublicKey(public_key2);
}

bool MatchingPrivateKeys(const PrivateKey& private_key1, const PrivateKey& private_key2) {
  return EncodePrivateKey(private_key1) == EncodePrivateKey(private_key2);
}

bool SerialiseKeys(const Keys& keys, std::string& serialised_keys) {
  KeysContainer container;
  container.set_identity(keys.identity);
  container.set_validation_token(keys.validation_token);
  std::string public_key(EncodePublicKey(keys.public_key));
  std::string private_key(EncodePrivateKey(keys.private_key));
  if (public_key.empty()) {
    LOG(kError) << "Failed to encode public key.";
    return false;
  }
  if (private_key.empty()) {
    LOG(kError) << "Failed to encode private key.";
    return false;
  }

  container.set_encoded_public_key(public_key);
  container.set_encoded_private_key(private_key);
  try {
    serialised_keys = container.SerializeAsString();
  }
  catch(...) {
    LOG(kError) << "Failed to serialise PB.";
    return false;
  }

  return true;
}

bool ParseKeys(const std::string& serialised_keys, Keys& keys) {
  KeysContainer container;
  try {
    if (!container.ParseFromString(serialised_keys)) {
        LOG(kError) << "Failed to parse into PB.";
      return false;
    }
  }
  catch(...) {
    LOG(kError) << "Failed to parse into PB.";
    return false;
  }

  keys.identity = container.identity();
  keys.validation_token = container.validation_token();

  keys.public_key = DecodePublicKey(container.encoded_public_key());
  keys.private_key = DecodePrivateKey(container.encoded_private_key());
  if (!ValidateKey(keys.public_key) || !ValidateKey(keys.private_key)) {
    LOG(kError) << "Failed to decode public or private key.";
    return false;
  }

  return true;
}

}  // namespace rsa

}  // namespace maidsafe
