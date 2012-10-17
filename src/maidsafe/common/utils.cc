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
#include <algorithm>
#include <array>
#include <future>
#include <limits>
#include <set>
#include <thread>
#include <vector>

#if defined(macintosh) || defined(__APPLE__) || \
defined(__APPLE_CC__) || (defined(linux) || \
defined(__linux) || defined(__linux__) || defined(__GNU__) \
|| defined(__GLIBC__)) && !defined(_CRAYC)
  #include  "pwd.h"  // NOLINT (dirvine)
  #include "sys/param.h"
#endif

#ifdef __MSVC__
  #include "windows.h"    // NOLINT - Viv
#endif

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4127)
#endif

#include "boost/config.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"

#include "cryptopp/base32.h"
#include "cryptopp/base64.h"
#include "cryptopp/hex.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"


namespace maidsafe {

namespace {

boost::mt19937 g_random_number_generator(static_cast<unsigned int>(
      boost::posix_time::microsec_clock::universal_time().time_of_day().
      total_microseconds()));
boost::mutex g_random_number_generator_mutex, g_srandom_number_generator_mutex,
             g_hex_encoder_mutex, g_hex_decoder_mutex,
             g_base32_encoder_mutex, g_base32_decoder_mutex,
             g_base64_encoder_mutex, g_base64_decoder_mutex;

struct BinaryUnit;
struct DecimalUnit;

template <typename Units>
struct UnitType {};

template <>
struct UnitType<BinaryUnit> {
  static const uint64_t kKilo = 1024;
  static const uint64_t kExaThreshold = 11529215046068469760U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = { {" B", " KiB", " MiB", " GiB", " TiB",
                                         " PiB", " EiB"} };
    return temp;
  }
};

template <>
struct UnitType<DecimalUnit> {
  static const uint64_t kKilo = 1000;
  static const uint64_t kExaThreshold = 9500000000000000000U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = { {" B", " kB", " MB", " GB", " TB",
                                         " PB", " EB"} };
    return temp;
  }
};

template <typename Units>
std::string BytesToSiUnits(const uint64_t &num) {
  const uint64_t kKilo(UnitType<Units>::kKilo);
  std::array<std::string, 7> qualifier = UnitType<Units>::Qualifier();

  if (num < kKilo)
    return boost::lexical_cast<std::string>(num) + qualifier[0];

  size_t count(1);
  uint64_t threshold(0), midpoint(kKilo / 2), divisor(kKilo);
  for (; count != 6; midpoint *= kKilo, divisor *= kKilo, ++count) {
    threshold = (divisor * kKilo) - midpoint;
    if (num < threshold)
      return boost::lexical_cast<std::string>((num + midpoint) / divisor) +
             qualifier[count];
  }

  threshold = UnitType<Units>::kExaThreshold;
  if (num < threshold) {
    return boost::lexical_cast<std::string>((num + midpoint) / divisor) +
           qualifier[6];
  } else {
    return boost::lexical_cast<std::string>(((num - midpoint) / divisor) + 1) +
           qualifier[6];
  }
}

}  // unnamed namespace


const boost::posix_time::ptime kMaidSafeEpoch(
    boost::posix_time::from_iso_string("20000101T000000"));

const int kInvalidVersion(-1);

boost::asio::ip::address GetLocalIp(boost::asio::ip::udp::endpoint peer_endpoint) {
  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket socket(io_service);
  try {
    socket.connect(peer_endpoint);
    if (socket.local_endpoint().address().is_unspecified() ||
        socket.local_endpoint().address().is_loopback())
      return boost::asio::ip::address();
    return socket.local_endpoint().address();
  }
  catch(const std::exception& e) {
    LOG(kError) << "Failed trying to connect to " << peer_endpoint << " - " << e.what();
    return boost::asio::ip::address();
  }
}

std::string VersionToString(int version,
                            std::string* major_version,
                            std::string* minor_version,
                            std::string* patch_version) {
  if (version < 0)
    return "";

  std::string full_version(boost::lexical_cast<std::string>(version));
  size_t padding_count(6 - full_version.size());
  full_version.insert(0, padding_count, '0');
  std::string major_ver(full_version.substr(0, 2));
  std::string minor_ver(full_version.substr(2, 2));
  std::string patch_ver(full_version.substr(4, 2));

  if (major_ver.at(0) == '0')
    major_ver.assign(major_ver.substr(1, 1));
  if (major_version)
    *major_version = major_ver;
  if (minor_version)
    *minor_version = minor_ver;
  if (patch_version)
    *patch_version = patch_ver;
  return major_ver + "." + minor_ver + "." + patch_ver;
}

