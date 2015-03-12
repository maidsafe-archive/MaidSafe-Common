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

#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pssr.h"
#include "cryptopp/cryptlib.h"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace rsa {

namespace {

void EncodeKey(const CryptoPP::BufferedTransformation& bt, std::string& key) {
  CryptoPP::StringSink name(key);
  bt.CopyTo(name);
  name.MessageEnd();
}

void DecodeKey(const std::vector<byte>& key, CryptoPP::BufferedTransformation& bt) {
  CryptoPP::ArraySource source(key.data(), key.size(), true /*pumpAll*/);
  source.TransferTo(bt);
  bt.MessageEnd();
}

}  // Unnamed namespace

detail::Spinlock& g_rsa_mutex() {
  static detail::Spinlock mutex;
  return mutex;
}

Keys GenerateKeyPair() {
  Keys keypair;
  CryptoPP::InvertibleRSAFunction parameters;
  try {
    parameters.GenerateRandomWithKeySize(crypto::random_number_generator(), Keys::kKeyBitSize);
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed generating key pair: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::keys_generation_error));
  }
  PrivateKey private_key(parameters);
  PublicKey public_key(parameters);
  keypair.private_key = private_key;
  keypair.public_key = public_key;
  if (!(keypair.private_key.Validate(crypto::random_number_generator(), 2) &&
        keypair.public_key.Validate(crypto::random_number_generator(), 2)))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::keys_generation_error));
  return keypair;
}

CipherText Encrypt(const PlainText& data, const PublicKey& public_key) {
  if (!public_key.Validate(crypto::random_number_generator(), 0))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));

  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(public_key);
  try {
    crypto::AES256KeyAndIV local_key_and_iv(
        RandomBytes(crypto::AES256_KeySize + crypto::AES256_IVSize));
    crypto::CipherText symm_encrypted_data(crypto::SymmEncrypt(data, local_key_and_iv));

    std::string encryption_key_encrypted;
    std::lock_guard<detail::Spinlock> lock(g_rsa_mutex());
    CryptoPP::ArraySource(
        local_key_and_iv.data(), local_key_and_iv.size(), true,
        new CryptoPP::PK_EncryptorFilter(crypto::random_number_generator(), encryptor,
                                         new CryptoPP::StringSink(encryption_key_encrypted)));
    return CipherText(Serialise(symm_encrypted_data, encryption_key_encrypted));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encrypting: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::encryption_error));
  }
}

PlainText Decrypt(const CipherText& data, const PrivateKey& private_key) {
  if (!private_key.Validate(crypto::random_number_generator(), 0))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_private_key));

  try {
    crypto::CipherText symm_encrypted_data;
    std::string encryption_key_encrypted;
    try {
      Parse(data.string(), symm_encrypted_data, encryption_key_encrypted);
    } catch (const std::exception& e) {
      LOG(kError) << "Failed to parse encrypted symmetric key and data: " << e.what();
      BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::decryption_error));
    }

    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key);
    std::string local_key_and_iv;
    CryptoPP::StringSource(
        encryption_key_encrypted, true,
        new CryptoPP::PK_DecryptorFilter(crypto::random_number_generator(), decryptor,
                                         new CryptoPP::StringSink(local_key_and_iv)));
    return crypto::SymmDecrypt(symm_encrypted_data, crypto::AES256KeyAndIV(local_key_and_iv));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed decrypting: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::decryption_error));
  }
}

Signature Sign(const PlainText& data, const PrivateKey& private_key) {
  if (!data.IsInitialised()) {
    LOG(kError) << "Sign data uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  if (!private_key.Validate(crypto::random_number_generator(), 0)) {
    LOG(kError) << "Sign invalid private_key";
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_private_key));
  }

  std::string signature;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::ArraySource(data.data(), data.size(), true,
                          new CryptoPP::SignerFilter(crypto::random_number_generator(), signer,
                                                     new CryptoPP::StringSink(signature)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::signing_error));
  }
  return Signature(signature);
}

