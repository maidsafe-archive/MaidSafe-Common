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
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  if (number_of_shares < 3) {
    LOG(kError) << "The number of shares (" << number_of_shares << ") must be at least 3.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  if (threshold < 2) {
    LOG(kError) << "The threshold (" << threshold << ") must be at least 2.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
}

}  // unnamed namespace

CryptoPP::RandomNumberGenerator& random_number_generator() {
  if (!g_random_number_generator.get())
    g_random_number_generator.reset(new CryptoPP::AutoSeededX917RNG<CryptoPP::AES>);
  return *g_random_number_generator;
}

CipherText SymmEncrypt(const PlainText& input, const AES256KeyAndIV& key_and_iv) {
  if (!input.IsInitialised() || !key_and_iv.IsInitialised()) {
    LOG(kError) << "SymmEncrypt one member of class uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  std::string result;
  try {
    CryptoPP::GCM<CryptoPP::AES, CryptoPP::GCM_64K_Tables>::Encryption encryptor;
    encryptor.SetKeyWithIV(key_and_iv.data(), AES256_KeySize, key_and_iv.data() + AES256_KeySize,
                           AES256_IVSize);
    CryptoPP::ArraySource(
        input.data(), input.size(), true,
        new CryptoPP::AuthenticatedEncryptionFilter(encryptor, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Failed symmetric encryption: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::symmetric_encryption_error));
  }
  return CipherText(NonEmptyString(result));
}

PlainText SymmDecrypt(const CipherText& input, const AES256KeyAndIV& key_and_iv) {
  if (!input->IsInitialised() || !key_and_iv.IsInitialised()) {
    LOG(kError) << "SymmEncrypt one of class uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  std::string result;
  try {
    CryptoPP::GCM<CryptoPP::AES, CryptoPP::GCM_64K_Tables>::Decryption decryptor;
    decryptor.SetKeyWithIV(key_and_iv.data(), AES256_KeySize, key_and_iv.data() + AES256_KeySize,
                           AES256_IVSize);
    CryptoPP::ArraySource(
        input->data(), input->size(), true,
        new CryptoPP::AuthenticatedDecryptionFilter(decryptor, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception&) {
    LOG(kError) << "Failed symmetric decryption: "
                << boost::current_exception_diagnostic_information(true);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::symmetric_decryption_error));
  }
  return PlainText(result);
}

CompressedText Compress(const UncompressedText& input, uint16_t compression_level) {
  if (compression_level > kMaxCompressionLevel) {
    LOG(kError) << "Requested compression level of " << compression_level << " is above the max of "
                << kMaxCompressionLevel;
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  if (!input.IsInitialised()) {
    LOG(kError) << "Compress input uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }

  std::string result;
  try {
    CryptoPP::ArraySource(input.data(), input.size(), true,
                           new CryptoPP::Gzip(new CryptoPP::StringSink(result), compression_level));
  } catch (const CryptoPP::Exception&) {
    LOG(kError) << "Failed compressing: " << boost::current_exception_diagnostic_information(true);
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
    CryptoPP::ArraySource(input->data(), input->size(), true,
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

DataParts SecretShareData(int32_t threshold, int32_t number_of_shares,
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

  DataParts result;
  for (int i = 0; i < number_of_shares; ++i) {
    out_vec[i].resize(array_sink[i]->TotalPutLength());
    result.emplace_back(std::move(out_vec[i]));
  }
  return result;
}

PlainText SecretRecoverData(const DataParts& parts) {
  size_t num_to_check = parts.size();
  // Safe to subtract 4 since each piece is prefixed with a byte piece number
  auto data_size(num_to_check * parts.front().size());
  std::vector<byte> data(data_size);

  auto array_sink = new CryptoPP::ArraySink(data.data(), data_size);
  CryptoPP::SecretRecovery recovery(static_cast<int>(num_to_check), array_sink);
  CryptoPP::vector_member_ptrs<CryptoPP::ArraySource> array_sources(num_to_check);
  CryptoPP::vector_member_ptrs<CryptoPP::StringSource> string_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (size_t i = 0; i < num_to_check; ++i) {
    array_sources[i].reset(
        new CryptoPP::ArraySource(parts[i].data(), parts[i].size(), false));
    array_sources[i]->Pump(4);
    array_sources[i]->Get(channel, 4);
    array_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (size_t i = 0; i < num_to_check; ++i)
    array_sources[i]->PumpAll();

  data.resize(array_sink->TotalPutLength());
  return PlainText(std::move(data));
}

DataParts InfoDisperse(int32_t threshold, int32_t number_of_shares, const PlainText& data) {
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

  DataParts result;
  for (int i = 0; i < number_of_shares; ++i) {
    out_vec[i].resize(array_sink[i]->TotalPutLength());
    result.emplace_back(std::move(out_vec[i]));
  }
  return result;
}

PlainText InfoRetrieve(const DataParts& parts) {
  size_t num_to_check = parts.size();
  // Safe to subtract 4 since each piece is prefixed with a byte piece number
  auto data_size(num_to_check * (parts.front().size() - 4));
  std::vector<byte> data(data_size);

  auto array_sink = new CryptoPP::ArraySink(data.data(), data_size);
  CryptoPP::InformationRecovery recovery(static_cast<int>(num_to_check), array_sink);
  CryptoPP::vector_member_ptrs<CryptoPP::ArraySource> array_sources(num_to_check);
  CryptoPP::SecByteBlock channel(4);

  for (size_t i = 0; i < num_to_check; ++i) {
    array_sources[i].reset(
        new CryptoPP::ArraySource(parts[i].data(), parts[i].size(), false));
    array_sources[i]->Pump(4);
    array_sources[i]->Get(channel, 4);
    array_sources[i]->Attach(new CryptoPP::ChannelSwitch(
        recovery, std::string(reinterpret_cast<char*>(channel.begin()), 4)));
  }

  for (size_t i = 0; i < num_to_check; ++i)
    array_sources[i]->PumpAll();

  data.resize(array_sink->TotalPutLength());
  return PlainText(std::move(data));
}

CipherText ObfuscateData(const Identity& name, const PlainText& plain_text) {
  const auto& name_str(name.string());
  AES256KeyAndIV key_and_iv(
      std::vector<byte>(name_str.begin(), name_str.begin() + AES256_KeySize + AES256_IVSize));
  return SymmEncrypt(plain_text, key_and_iv);
}

PlainText DeobfuscateData(const Identity& name, const CipherText& cipher_text) {
  const auto& name_str(name.string());
  AES256KeyAndIV key_and_iv(
      std::vector<byte>(name_str.begin(), name_str.begin() + AES256_KeySize + AES256_IVSize));
  return SymmDecrypt(cipher_text, key_and_iv);
}

}  // namespace crypto

}  // namespace maidsafe
