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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" ANDp
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

#include "maidsafe/common/crypto.h"
#include <memory>
#include <algorithm>
#include <vector>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "cryptopp/gzip.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/pssr.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/cryptlib.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace crypto {

namespace {

CryptoPP::RandomNumberGenerator& rng() {
  static CryptoPP::AutoSeededRandomPool random_number_generator;
  return random_number_generator;
}

}  // unnamed namespace


const uint16_t kMaxCompressionLevel = 9;
const std::string kMaidSafeVersionLabel1 = "MaidSafe Version 1 Key Derivation";
const std::string kMaidSafeVersionLabel = kMaidSafeVersionLabel1;

std::string XOR(const std::string& first, const std::string& second) {
  size_t common_size(first.size());
  if ((common_size != second.size()) || (common_size == 0)) {
    LOG(kWarning) << "Size mismatch or zero.";
    return "";
  }

  std::string result(common_size, 0);
  for (size_t i(0); i != common_size; ++i)
    result[i] = first[i] ^ second[i];

  return result;
}

SecurePassword CreateSecurePassword(const UserPassword& password,
                                    const Salt& salt,
                                    const uint32_t& pin,
                                    const std::string& label) {
  if (!password.IsInitialised() || !salt.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  uint16_t iter = (pin % 10000) + 10000;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
  CryptoPP::SecByteBlock derived(AES256_KeySize + AES256_IVSize);
  byte purpose = 0;  // unused in this pbkdf implementation
  CryptoPP::SecByteBlock context(salt.string().size() + label.size());
  std::copy_n(salt.string().data(), salt.string().size(), &context[0]);
  std::copy_n(label.data(),  label.size(), &context[salt.string().size()]);
  pbkdf.DeriveKey(derived, derived.size(), purpose,
                  reinterpret_cast<const byte*>(password.string().data()),
                  password.string().size(), context.data(), context.size(), iter);
  std::string derived_password;
  CryptoPP::StringSink string_sink(derived_password);
  string_sink.Put(derived, derived.size());
  return SecurePassword(derived_password);
}

CipherText SymmEncrypt(const PlainText& input,
                       const AES256Key& key,
                       const AES256InitialisationVector& initialisation_vector) {
  if (!input.IsInitialised() || !key.IsInitialised() || !initialisation_vector.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  std::string result;
  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];
    CryptoPP::StringSource(key.string(), true,
        new CryptoPP::ArraySink(byte_key, AES256_KeySize));
    CryptoPP::StringSource(initialisation_vector.string(), true,
        new CryptoPP::ArraySink(byte_iv, AES256_IVSize));

    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(byte_key, AES256_KeySize, byte_iv);
    CryptoPP::StringSource(input.string(), true,
        new CryptoPP::StreamTransformationFilter(encryptor, new CryptoPP::StringSink(result)));
  } catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed symmetric encryption: " << e.what();
    ThrowError(CommonErrors::symmetric_encryption_error);
  }
  return CipherText(result);
}

PlainText SymmDecrypt(const CipherText& input,
                      const AES256Key& key,
                      const AES256InitialisationVector& initialisation_vector) {
  if (!input.IsInitialised() || !key.IsInitialised() || !initialisation_vector.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  std::string result;
  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];
    CryptoPP::StringSource(key.string(), true,
        new CryptoPP::ArraySink(byte_key, AES256_KeySize));
    CryptoPP::StringSource(initialisation_vector.string(), true,
        new CryptoPP::ArraySink(byte_iv, AES256_IVSize));

    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(byte_key, AES256_KeySize, byte_iv);
    CryptoPP::StringSource(input.string(), true,
        new CryptoPP::StreamTransformationFilter(decryptor, new CryptoPP::StringSink(result)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed symmetric decryption: " << e.what();
    ThrowError(CommonErrors::symmetric_decryption_error);
  }
  return PlainText(result);
}

CompressedText Compress(const UncompressedText& input, const uint16_t& compression_level) {
  if (compression_level > kMaxCompressionLevel) {
    LOG(kError) << "Requested compression level of " << compression_level << " is above the max of "
                << kMaxCompressionLevel;
    ThrowError(CommonErrors::invalid_parameter);
  }
  if (!input.IsInitialised())
    ThrowError(CommonErrors::uninitialised);

  std::string result;
  try {
    CryptoPP::StringSource(input.string(), true, new CryptoPP::Gzip(
        new CryptoPP::StringSink(result), compression_level));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed compressing: " << e.what();
    ThrowError(CommonErrors::compression_error);
  }
  return CompressedText(result);
}

UncompressedText Uncompress(const CompressedText& input) {
  if (!input.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  std::string result;
  try {
    CryptoPP::StringSource(input.string(), true, new CryptoPP::Gunzip(
        new CryptoPP::StringSink(result)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Failed uncompressing: " << e.what();
    ThrowError(CommonErrors::uncompression_error);
  }
  return UncompressedText(result);
}

std::vector<std::string> SecretShareData(const int32_t& threshold,
                                         const int32_t& number_of_shares,
                                         const std::string& data) {
  CryptoPP::ChannelSwitch* channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::StringSource source(data, false,
      new CryptoPP::SecretSharing(rng(), threshold, number_of_shares, channel_switch));
  CryptoPP::vector_member_ptrs<CryptoPP::StringSink> string_sink(number_of_shares);
  std::vector<std::string> out_strings(number_of_shares);
  std::string channel;

  for (int i = 0; i < number_of_shares; ++i) {
    string_sink[i].reset(new CryptoPP::StringSink(out_strings[i]));
    channel = CryptoPP::WordToString<CryptoPP::word32>(i);
    string_sink[i]->Put(const_cast<byte*>(reinterpret_cast<const byte*>(channel.data())), 4);
    // see http://www.cryptopp.com/wiki/ChannelSwitch
    channel_switch->AddRoute(channel, *string_sink[i], CryptoPP::DEFAULT_CHANNEL);
  }
  source.PumpAll();
  return out_strings;
}

std::string SecretRecoverData(const int32_t& threshold,
                              const std::vector<std::string>& in_strings) {
  int32_t size(static_cast<int32_t>(in_strings.size()));
  int32_t num_to_check = std::min(size, threshold);
  std::string data;

  CryptoPP::SecretRecovery recovery(num_to_check, new CryptoPP::StringSink(data));
  CryptoPP::vector_member_ptrs<CryptoPP::StringSource> string_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (auto i = 0; i < num_to_check; ++i) {
    string_sources[i].reset(new CryptoPP::StringSource(in_strings[i], false));
    string_sources[i]->Pump(4);
    string_sources[i]->Get(channel, 4);
    string_sources[i]->Attach(new CryptoPP::ChannelSwitch(recovery,
                                  std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }
  while (string_sources[0]->Pump(256)) {
    for (auto i = 1; i < num_to_check; ++i)
      string_sources[i]->Pump(256);
  }

  for (auto i = 0; i < num_to_check; ++i)
    string_sources[i]->PumpAll();

  return data;
}

}  // namespace crypto

}  // namespace maidsafe
