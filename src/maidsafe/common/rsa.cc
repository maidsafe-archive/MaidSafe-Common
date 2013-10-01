/*  Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/rsa.h"

#include <memory>

#ifdef __MSVC__
#pragma warning(push, 1)
#pragma warning(disable : 4702)
#endif
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pssr.h"
#include "cryptopp/cryptlib.h"
#include "boost/assert.hpp"
#ifdef __MSVC__
#pragma warning(pop)
#endif

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/safe_encrypt.pb.h"

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

}  // Unnamed namespace

Keys GenerateKeyPair() {
  Keys keypair;
  CryptoPP::InvertibleRSAFunction parameters;
  try {
    parameters.GenerateRandomWithKeySize(rng(), Keys::kKeyBitSize);
  }
  catch (const CryptoPP::Exception& e) {
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
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(public_key);
  protobuf::SafeEncrypt safe_encrypt;
  try {
    crypto::AES256Key symm_encryption_key(RandomString(crypto::AES256_KeySize));
    crypto::AES256InitialisationVector symm_encryption_iv(RandomString(crypto::AES256_IVSize));
    safe_encrypt.set_data(
        crypto::SymmEncrypt(data, symm_encryption_key, symm_encryption_iv).string());
    std::string encryption_key_encrypted;
    std::string const local_key_and_iv = symm_encryption_key.string() + symm_encryption_iv.string();
    CryptoPP::StringSource(
        local_key_and_iv, true,
        new CryptoPP::PK_EncryptorFilter(rng(), encryptor,
                                         new CryptoPP::StringSink(encryption_key_encrypted)));
    safe_encrypt.set_key(encryption_key_encrypted);
    if (!safe_encrypt.SerializeToString(&result))
      ThrowError(AsymmErrors::keys_serialisation_error);
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric encrypting: " << e.what();
    ThrowError(AsymmErrors::encryption_error);
  }
  return CipherText(result);
}

PlainText Decrypt(const CipherText& data, const PrivateKey& private_key) {
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);

  PlainText result;
  try {
    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key);
    protobuf::SafeEncrypt safe_encrypt;
    if (safe_encrypt.ParseFromString(data.string())) {
      std::string out_data;
      CryptoPP::StringSource(
          safe_encrypt.key(), true,
          new CryptoPP::PK_DecryptorFilter(rng(), decryptor, new CryptoPP::StringSink(out_data)));
      if (out_data.size() < crypto::AES256_KeySize + crypto::AES256_IVSize) {
        LOG(kError) << "Asymmetric decryption failed to yield correct symmetric key and IV.";
        ThrowError(AsymmErrors::decryption_error);
      }
      result = crypto::SymmDecrypt(crypto::CipherText(safe_encrypt.data()),
                                   crypto::AES256Key(out_data.substr(0, crypto::AES256_KeySize)),
                                   crypto::AES256InitialisationVector(out_data.substr(
                                       crypto::AES256_KeySize, crypto::AES256_IVSize)));
    } else {
      ThrowError(AsymmErrors::decryption_error);
    }
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric decrypting: " << e.what();
    ThrowError(AsymmErrors::decryption_error);
  }

  return result;
}

Signature Sign(const PlainText& data, const PrivateKey& private_key) {
  if (!data.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);

  std::string signature;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::StringSource(
        data.string(), true,
        new CryptoPP::SignerFilter(rng(), signer, new CryptoPP::StringSink(signature)));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    ThrowError(AsymmErrors::signing_error);
  }
  return Signature(signature);
}

Signature SignFile(const boost::filesystem::path& filename, const PrivateKey& private_key) {
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::signing_error);

  std::string signature;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::FileSource(
        filename.c_str(), true,
        new CryptoPP::SignerFilter(rng(), signer, new CryptoPP::StringSink(signature)));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      ThrowError(AsymmErrors::invalid_file);
    ThrowError(AsymmErrors::signing_error);
  }
  return Signature(signature);
}

bool CheckSignature(const PlainText& data, const Signature& signature,
                    const PublicKey& public_key) {
  if (!data.IsInitialised() || !signature.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_public_key);

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    return verifier.VerifyMessage(
        reinterpret_cast<const byte*>(data.string().c_str()), data.string().size(),
        reinterpret_cast<const byte*>(signature.string().c_str()), signature.string().size());
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    ThrowError(AsymmErrors::signing_error);
    return false;
  }
}

bool CheckFileSignature(const boost::filesystem::path& filename, const Signature& signature,
                        const PublicKey& public_key) {
  if (!signature.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_public_key);

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    auto verifier_filter = new CryptoPP::VerifierFilter(verifier);
    verifier_filter->Put(reinterpret_cast<const byte*>(signature.string().c_str()),
                         verifier.SignatureLength());
    CryptoPP::FileSource file_source(filename.c_str(), true, verifier_filter);
    return verifier_filter->GetLastResult();
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      ThrowError(AsymmErrors::invalid_file);
    ThrowError(AsymmErrors::invalid_signature);
    return false;
  }
}

EncodedPrivateKey EncodeKey(const PrivateKey& private_key) {
  if (!private_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);

  std::string encoded_key;
  try {
    CryptoPP::ByteQueue queue;
    private_key.DEREncodePrivateKey(queue);
    EncodeKey(queue, encoded_key);
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding private key: " << e.what();
    encoded_key.clear();
    ThrowError(AsymmErrors::invalid_private_key);
  }
  return EncodedPrivateKey(encoded_key);
}

EncodedPublicKey EncodeKey(const PublicKey& public_key) {
  if (!public_key.Validate(rng(), 0))
    ThrowError(AsymmErrors::invalid_private_key);

  std::string encoded_key;
  try {
    CryptoPP::ByteQueue queue;
    public_key.DEREncodePublicKey(queue);
    EncodeKey(queue, encoded_key);
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding public key: " << e.what();
    encoded_key.clear();
    ThrowError(AsymmErrors::invalid_public_key);
  }
  return EncodedPublicKey(encoded_key);
}

PrivateKey DecodeKey(const EncodedPrivateKey& private_key) {
  if (!private_key.IsInitialised())
    ThrowError(CommonErrors::uninitialised);

  PrivateKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(private_key.string(), queue);
    key.BERDecodePrivateKey(queue, false /*paramsPresent*/,
                            static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch (const std::exception& e) {
    LOG(kError) << "Failed decoding private key: " << e.what();
    ThrowError(AsymmErrors::invalid_private_key);
  }
  return key;
}

PublicKey DecodeKey(const EncodedPublicKey& public_key) {
  if (!public_key.IsInitialised())
    ThrowError(CommonErrors::uninitialised);

  PublicKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(public_key.string(), queue);
    key.BERDecodePublicKey(queue, false /*paramsPresent*/,
                           static_cast<size_t>(queue.MaxRetrievable()));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed decoding public key: " << e.what();
    ThrowError(AsymmErrors::invalid_public_key);
  }
  return key;
}

bool ValidateKey(const PublicKey& public_key) { return public_key.Validate(rng(), 2); }

bool MatchingKeys(const PrivateKey& private_key1, const PrivateKey& private_key2) {
  CryptoPP::ByteQueue queue1, queue2;
  private_key1.DEREncodePrivateKey(queue1);
  private_key2.DEREncodePrivateKey(queue2);
  return queue1 == queue2;
}

bool MatchingKeys(const PublicKey& public_key1, const PublicKey& public_key2) {
  CryptoPP::ByteQueue queue1, queue2;
  public_key1.DEREncodePublicKey(queue1);
  public_key2.DEREncodePublicKey(queue2);
  return queue1 == queue2;
}

}  // namespace rsa

}  // namespace maidsafe
