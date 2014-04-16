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

#include "maidsafe/common/crypto.h"

#include <memory>
#include <algorithm>
#include <vector>

#include "boost/thread/tss.hpp"

#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace crypto {

namespace {

// Keep outside the function to avoid lazy static init races on MSVC
static boost::thread_specific_ptr<CryptoPP::AutoSeededRandomPool> g_random_number_generator;

}  // unnamed namespace

const uint16_t kMaxCompressionLevel = 9;
const std::string kMaidSafeVersionLabel1 = "MaidSafe Version 1 Key Derivation";
const std::string kMaidSafeVersionLabel = kMaidSafeVersionLabel1;

CryptoPP::RandomNumberGenerator& random_number_generator() {
  if (!g_random_number_generator.get())
    g_random_number_generator.reset(new CryptoPP::AutoSeededRandomPool);
  return *g_random_number_generator;
}

std::string XOR(const std::string& first, const std::string& second) {
  size_t common_size(first.size());
  if ((common_size != second.size()) || (common_size == 0)) {
    LOG(kWarning) << "Size mismatch or zero.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unable_to_handle_request));
  }

  std::string result(common_size, 0);
  auto first_itr(std::begin(first));
  auto second_itr(std::begin(second));
  auto result_itr(std::begin(result));
  while (result_itr != std::end(result)) {
    *result_itr = *first_itr ^ *second_itr;
    ++first_itr;
    ++second_itr;
    ++result_itr;
  }

  return result;
}

CipherText SymmEncrypt(const PlainText& input, const AES256Key& key,
                       const AES256InitialisationVector& initialisation_vector) {
  if (!input.IsInitialised() || !key.IsInitialised() || !initialisation_vector.IsInitialised()) {
    LOG(kError) << "SymmEncrypt one of class uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  std::string result;
  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];
    CryptoPP::StringSource(key.string(), true, new CryptoPP::ArraySink(byte_key, AES256_KeySize));
    CryptoPP::StringSource(initialisation_vector.string(), true,
                           new CryptoPP::ArraySink(byte_iv, AES256_IVSize));

    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(byte_key, AES256_KeySize, byte_iv);
    CryptoPP::StringSource(input.string(), true, new CryptoPP::StreamTransformationFilter(
                                                     encryptor, new CryptoPP::StringSink(result)));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed symmetric encryption: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::symmetric_encryption_error));
  }
  return CipherText(NonEmptyString(result));
}

PlainText SymmDecrypt(const CipherText& input, const AES256Key& key,
                      const AES256InitialisationVector& initialisation_vector) {
  if (!input->IsInitialised() || !key.IsInitialised() || !initialisation_vector.IsInitialised()) {
    LOG(kError) << "SymmEncrypt one of class uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  std::string result;
  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];
    CryptoPP::StringSource(key.string(), true, new CryptoPP::ArraySink(byte_key, AES256_KeySize));
    CryptoPP::StringSource(initialisation_vector.string(), true,
                           new CryptoPP::ArraySink(byte_iv, AES256_IVSize));

    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(byte_key, AES256_KeySize, byte_iv);
    CryptoPP::StringSource(input->string(), true, new CryptoPP::StreamTransformationFilter(
                                                     decryptor, new CryptoPP::StringSink(result)));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed symmetric decryption: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::symmetric_decryption_error));
  }
  return PlainText(result);
}

CompressedText Compress(const UncompressedText& input, uint16_t compression_level) {
  if (compression_level > kMaxCompressionLevel) {
    LOG(kError) << "Requested compression level of " << compression_level << " is above the max of "
                << kMaxCompressionLevel;
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
  if (!input.IsInitialised()) {
    LOG(kError) << "Compress input uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }

  std::string result;
  try {
    CryptoPP::StringSource(input.string(), true,
                           new CryptoPP::Gzip(new CryptoPP::StringSink(result), compression_level));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed compressing: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::compression_error));
  }
  return CompressedText(NonEmptyString(result));
}

UncompressedText Uncompress(const CompressedText& input) {
  if (!input->IsInitialised()) {
    LOG(kError) << "Uncompress input uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  std::string result;
  try {
    CryptoPP::StringSource(input->string(), true,
                           new CryptoPP::Gunzip(new CryptoPP::StringSink(result)));
  }
  catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed uncompressing: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uncompression_error));
  }
  return UncompressedText(result);
}

std::vector<std::string> SecretShareData(int32_t threshold, int32_t number_of_shares,
                                         const std::string& data) {
  auto channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::StringSource source(
      data, false, new CryptoPP::SecretSharing(random_number_generator(), threshold,
      number_of_shares, channel_switch));
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

std::string SecretRecoverData(int32_t threshold,
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
    string_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
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