Signature SignFile(const boost::filesystem::path& filename, const PrivateKey& private_key) {
  if (!private_key.Validate(crypto::random_number_generator(), 0))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::signing_error));

  std::string signature;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Signer signer(private_key);
  try {
    CryptoPP::FileSource(filename.c_str(), true,
                         new CryptoPP::SignerFilter(crypto::random_number_generator(), signer,
                                                    new CryptoPP::StringSink(signature)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signing: " << e.what();
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_file));
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::signing_error));
  }
  return Signature(signature);
}

bool CheckSignature(const PlainText& data, const Signature& signature,
                    const PublicKey& public_key) {
  if (!data.IsInitialised() || !signature.IsInitialised()) {
    LOG(kError) << "CheckSignature data or signature uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  if (!public_key.Validate(crypto::random_number_generator(), 0)) {
    LOG(kError) << "CheckSignature invalid public_key";
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    return verifier.VerifyMessage(data.data(), data.size(), signature.data(), signature.size());
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::signing_error));
  }
}

bool CheckFileSignature(const boost::filesystem::path& filename, const Signature& signature,
                        const PublicKey& public_key) {
  if (!signature.IsInitialised()) {
    LOG(kError) << "CheckFileSignature signature uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  if (!public_key.Validate(crypto::random_number_generator(), 0)) {
    LOG(kError) << "CheckFileSignature invalid public_key";
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));
  }

  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(public_key);
  try {
    auto verifier_filter = new CryptoPP::VerifierFilter(verifier);
    verifier_filter->Put(signature.data(), verifier.SignatureLength());
    CryptoPP::FileSource file_source(filename.c_str(), true, verifier_filter);
    return verifier_filter->GetLastResult();
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed asymmetric signature checking: " << e.what();
    if (e.GetErrorType() == CryptoPP::Exception::IO_ERROR)
      BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_file));
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_signature));
  }
}

EncodedPrivateKey EncodeKey(const PrivateKey& private_key) {
  if (!private_key.Validate(crypto::random_number_generator(), 0))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_private_key));

  std::string encoded_key;
  try {
    CryptoPP::ByteQueue queue;
    private_key.DEREncodePrivateKey(queue);
    EncodeKey(queue, encoded_key);
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding private key: " << e.what();
    encoded_key.clear();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_private_key));
  }
  return EncodedPrivateKey(encoded_key);
}

EncodedPublicKey EncodeKey(const PublicKey& public_key) {
  if (!public_key.Validate(crypto::random_number_generator(), 0))
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));

  std::string encoded_key;
  try {
    CryptoPP::ByteQueue queue;
    public_key.DEREncodePublicKey(queue);
    EncodeKey(queue, encoded_key);
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed encoding public key: " << e.what();
    encoded_key.clear();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));
  }
  return EncodedPublicKey(encoded_key);
}

PrivateKey DecodeKey(const EncodedPrivateKey& private_key) {
  if (!private_key.IsInitialised()) {
    LOG(kError) << "DecodeKey private_key uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }

  PrivateKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(private_key.string(), queue);
    key.BERDecodePrivateKey(queue, false /*paramsPresent*/,
                            static_cast<size_t>(queue.MaxRetrievable()));
  } catch (const std::exception& e) {
    LOG(kError) << "Failed decoding private key: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_private_key));
  }
  return key;
}

PublicKey DecodeKey(const EncodedPublicKey& public_key) {
  if (!public_key.IsInitialised()) {
    LOG(kError) << "DecodeKey public_key uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }

  PublicKey key;
  try {
    CryptoPP::ByteQueue queue;
    DecodeKey(public_key.string(), queue);
    key.BERDecodePublicKey(queue, false /*paramsPresent*/,
                           static_cast<size_t>(queue.MaxRetrievable()));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed decoding public key: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::invalid_public_key));
  }
  return key;
}

bool ValidateKey(const PublicKey& public_key) {
  return public_key.Validate(crypto::random_number_generator(), 2);
}

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