int VersionToInt(const std::string& version) {
  boost::tokenizer<boost::char_separator<char>> tokens(version, boost::char_separator<char>("."));
  if (std::distance(tokens.begin(), tokens.end()) != 3)
    return kInvalidVersion;

  auto itr(tokens.begin());

  int16_t major_version(0), minor_version(0), patch_level(0);
  try {
    major_version = boost::lexical_cast<int16_t>(*(itr++));

    minor_version = boost::lexical_cast<int16_t>(*itr);
    if ((*itr++).size() != 2U) {
      LOG(kWarning) << "Invalid minor version " << version;
      return kInvalidVersion;
    }

    patch_level = boost::lexical_cast<int16_t>(*itr);
    if ((*itr++).size() != 2U) {
      LOG(kWarning) << "Invalid patch level " << version;
      return kInvalidVersion;
    }
  }
  catch(const boost::bad_lexical_cast& e) {
    LOG(kWarning) << "Invalid version " << version << ": " << e.what();
    return kInvalidVersion;
  }

  if (major_version < 0 || minor_version < 0 || patch_level < 0) {
    LOG(kWarning) << "Invalid version " << version;
    return kInvalidVersion;
  }

  return (major_version * 10000) + (minor_version * 100) + patch_level;
}

int32_t CpuSize() {
  return (sizeof(void *) * 8);  // NOLINT (Fraser)
}

std::string BytesToDecimalSiUnits(const uint64_t &num) {
  return BytesToSiUnits<DecimalUnit>(num);
}

std::string BytesToBinarySiUnits(const uint64_t &num) {
  return BytesToSiUnits<BinaryUnit>(num);
}

int32_t RandomInt32() {
  boost::uniform_int<> uniform_distribution(0,
      boost::integer_traits<int32_t>::const_max);
  boost::mutex::scoped_lock lock(g_random_number_generator_mutex);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
      g_random_number_generator, uniform_distribution);
  return uni();
}

uint32_t RandomUint32() {
  return static_cast<uint32_t>(RandomInt32());
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
  {
    boost::mutex::scoped_lock lock(g_hex_encoder_mutex);
    CryptoPP::StringSource(non_hex_input, true,
        new CryptoPP::HexEncoder(new CryptoPP::StringSink(hex_output), false));
  }
  return hex_output;
}

std::string EncodeToBase64(const std::string &non_base64_input) {
  std::string base64_output;
  {
    boost::mutex::scoped_lock lock(g_base64_encoder_mutex);
    CryptoPP::StringSource(non_base64_input, true, new CryptoPP::Base64Encoder(
        new CryptoPP::StringSink(base64_output), false, 255));
  }
  return base64_output;
}

std::string EncodeToBase32(const std::string &non_base32_input) {
  std::string base32_output;
  {
    boost::mutex::scoped_lock lock(g_base32_encoder_mutex);
    CryptoPP::StringSource(non_base32_input, true, new CryptoPP::Base32Encoder(
        new CryptoPP::StringSink(base32_output), false));
  }
  return base32_output;
}

std::string DecodeFromHex(const std::string &hex_input) {
  std::string non_hex_output;
  {
    boost::mutex::scoped_lock lock(g_hex_decoder_mutex);
    CryptoPP::StringSource(hex_input, true,
        new CryptoPP::HexDecoder(new CryptoPP::StringSink(non_hex_output)));
  }
  return non_hex_output;
}

std::string DecodeFromBase64(const std::string &base64_input) {
  std::string non_base64_output;
  {
    boost::mutex::scoped_lock lock(g_base64_decoder_mutex);
    CryptoPP::StringSource(base64_input, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(non_base64_output)));
  }
  return non_base64_output;
}

std::string DecodeFromBase32(const std::string &base32_input) {
  std::string non_base32_output;
  {
    boost::mutex::scoped_lock lock(g_base64_decoder_mutex);
    CryptoPP::StringSource(base32_input, true,
        new CryptoPP::Base32Decoder(
            new CryptoPP::StringSink(non_base32_output)));
  }
  return non_base32_output;
}

std::string HexSubstr(const std::string &non_hex) {
  std::string hex(EncodeToHex(non_hex));
  if (hex.size() > 16)
    return (hex.substr(0, 7) + ".." + hex.substr(hex.size() - 7));
  else
    return hex;
}

std::string Base32Substr(const std::string &non_base32) {
  std::string base32(EncodeToBase32(non_base32));
  if (base32.size() > 16)
    return (base32.substr(0, 7) + ".." + base32.substr(base32.size() - 7));
  else
    return base32;
}

std::string Base64Substr(const std::string &non_base64) {
  std::string base64(EncodeToBase64(non_base64));
  if (base64.size() > 16)
    return (base64.substr(0, 7) + ".." + base64.substr(base64.size() - 7));
  else
    return base64;
}

