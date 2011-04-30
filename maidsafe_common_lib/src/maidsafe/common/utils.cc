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

#include "maidsafe/common/utils.h"

#include <ctype.h>

#include <cstdint>
#include <algorithm>
#include <limits>
#include <string>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4127)
#endif

#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/scoped_array.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"

#include "cryptopp/integer.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base32.h"
#include "cryptopp/base64.h"
#include "cryptopp/hex.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/log.h"

namespace maidsafe {

CryptoPP::AutoSeededX917RNG<CryptoPP::AES> g_srandom_number_generator;
boost::mt19937 g_random_number_generator(static_cast<unsigned int>(
      boost::posix_time::microsec_clock::universal_time().time_of_day().
      total_microseconds()));
boost::mutex g_srandom_number_generator_mutex;
boost::mutex g_random_number_generator_mutex;

boost::int32_t SRandomInt32() {
  boost::int32_t result(0);
  bool success = false;
  while (!success) {
    boost::mutex::scoped_lock lock(g_srandom_number_generator_mutex);
    CryptoPP::Integer rand_num(g_srandom_number_generator, 32);
    if (rand_num.IsConvertableToLong()) {
      result = static_cast<boost::int32_t>(
               rand_num.AbsoluteValue().ConvertToLong());
      success = true;
    }
  }
  return result;
}

boost::int32_t RandomInt32() {
  boost::uniform_int<> uniform_distribution(0,
      boost::integer_traits<boost::int32_t>::const_max);
  boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
      g_random_number_generator, uniform_distribution);
  return uni();
}

boost::uint32_t SRandomUint32() {
  return static_cast<boost::uint32_t>(SRandomInt32());
}

boost::uint32_t RandomUint32() {
  return static_cast<boost::uint32_t>(RandomInt32());
}

std::string SRandomString(const size_t &length) {
  std::string random_string;
  random_string.reserve(length);
  while (random_string.size() < length) {
#ifdef MAIDSAFE_APPLE
     size_t iter_length = (length - random_string.size()) < 65536U ?
                          (length - random_string.size()) : 65536U;
#else
    size_t iter_length = std::min(length - random_string.size(), size_t(65536));
#endif
    boost::scoped_array<byte> random_bytes(new byte[iter_length]);
    {
      boost::mutex::scoped_lock lock(g_srandom_number_generator_mutex);
      g_srandom_number_generator.GenerateBlock(random_bytes.get(), iter_length);
    }
    std::string random_substring;
    CryptoPP::StringSink string_sink(random_substring);
    string_sink.Put(random_bytes.get(), iter_length);
    random_string += random_substring;
  }
  return random_string;
}

std::string RandomString(const size_t &length) {
  boost::uniform_int<> uniform_distribution(0, 255);
  std::string random_string(length, 0);
  {
    boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        g_random_number_generator, uniform_distribution);
    std::generate(random_string.begin(), random_string.end(), uni);
  }
  return random_string;
}

std::string RandomAlphaNumericString(const size_t &length) {
  static const char alpha_numerics[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  boost::uniform_int<> uniform_distribution(0, 61);
  std::string random_string(length, 0);
  {
    boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        g_random_number_generator, uniform_distribution);
    for (auto it = random_string.begin(); it != random_string.end(); ++it)
      *it = alpha_numerics[uni()];
  }
  return random_string;
}

std::string IntToString(const int &value) {
  return boost::lexical_cast<std::string>(value);
}

std::string EncodeToHex(const std::string &non_hex_input) {
  std::string hex_output;
  CryptoPP::StringSource(non_hex_input, true,
      new CryptoPP::HexEncoder(new CryptoPP::StringSink(hex_output), false));
  return hex_output;
}

std::string EncodeToBase64(const std::string &non_base64_input) {
  std::string base64_output;
  CryptoPP::StringSource(non_base64_input, true, new CryptoPP::Base64Encoder(
      new CryptoPP::StringSink(base64_output), false, 255));
  return base64_output;
}

std::string EncodeToBase32(const std::string &non_base32_input) {
  std::string base32_output;
  CryptoPP::StringSource(non_base32_input, true, new CryptoPP::Base32Encoder(
      new CryptoPP::StringSink(base32_output), false));
  return base32_output;
}

std::string DecodeFromHex(const std::string &hex_input) {
  std::string non_hex_output;
  CryptoPP::StringSource(hex_input, true,
      new CryptoPP::HexDecoder(new CryptoPP::StringSink(non_hex_output)));
  return non_hex_output;
}

std::string DecodeFromBase64(const std::string &base64_input) {
  std::string non_base64_output;
  CryptoPP::StringSource(base64_input, true,
      new CryptoPP::Base64Decoder(new CryptoPP::StringSink(non_base64_output)));
  return non_base64_output;
}

std::string DecodeFromBase32(const std::string &base32_input) {
  std::string non_base32_output;
  CryptoPP::StringSource(base32_input, true,
      new CryptoPP::Base32Decoder(new CryptoPP::StringSink(non_base32_output)));
  return non_base32_output;
}

boost::posix_time::time_duration GetDurationSinceEpoch() {
  return boost::posix_time::microsec_clock::universal_time() - kMaidSafeEpoch;
}

bool ReadFile(const fs::path &file_path, std::string *content) {
  if (!content)
    return false;
  try {
    std::uintmax_t file_size(fs::file_size(file_path));
    fs::ifstream file_in(file_path, std::ios::in | std::ios::binary);
    if (!file_in.good())
      return false;
    if (file_size == 0U) {
      content->clear();
      return true;
    }
    content->resize(file_size);
    file_in.read(&((*content)[0]), file_size);
    file_in.close();
  }
  catch(...) {
    return false;
  }
  return true;
}

bool WriteFile(const fs::path &file_path, const std::string &content) {
  try {
    if (!file_path.has_filename())
      return false;
    fs::ofstream file_out(file_path, std::ios::out | std::ios::trunc |
                                     std::ios::binary);
    file_out.write(content.data(), content.size());
    file_out.close();
  }
  catch(...) {
    return false;
  }
  return true;
}


namespace test {

TestPath CreateTestPath(std::string test_prefix) {
  if (test_prefix.empty())
    test_prefix = "MaidSafe_Test";

  if (test_prefix.substr(0, 13) != "MaidSafe_Test" &&
      test_prefix.substr(0, 12) != "Sigmoid_Test") {
    LOG(WARNING) << "Test prefix should preferably be \"MaidSafe_Test<optional"
                 << " test name>\" or \"Sigmoid_Test<optional test name>\".";
  }

  test_prefix += "_%%%%-%%%%-%%%%";

  boost::system::error_code error_code;
  TestPath test_path(new fs::path(fs::unique_path(
      fs::temp_directory_path(error_code) / test_prefix)), CleanupTest);
  if (error_code) {
    LOG(WARNING) << "Can't get a temp directory: " << error_code.message();
    return TestPath(new fs::path);
  }

  if (!fs::create_directories(*test_path, error_code) || error_code) {
    LOG(WARNING) << "Failed to create test directory " << *test_path
                 << "  (error message: " << error_code.message() << ")";
    return TestPath(new fs::path);
  }

  LOG(INFO) << "Created test directory " << *test_path;
  return test_path;
}

void CleanupTest(fs::path *test_path) {
  if (test_path->empty())
    return;
  boost::system::error_code error_code;
  if (!fs::remove_all(*test_path, error_code) || error_code) {
    LOG(WARNING) << "Failed to clean up test directory " << *test_path
                 << "  (error message: " << error_code.message() << ")";
  }
}

}  // namespace test

}  // namespace maidsafe
