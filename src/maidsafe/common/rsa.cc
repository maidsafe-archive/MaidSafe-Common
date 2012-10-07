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
    LOG(kError) << "Failed generating key pair: " << e.what();
    ThrowError(AsymmErrors::keys_generation_error);
  }
  PrivateKey private_key(parameters);
  PublicKey public_key(parameters);
  keypair.private_key = private_key;
  keypair.public_key = public_key;
  if (!(keypair.private_key.Validate(rng(), 2) && keypair.public_key.Validate(rng(), 2)))
    ThrowError(AsymmErrors::keys_generation_error);
  return keypair;
}

CipherText Encrypt(const PlainText& data, const PublicKey& public_key) {
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_public_key);

  std::string result;
  result.clear();
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(public_key);
  SafeEncrypt safe_enc;
  try {
    crypto::AES256Key symm_encryption_key(RandomString(crypto::AES256_KeySize));
    crypto::AES256InitialisationVector symm_encryption_iv(RandomString(crypto::AES256_IVSize));
    safe_enc.set_data(crypto::SymmEncrypt(data, symm_encryption_key, symm_encryption_iv).string());
    std::string encryption_key_encrypted;
    CryptoPP::StringSource(symm_encryption_key.string() + symm_encryption_iv.string(),
                           true,
                           new CryptoPP::PK_EncryptorFilter(rng(),
                                                            encryptor,
                                                            new CryptoPP::StringSink(
                                                                encryption_key_encrypted)));
    safe_enc.set_key(encryption_key_encrypted);
    if (!safe_enc.SerializeToString(&result)) {
      ThrowError(AsymmErrors::keys_serialisation_error);
    }
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric encrypting: " << e.what();
    ThrowError(AsymmErrors::encryption_error);
  }
  return CipherText(result);
}

PlainText Decrypt(const CipherText& data, const PrivateKey& private_key) {
#ifndef NDEBUG
  // Passing a default-constructed PrivateKey takes several minutes to throw in Debug using VS2012.
  // We don't want this cost when running tests in Debug, but we don't want the cost of checking
  // for an edge case in production code, hence this Debug block.
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);
#endif
  PlainText result;
  try {
    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key);
    SafeEncrypt safe_enc;
    if (ParseSafeEncrypt(data.string(), safe_enc)) {
      std::string out_data;
      CryptoPP::StringSource(safe_enc.key(),
                             true,
                             new CryptoPP::PK_DecryptorFilter(rng(),
                                                              decryptor,
                                                              new CryptoPP::StringSink(out_data)));
      if (out_data.size() < crypto::AES256_KeySize + crypto::AES256_IVSize) {
        LOG(kError) << "Asymmetric decryption failed to yield correct symmetric key and IV.";
        ThrowError(AsymmErrors::decryption_error);
      }
      result = crypto::SymmDecrypt(crypto::CipherText(safe_enc.data()),
                                   crypto::AES256Key(out_data.substr(0, crypto::AES256_KeySize)),
                                   crypto::AES256InitialisationVector(
                                       out_data.substr(crypto::AES256_KeySize,
                                                       crypto::AES256_IVSize)));
    } else {
      ThrowError(AsymmErrors::decryption_error);
    }
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric decrypting: " << e.what();
    ThrowError(AsymmErrors::decryption_error);
  }

  return result;
}

Signature Sign(const PlainText& data, const PrivateKey& private_key) {
#ifndef NDEBUG
  // Passing a default-constructed PrivateKey takes several minutes to throw in Debug using VS2012.
  // We don't want this cost when running tests in Debug, but we don't want the cost of checking
  // for an edge case in production code, hence this Debug block.
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);
#endif
  std::string signature;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::StringSource(data.string(),
                           true,
                           new CryptoPP::SignerFilter(rng(),
                                                      signer,
                                                      new CryptoPP::StringSink(signature)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    ThrowError(AsymmErrors::signing_error);
  }
  return Signature(signature);
}

Signature SignFile(const boost::filesystem::path& filename, const PrivateKey& private_key) {
#ifndef NDEBUG
  // See comment in similar Debug block of rsa::Sign function
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::signing_error);
#endif

  std::string signature;
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
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      ThrowError(AsymmErrors::invalid_file);
    ThrowError(AsymmErrors::signing_error);
  }
  return Signature(signature);
}

bool CheckSignature(const PlainText& data,
                    const Signature& signature,
                    const PublicKey& public_key) {
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_public_key);

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    return verifier.VerifyMessage(reinterpret_cast<const byte*>(data.string().c_str()),
                                  data.string().size(),
                                  reinterpret_cast<const byte*>(signature.string().c_str()),
                                  signature.string().size());
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    ThrowError(AsymmErrors::signing_error);
    return false;
  }
}

bool CheckFileSignature(const boost::filesystem::path& filename,
                        const Signature& signature,
                        const PublicKey& public_key) {
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_public_key);

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    CryptoPP::VerifierFilter* verifier_filter = new CryptoPP::VerifierFilter(verifier);
    verifier_filter->Put(reinterpret_cast<const byte*>(signature.string().c_str()),
                         verifier.SignatureLength());
    CryptoPP::FileSource file_source(filename.c_str(), true, verifier_filter);
    return verifier_filter->GetLastResult();
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      ThrowError(AsymmErrors::invalid_file);
    ThrowError(AsymmErrors::invalid_signature);
    return false;
  }
}

EncodedPrivateKey EncodeKey(const PrivateKey& key) {
  std::string private_key;
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePrivateKey(queue);
    EncodeKey(queue, private_key);
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding private key: " << e.what();
    private_key.clear();
    ThrowError(AsymmErrors::invalid_private_key);
  }
  return EncodedPrivateKey(private_key);
}

EncodedPublicKey EncodeKey(const PublicKey& key) {
  std::string public_key;
  try {
    CryptoPP::ByteQueue queue;
    key.DEREncodePublicKey(queue);
    EncodeKey(queue, public_key);
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding public key: " << e.what();
    public_key.clear();
    ThrowError(AsymmErrors::invalid_public_key);
  }
  return EncodedPublicKey(public_key);
}

PrivateKey DecodeKey(const EncodedPrivateKey& private_key) {
  PrivateKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(private_key.string(), queue);
    key.BERDecodePrivateKey(queue,
                            false /*paramsPresent*/,
                            static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const std::exception& e) {
    LOG(kError) << "Failed decoding private key: " << e.what();
    ThrowError(AsymmErrors::invalid_private_key);
  }
  return key;
}

PublicKey DecodeKey(const EncodedPublicKey& public_key) {
  PublicKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(public_key.string(), queue);
    key.BERDecodePublicKey(queue,
                           false /*paramsPresent*/,
                           static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed decoding public key: " << e.what();
    ThrowError(AsymmErrors::invalid_public_key);
  }
  return key;
}

// TODO(Fraser#5#): 2012-10-02 - Find more efficient way of achieving this
bool MatchingKeys(const PublicKey& public_key1, const PublicKey& public_key2) {
  return EncodeKey(public_key1) == EncodeKey(public_key2);
}

// TODO(Fraser#5#): 2012-10-02 - Find more efficient way of achieving this
bool MatchingKeys(const PrivateKey& private_key1, const PrivateKey& private_key2) {
  return EncodeKey(private_key1) == EncodeKey(private_key2);
}


}  // namespace rsa

}  // namespace maidsafe