std::string DebugId(const Identity& id) {
  return id.IsInitialised() ? "Uninitialised Identity" : HexSubstr(id.string());
}

boost::posix_time::time_duration GetDurationSinceEpoch() {
  return boost::posix_time::microsec_clock::universal_time() - kMaidSafeEpoch;
}

uint32_t GetTimeStamp() {
  boost::posix_time::time_duration since_epoch(GetDurationSinceEpoch());
  return since_epoch.total_seconds();
}

bool ReadFile(const fs::path &file_path, std::string *content) {
  if (!content) {
    LOG(kError) << "Failed to read file " << file_path
                << ": NULL pointer passed";
    return false;
  }

  try {
    uintmax_t file_size(fs::file_size(file_path));
    if (file_size > std::numeric_limits<unsigned int>::max()) {
      LOG(kError) << "Failed to read file " << file_path << ": File size "
                  << file_size << " too large (over "
                  << std::numeric_limits<unsigned int>::max() << ")";
      return false;
    }

    uint16_t i(0), seed(10 + RandomUint32() % 10);

    while ((i < 5) && (file_size == 0U)) {
      Sleep(boost::posix_time::milliseconds(seed));
      file_size = fs::file_size(file_path);
      ++i;
      LOG(kWarning) << "Re-read attempt " << i << " after sleeping for " << seed
                    << " ms, file size is " << file_size;
      seed *= 2;
    }

    fs::ifstream file_in(file_path, std::ios::in | std::ios::binary);
    if (!file_in.good()) {
      LOG(kError) << "Failed to read file " << file_path << ": Bad filestream";
      return false;
    }
    if (file_size == 0U) {
      content->clear();
      return true;
    }

    content->resize(static_cast<unsigned int>(file_size));
    file_in.read(&((*content)[0]), file_size);
    file_in.close();
  }
  catch(const std::exception &e) {
    LOG(kError) << "Failed to read file " << file_path << ": " << e.what();
    return false;
  }
  return true;
}

NonEmptyString ReadFile(const fs::path &file_path) {
  uintmax_t file_size(fs::file_size(file_path));
  if (file_size > std::numeric_limits<unsigned int>::max())
    ThrowError(CommonErrors::file_too_large);

  uint16_t i(0), seed(10 + RandomUint32() % 10);
  while ((i < 5) && (file_size == 0U)) {
    Sleep(boost::posix_time::milliseconds(seed));
    file_size = fs::file_size(file_path);
    ++i;
    LOG(kWarning) << "Re-read attempt " << i << " after sleeping for " << seed
                  << " ms, file size is " << file_size;
    seed *= 2;
  }

  fs::ifstream file_in(file_path, std::ios::in | std::ios::binary);

  std::vector<char> file_content(static_cast<unsigned int>(file_size));
  file_in.read(&file_content[0], file_size);
  file_in.close();
  return NonEmptyString(std::string(&file_content[0], static_cast<unsigned int>(file_size)));
}

bool WriteFile(const fs::path &file_path, const std::string &content) {
  try {
    if (!file_path.has_filename()) {
      LOG(kError) << "Failed to write: file_path " << file_path
                  << " has no filename";
      return false;
    }
    fs::ofstream file_out(file_path, std::ios::out | std::ios::trunc |
                                     std::ios::binary);
    if (!file_out.good()) {
      LOG(kError) << "Can't get ofstream created for " << file_path;
      return false;
    }
    file_out.write(content.data(), content.size());
    file_out.close();
  }
  catch(const std::exception &e) {
    LOG(kError) << "Failed to write file " << file_path << ": " << e.what();
    return false;
  }
  return true;
}

bool Sleep(const boost::posix_time::time_duration &duration) {
  try {
    boost::this_thread::sleep(duration);
  }
  catch(const boost::thread_interrupted&) {
    LOG(kWarning) << "Thread was interrupted while sleeping for " << duration;
    return false;
  }
  return true;
}


fs::path GetHomeDir() {
#if defined(MAIDSAFE_WIN32)
  std::string env_home2(getenv("HOMEPATH"));
  std::string env_home_drive(getenv("HOMEDRIVE"));
  if ((!env_home2.empty()) && (!env_home_drive.empty()))
    return fs::path(env_home_drive + env_home2);
#elif defined(MAIDSAFE_APPLE) || defined(MAIDSAFE_LINUX)
  struct passwd *p = getpwuid(getuid());  // NOLINT (dirvine)
  std::string home(p->pw_dir);
  if (!home.empty())
    return fs::path(home);
  std::string env_home(getenv("HOME"));
  if (!env_home.empty())
    return fs::path(env_home);
#endif
  LOG(kError) << "Cannot deduce home directory path";
  return fs::path();
}

