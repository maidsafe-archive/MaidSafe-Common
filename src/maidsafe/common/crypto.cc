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
static boost::thread_specific_ptr<CryptoPP::AutoSeededX917RNG<CryptoPP::AES>>
    g_random_number_generator;

void ValidateDispersalArgs(int32_t threshold, int32_t number_of_shares) {
  if (threshold > number_of_shares) {
    LOG(kError) << "The threshold (" << threshold
                << ") must be less than or equal to the number of shares (" << number_of_shares
                << ").";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
  if (number_of_shares < 3) {
    LOG(kError) << "The number of shares (" << number_of_shares << ") must be at least 3.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
  if (threshold < 2) {
    LOG(kError) << "The threshold (" << threshold << ") must be at least 2.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
}

}  // unnamed namespace

const uint16_t kMaxCompressionLevel = 9;
const std::string kMaidSafeVersionLabel1 = "MaidSafe Version 1 Key Derivation";
const std::string kMaidSafeVersionLabel = kMaidSafeVersionLabel1;

CryptoPP::RandomNumberGenerator& random_number_generator() {
  if (!g_random_number_generator.get())
    g_random_number_generator.reset(new CryptoPP::AutoSeededX917RNG<CryptoPP::AES>);
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
    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(
        reinterpret_cast<const byte*>(key.string().data()), key.string().size(),
        reinterpret_cast<const byte*>(initialisation_vector.string().data()));
    CryptoPP::StringSource(input.string(), true, new CryptoPP::StreamTransformationFilter(
                                                     encryptor, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
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
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(
        reinterpret_cast<const byte*>(key.string().data()), key.string().size(),
        reinterpret_cast<const byte*>(initialisation_vector.string().data()));
    CryptoPP::StringSource(input->string(), true, new CryptoPP::StreamTransformationFilter(
                                                      decryptor, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
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
  } catch (const CryptoPP::Exception& e) {
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
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed uncompressing: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uncompression_error));
  }
  return UncompressedText(result);
}

std::vector<std::string> SecretShareData(int32_t threshold, int32_t number_of_shares,
                                         const std::string& data) {
  ValidateDispersalArgs(threshold, number_of_shares);
  auto channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::StringSource source(data, false,
                                new CryptoPP::SecretSharing(random_number_generator(), threshold,
                                                            number_of_shares, channel_switch));
  CryptoPP::vector_member_ptrs<CryptoPP::StringSink> string_sink(number_of_shares);
  std::vector<std::string> out_strings(number_of_shares);
  std::string channel;

  for (int i = 0; i < number_of_shares; ++i) {
    string_sink[i].reset(new CryptoPP::StringSink(out_strings[i]));
    channel = CryptoPP::WordToString<CryptoPP::word32>(i);
    string_sink[i]->Put(reinterpret_cast<const byte*>(channel.data()), 4);
    // see http://www.cryptopp.com/wiki/ChannelSwitch
    channel_switch->AddRoute(channel, *string_sink[i], CryptoPP::DEFAULT_CHANNEL);
  }
  source.PumpAll();
  return out_strings;
}

std::vector<std::vector<byte>> SecretShareData(int32_t threshold, int32_t number_of_shares,
                                               const std::vector<byte>& data) {
  ValidateDispersalArgs(threshold, number_of_shares);
  auto channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::ArraySource source(data.data(), data.size(), false,
                               new CryptoPP::SecretSharing(random_number_generator(), threshold,
                                                           number_of_shares, channel_switch));

  CryptoPP::vector_member_ptrs<CryptoPP::ArraySink> array_sink(number_of_shares);
  std::vector<std::vector<byte>> out_vec(number_of_shares);
  std::string channel;

  for (int i = 0; i < number_of_shares; ++i) {
    out_vec[i].resize(data.size() + 8);  // for padding
    array_sink[i].reset(new CryptoPP::ArraySink(out_vec[i].data(), out_vec[i].size()));
    channel = CryptoPP::WordToString<CryptoPP::word32>(i);
    array_sink[i]->Put(reinterpret_cast<const byte*>(channel.data()), 4);
    // see http://www.cryptopp.com/wiki/ChannelSwitch
    channel_switch->AddRoute(channel, *array_sink[i], CryptoPP::DEFAULT_CHANNEL);
  }
  source.PumpAll();
  for (int i = 0; i < number_of_shares; ++i)
    out_vec[i].resize(array_sink[i]->TotalPutLength());
  return out_vec;
}

std::string SecretRecoverData(const std::vector<std::string>& in_strings) {
  size_t num_to_check = in_strings.size();
  std::string data;

  CryptoPP::SecretRecovery recovery(static_cast<int>(num_to_check), new CryptoPP::StringSink(data));
  CryptoPP::vector_member_ptrs<CryptoPP::StringSource> string_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (size_t i = 0; i < num_to_check; ++i) {
    string_sources[i].reset(new CryptoPP::StringSource(in_strings[i], false));
    string_sources[i]->Pump(4);
    string_sources[i]->Get(channel, 4);
    string_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (size_t i = 0; i < num_to_check; ++i)
    string_sources[i]->PumpAll();

  return data;
}

std::vector<byte> SecretRecoverData(const std::vector<std::vector<byte>>& in_arrays) {
  size_t num_to_check = in_arrays.size();
  // Safe to subtract 4 since each piece is prefixed with a byte piece number
  auto data_size(num_to_check * in_arrays.front().size());
  std::vector<byte> data(data_size);

  auto array_sink = new CryptoPP::ArraySink(data.data(), data_size);
  CryptoPP::SecretRecovery recovery(static_cast<int>(num_to_check), array_sink);
  CryptoPP::vector_member_ptrs<CryptoPP::ArraySource> array_sources(num_to_check);
  CryptoPP::vector_member_ptrs<CryptoPP::StringSource> string_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (size_t i = 0; i < num_to_check; ++i) {
    array_sources[i].reset(
        new CryptoPP::ArraySource(in_arrays[i].data(), in_arrays[i].size(), false));
    array_sources[i]->Pump(4);
    array_sources[i]->Get(channel, 4);
    array_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (size_t i = 0; i < num_to_check; ++i)
    array_sources[i]->PumpAll();

  data.resize(array_sink->TotalPutLength());
  std::string ffff(data.begin(), data.end());
  (void)ffff;

  return data;
}

//  Rabin's information dispersal algorithm, space efficent
std::vector<std::string> InfoDisperse(int32_t threshold, int32_t number_of_shares,
                                      const std::string& data) {
  ValidateDispersalArgs(threshold, number_of_shares);
  auto channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::StringSource source(
      data, false, new CryptoPP::InformationDispersal(threshold, number_of_shares, channel_switch));

  CryptoPP::vector_member_ptrs<CryptoPP::StringSink> string_sink(number_of_shares);
  std::vector<std::string> out_strings(number_of_shares);
  std::string channel;

  for (int i = 0; i < number_of_shares; ++i) {
    string_sink[i].reset(new CryptoPP::StringSink(out_strings[i]));
    channel = CryptoPP::WordToString<CryptoPP::word32>(i);
    string_sink[i]->Put(reinterpret_cast<const byte*>(channel.data()), 4);
    // see http://www.cryptopp.com/wiki/ChannelSwitch
    channel_switch->AddRoute(channel, *string_sink[i], CryptoPP::DEFAULT_CHANNEL);
  }
  source.PumpAll();
  return out_strings;
}

std::vector<std::vector<byte>> InfoDisperse(int32_t threshold, int32_t number_of_shares,
                                            const std::vector<byte>& data) {
  ValidateDispersalArgs(threshold, number_of_shares);
  auto channel_switch = new CryptoPP::ChannelSwitch;
  CryptoPP::ArraySource source(
      data.data(), data.size(), false,
      new CryptoPP::InformationDispersal(threshold, number_of_shares, channel_switch));

  CryptoPP::vector_member_ptrs<CryptoPP::ArraySink> array_sink(number_of_shares);
  std::vector<std::vector<byte>> out_vec(number_of_shares);
  std::string channel;

  for (int i = 0; i < number_of_shares; ++i) {
    out_vec[i].resize(data.size());
    array_sink[i].reset(new CryptoPP::ArraySink(out_vec[i].data(), out_vec[i].size()));
    channel = CryptoPP::WordToString<CryptoPP::word32>(i);
    array_sink[i]->Put(reinterpret_cast<const byte*>(channel.data()), 4);
    // see http://www.cryptopp.com/wiki/ChannelSwitch
    channel_switch->AddRoute(channel, *array_sink[i], CryptoPP::DEFAULT_CHANNEL);
  }
  source.PumpAll();
  for (int i = 0; i < number_of_shares; ++i)
    out_vec[i].resize(array_sink[i]->TotalPutLength());
  return out_vec;
}

std::string InfoRetrieve(const std::vector<std::string>& in_strings) {
  std::string data;
  size_t num_to_check = in_strings.size();
  CryptoPP::InformationRecovery recovery(static_cast<int>(num_to_check),
                                         new CryptoPP::StringSink(data));
  CryptoPP::vector_member_ptrs<CryptoPP::StringSource> string_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (auto i = 0; i < num_to_check; ++i) {
    string_sources[i].reset(new CryptoPP::StringSource(in_strings[i], false));
    string_sources[i]->Pump(4);
    string_sources[i]->Get(channel, 4);
    string_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (auto i = 0; i < num_to_check; ++i)
    string_sources[i]->PumpAll();

  return data;
}

std::vector<byte> InfoRetrieve(const std::vector<std::vector<byte>>& in_arrays) {
  size_t num_to_check = in_arrays.size();
  // Safe to subtract 4 since each piece is prefixed with a byte piece number
  auto data_size(num_to_check * (in_arrays.front().size() - 4));
  std::vector<byte> data(data_size);

  auto array_sink = new CryptoPP::ArraySink(data.data(), data_size);
  CryptoPP::InformationRecovery recovery(static_cast<int>(num_to_check), array_sink);
  CryptoPP::vector_member_ptrs<CryptoPP::ArraySource> array_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (auto i = 0; i < num_to_check; ++i) {
    array_sources[i].reset(
        new CryptoPP::ArraySource(in_arrays[i].data(), in_arrays[i].size(), false));
    array_sources[i]->Pump(4);
    array_sources[i]->Get(channel, 4);
    array_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (auto i = 0; i < num_to_check; ++i)
    array_sources[i]->PumpAll();

  data.resize(array_sink->TotalPutLength());
  return data;
}

CipherText ObfuscateData(const Identity& name, const PlainText& plain_text) {
  AES256Key key(name.string().substr(0, AES256_KeySize));
  AES256InitialisationVector iv(name.string().substr(AES256_KeySize, AES256_IVSize));
  return SymmEncrypt(plain_text, key, iv);
}

PlainText DeobfuscateData(const Identity& name, const CipherText& cipher_text) {
  AES256Key key(name.string().substr(0, AES256_KeySize));
  AES256InitialisationVector iv(name.string().substr(AES256_KeySize, AES256_IVSize));
  return SymmDecrypt(cipher_text, key, iv);
}

}  // namespace crypto

}  // namespace maidsafe
