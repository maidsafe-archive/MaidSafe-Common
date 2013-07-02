/* Copyright 2009 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#include "maidsafe/common/utils.h"

#include <ctype.h>
#include <algorithm>
#include <array>
#include <ctime>
#include <fstream>
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
#include "boost/filesystem/operations.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"
#include "boost/token_functions.hpp"

#include "cryptopp/base32.h"
#include "cryptopp/base64.h"
#include "cryptopp/hex.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"

namespace fs = boost::filesystem;
namespace bptime = boost::posix_time;

namespace maidsafe {

namespace {

boost::mt19937 g_random_number_generator(static_cast<unsigned int>(
      bptime::microsec_clock::universal_time().time_of_day().
      total_microseconds()));
std::mutex g_random_number_generator_mutex, g_srandom_number_generator_mutex;

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
    return std::to_string(num) + qualifier[0];

  size_t count(1);
  uint64_t threshold(0), midpoint(kKilo / 2), divisor(kKilo);
  for (; count != 6; midpoint *= kKilo, divisor *= kKilo, ++count) {
    threshold = (divisor * kKilo) - midpoint;
    if (num < threshold)
      return std::to_string((num + midpoint) / divisor) + qualifier[count];
  }

  threshold = UnitType<Units>::kExaThreshold;
  return num < threshold ?
         (std::to_string((num + midpoint) / divisor) + qualifier[6]) :
         (std::to_string(((num - midpoint) / divisor) + 1) + qualifier[6]);
}

const char kHexAlphabet[] = "0123456789abcdef";

}  // unnamed namespace


const bptime::ptime kMaidSafeEpoch(bptime::from_iso_string("20000101T000000"));
const int kInvalidVersion(-1);
const uint16_t kLivePort(5483);

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

  std::string full_version(std::to_string(version));
  size_t padding_count(6 - full_version.size());
  full_version.insert(0, padding_count, '0');
  std::string major_ver(full_version.substr(0, 2));
  std::string minor_ver(full_version.substr(2, 1));
  std::string patch_ver(full_version.substr(3, 3));

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
    major_version = static_cast<int16_t>(std::stoi(*(itr++)));

    minor_version = static_cast<int16_t>(std::stoi(*itr));
    if ((*itr++).size() != 1U) {
      LOG(kWarning) << "Invalid minor version " << version;
      return kInvalidVersion;
    }

    patch_level = static_cast<int16_t>(std::stoi(*itr));
    if ((*itr++).size() != 3U) {
      LOG(kWarning) << "Invalid patch level " << version;
      return kInvalidVersion;
    }
  }
  catch(const std::logic_error& e) {
    LOG(kWarning) << "Invalid version " << version << ": " << e.what();
    return kInvalidVersion;
  }

  if (major_version < 0 || minor_version < 0 || patch_level < 0) {
    LOG(kWarning) << "Invalid version " << version;
    return kInvalidVersion;
  }

  return (major_version * 10000) + (minor_version * 1000) + patch_level;
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
  boost::uniform_int<> uniform_distribution(0, boost::integer_traits<int32_t>::const_max);
  std::lock_guard<std::mutex> lock(g_random_number_generator_mutex);
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
    std::lock_guard<std::mutex> lock(g_random_number_generator_mutex);
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
    std::lock_guard<std::mutex> lock(g_random_number_generator_mutex);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        g_random_number_generator, uniform_distribution);
    for (auto & elem : random_string)
      elem = alpha_numerics[uni()];
  }
  return random_string;
}

std::string EncodeToHex(const std::string &non_hex_input) {
  auto size(non_hex_input.size());
  std::string hex_output(size * 2, 0);
  for (size_t i(0), j(0); i != size; ++i) {
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) / 16];
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) % 16];
  }
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

std::string HexSubstr(const std::string &non_hex) {
  size_t non_hex_size(non_hex.size());
  if (non_hex_size < 7)
    return EncodeToHex(non_hex);

  std::string hex(14, 0);
  size_t non_hex_index(0), hex_index(0);
  for (; non_hex_index != 3; ++non_hex_index) {
    hex[hex_index++] = kHexAlphabet[static_cast<unsigned char>(non_hex[non_hex_index]) / 16];
    hex[hex_index++] = kHexAlphabet[static_cast<unsigned char>(non_hex[non_hex_index]) % 16];
  }
  hex[hex_index++] = '.';
  hex[hex_index++] = '.';
  non_hex_index = non_hex_size - 3;
  for (; non_hex_index != non_hex_size; ++non_hex_index) {
    hex[hex_index++] = kHexAlphabet[static_cast<unsigned char>(non_hex[non_hex_index]) / 16];
    hex[hex_index++] = kHexAlphabet[static_cast<unsigned char>(non_hex[non_hex_index]) % 16];
  }
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

bptime::time_duration GetDurationSinceEpoch() {
  return bptime::microsec_clock::universal_time() - kMaidSafeEpoch;
}

uint32_t GetTimeStamp() {
  bptime::time_duration since_epoch(GetDurationSinceEpoch());
  return since_epoch.total_seconds();
}

int64_t MillisecondTimeStamp() {
  bptime::time_duration since_epoch(GetDurationSinceEpoch());
  return since_epoch.total_milliseconds();
}

std::string GetLocalTime() {
  auto now(std::chrono::system_clock::now());
  auto seconds_since_epoch(
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()));

  std::time_t now_t(
      std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::time_point(seconds_since_epoch)));

  char temp[10];
  if (!std::strftime(temp, 10, "%H:%M:%S.", std::localtime(&now_t)))  // NOLINT (Fraser)
    ThrowError(CommonErrors::unknown);

  return std::string(temp) + std::to_string((now.time_since_epoch() - seconds_since_epoch).count());
}

bool ReadFile(const fs::path &file_path, std::string *content) {
  if (!content) {
    LOG(kError) << "Failed to read file " << file_path << ": NULL pointer passed";
    return false;
  }

  try {
    uintmax_t file_size(fs::file_size(file_path));
    if (file_size > std::numeric_limits<size_t>::max()) {
      LOG(kError) << "Failed to read file " << file_path << ": File size "
                  << file_size << " too large (over "
                  << std::numeric_limits<size_t>::max() << ")";
      return false;
    }

    std::ifstream file_in(file_path.c_str(), std::ios::in | std::ios::binary);
    if (!file_in.good()) {
      LOG(kError) << "Failed to read file " << file_path << ": Bad filestream";
      return false;
    }
    if (file_size == 0U) {
      content->clear();
      return true;
    }

    content->resize(static_cast<size_t>(file_size));
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
  if (file_size > std::numeric_limits<size_t>::max())
    ThrowError(CommonErrors::file_too_large);

  std::ifstream file_in(file_path.c_str(), std::ios::in | std::ios::binary);

  std::vector<char> file_content(static_cast<size_t>(file_size));
  file_in.read(&file_content[0], file_size);
  file_in.close();
  return NonEmptyString(std::string(&file_content[0], static_cast<size_t>(file_size)));
}

bool WriteFile(const fs::path &file_path, const std::string &content) {
  try {
    if (!file_path.has_filename()) {
      LOG(kError) << "Failed to write: file_path " << file_path << " has no filename";
      return false;
    }
    std::ofstream file_out(file_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
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

bool Sleep(const bptime::time_duration &duration) {
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

fs::path GetAppInstallDir() {
#if defined(MAIDSAFE_WIN32)
  std::string program_files =
      getenv(kTargetArchitecture == "x86_64" ? "ProgramFiles(x86)" : "ProgramFiles");
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

unsigned int Concurrency() {
  return std::max(std::thread::hardware_concurrency(), 2U);
}

}  // namespace maidsafe