fs::path GetUserAppDir() {
  const fs::path kHomeDir(GetHomeDir());
  if (kHomeDir.empty()) {
    LOG(kError) << "Cannot deduce user application directory path";
    return fs::path();
  }
#if defined(MAIDSAFE_WIN32)
  return fs::path(getenv("APPDATA")) / kCompanyName / kApplicationName;
#elif defined(MAIDSAFE_APPLE)
  return kHomeDir / "/Library/Application Support/" / kCompanyName /
         kApplicationName;
#elif defined(MAIDSAFE_LINUX)
  return kHomeDir / ".config" / kCompanyName / kApplicationName;
#else
  LOG(kError) << "Cannot deduce user application directory path";
  return fs::path();
#endif
}


fs::path GetSystemAppSupportDir() {
#if defined(MAIDSAFE_WIN32)
  return fs::path(getenv("ALLUSERSPROFILE")) / kCompanyName / kApplicationName;
#elif defined(MAIDSAFE_APPLE)
  return fs::path("/Library/Application Support/") / kCompanyName /
         kApplicationName;
#elif defined(MAIDSAFE_LINUX)
  return fs::path("/usr/share/") / kCompanyName / kApplicationName;
#else
  LOG(kError) << "Cannot deduce system wide application directory path";
  return fs::path();
#endif
}

unsigned int Concurrency() {
  return std::max(std::thread::hardware_concurrency(), 2U);
}

fs::path GetAppInstallDir() {
#if defined(MAIDSAFE_WIN32)
  char* program_files;
#if defined(MAIDSAFE_WIN64)
  program_files = getenv("ProgramFiles(x86)");
#else
  program_files = getenv("ProgramFiles");
#endif
  return fs::path(program_files) / kCompanyName / kApplicationName;
#elif defined(MAIDSAFE_APPLE)
  return fs::path("/Applications/");
#elif defined(MAIDSAFE_LINUX)
  return fs::path("/usr/bin/");
#else
  LOG(kError) << "Cannot deduce application directory path";
  return fs::path();
#endif
}


namespace test {

TestPath CreateTestPath(std::string test_prefix) {
  if (test_prefix.empty())
    test_prefix = "MaidSafe_Test";

  if (test_prefix.substr(0, 13) != "MaidSafe_Test" &&
      test_prefix.substr(0, 12) != "Sigmoid_Test") {
    LOG(kWarning) << "Test prefix should preferably be \"MaidSafe_Test<optional"
                 << " test name>\" or \"Sigmoid_Test<optional test name>\".";
  }

  test_prefix += "_%%%%-%%%%-%%%%";

  boost::system::error_code error_code;
  fs::path *test_path(new fs::path(fs::unique_path(
      fs::temp_directory_path(error_code) / test_prefix)));
  std::string debug(test_path->string());
  TestPath test_path_ptr(test_path, [debug](fs::path *delete_path) {
        if (!delete_path->empty()) {
          boost::system::error_code ec;
          if (fs::remove_all(*delete_path, ec) == 0) {
            LOG(kWarning) << "Failed to remove " << *delete_path;
          }
          if (ec.value() != 0) {
            LOG(kWarning) << "Error removing " << *delete_path << "  "
                          << ec.message();
          }
        }
        delete delete_path;
      });
  if (error_code) {
    LOG(kWarning) << "Can't get a temp directory: " << error_code.message();
    return TestPath(new fs::path);
  }

  if (!fs::create_directories(*test_path, error_code) || error_code) {
    LOG(kWarning) << "Failed to create test directory " << *test_path
                 << "  (error message: " << error_code.message() << ")";
    return TestPath(new fs::path);
  }

  LOG(kInfo) << "Created test directory " << *test_path;
  return test_path_ptr;
}

void RunInParallel(int thread_count, std::function<void()> functor) {
  std::vector<std::future<void>> futures;
  for (int i = 0; i < thread_count; ++i)
    futures.push_back(std::async(std::launch::async, functor));
  for (auto& future : futures)
    future.get();
}

uint16_t GetRandomPort() {
  static std::set<uint16_t> already_used_ports;
  bool unique(false);
  uint16_t port(0);
  do {
    port = (RandomUint32() % 64511) + 1025;
    unique = (already_used_ports.insert(port)).second;
  } while (!unique);
  if (already_used_ports.size() == 10000) {
    LOG(kInfo) << "Clearing already-used ports list.";
    already_used_ports.clear();
  }
  return port;
}

}  // namespace test

}  // namespace maidsafe
