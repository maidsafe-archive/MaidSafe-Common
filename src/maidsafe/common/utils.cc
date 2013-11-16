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

#include "maidsafe/common/utils.h"

#include <ctype.h>
#include <algorithm>
#include <array>
#include <ctime>
#include <cwchar>
#include <fstream>
#include <limits>
#include <locale>  // NOLINT
#include <set>
#include <thread>
#include <vector>

#if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__) ||             \
    (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || \
     defined(__GLIBC__)) &&                                                          \
        !defined(_CRAYC)
#include  "pwd.h"  // NOLINT (dirvine)
#include "sys/param.h"
#endif

#ifdef _MSC_VER
#include "windows.h"  // NOLINT - Viv
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

#include "maidsafe/common/config.h"
#include "maidsafe/common/log.h"

namespace fs = boost::filesystem;
namespace bptime = boost::posix_time;
namespace po = boost::program_options;

namespace maidsafe {

namespace {

boost::mt19937 g_random_number_generator(static_cast<unsigned int>(
    bptime::microsec_clock::universal_time().time_of_day().total_microseconds()));
std::mutex g_random_number_generator_mutex;

struct BinaryUnit;
struct DecimalUnit;

template <typename Units>
struct UnitType {};

template <>
struct UnitType<BinaryUnit> {
  static const uint64_t kKilo = 1024;
  static const uint64_t kExaThreshold = 11529215046068469760U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = {{" B", " KiB", " MiB", " GiB", " TiB", " PiB", " EiB"}};
    return temp;
  }
};

template <>
struct UnitType<DecimalUnit> {
  static const uint64_t kKilo = 1000;
  static const uint64_t kExaThreshold = 9500000000000000000U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = {{" B", " kB", " MB", " GB", " TB", " PB", " EB"}};
    return temp;
  }
};

template <typename Units>
std::string BytesToSiUnits(uint64_t num) {
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
  return num < threshold ? (std::to_string((num + midpoint) / divisor) + qualifier[6])
                         : (std::to_string(((num - midpoint) / divisor) + 1) + qualifier[6]);
}

const char kHexAlphabet[] = "0123456789abcdef";
const char kHexLookup[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,  8,  9,  0,  0,  0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

const char kBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char kPadCharacter('=');

template <typename CharIn, typename CharOut>
std::basic_string<CharOut> StringToString(const std::basic_string<CharIn>& input) {
  // TODO(Fraser#5#): 2013-11-01 - Use C++11's std::wstring_convert once available.
  const std::locale kLocale("");
  typedef std::codecvt<CharIn, CharOut, std::mbstate_t> Converter;
  const Converter& converter = std::use_facet<Converter>(kLocale);
  std::basic_string<CharOut> output(input.size() * converter.max_length(), 0);
  std::mbstate_t state;
  const CharIn* from_next;
  CharOut* to_next;
  const auto result(converter.out(state, &input[0], &input[input.size()], from_next,
                                  &output[0], &output[output.size()], to_next));
  if (result != Converter::ok && result != Converter::noconv)
    ThrowError(CommonErrors::invalid_conversion);

  output.resize(to_next - &output[0]);
  return output;
}

}  // unnamed namespace

namespace detail {

boost::mt19937& random_number_generator() { return g_random_number_generator; }
std::mutex& random_number_generator_mutex() { return g_random_number_generator_mutex; }

}  // namespace detail

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
  catch (const std::exception& e) {
    LOG(kError) << "Failed trying to connect to " << peer_endpoint << " - " << e.what();
    return boost::asio::ip::address();
  }
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
  catch (const std::logic_error& e) {
    LOG(kWarning) << "Invalid version " << version << ": " << e.what();
    return kInvalidVersion;
  }

  if (major_version < 0 || minor_version < 0 || patch_level < 0) {
    LOG(kWarning) << "Invalid version " << version;
    return kInvalidVersion;
  }

  return (major_version * 10000) + (minor_version * 1000) + patch_level;
}

std::string BytesToDecimalSiUnits(uint64_t num) { return BytesToSiUnits<DecimalUnit>(num); }

std::string BytesToBinarySiUnits(uint64_t num) { return BytesToSiUnits<BinaryUnit>(num); }

int32_t RandomInt32() {
  boost::uniform_int<> uniform_distribution(0, boost::integer_traits<int32_t>::const_max);
  std::lock_guard<std::mutex> lock(g_random_number_generator_mutex);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(g_random_number_generator,
                                                                      uniform_distribution);
  return uni();
}

uint32_t RandomUint32() { return static_cast<uint32_t>(RandomInt32()); }

std::string RandomString(size_t size) { return GetRandomString<std::string>(size); }

std::string RandomAlphaNumericString(size_t size) {
  return GetRandomAlphaNumericString<std::string>(size);
}

std::string HexEncode(const std::string& non_hex_input) {
  auto size(non_hex_input.size());
  std::string hex_output(size * 2, 0);
  for (size_t i(0), j(0); i != size; ++i) {
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) / 16];
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) % 16];
  }
  return hex_output;
}

std::string HexDecode(const std::string& hex_input) {
  auto size(hex_input.size());
  if (size % 2)  // Sanity check
    ThrowError(CommonErrors::invalid_conversion);

  std::string non_hex_output(size / 2, 0);
  for (size_t i(0), j(0); i != size / 2; ++i) {
    non_hex_output[i] = (kHexLookup[static_cast<int>(hex_input[j++])] << 4);
    non_hex_output[i] |= kHexLookup[static_cast<int>(hex_input[j++])];
  }
  return non_hex_output;
}

std::string Base64Encode(const std::string& non_base64_input) {
  std::basic_string<byte> encoded_string(
      ((non_base64_input.size() / 3) + (non_base64_input.size() % 3 > 0)) * 4, 0);
  int32_t temp;
  auto cursor = std::begin(reinterpret_cast<const std::basic_string<byte>&>(non_base64_input));
  size_t i = 0;
  size_t common_output_size((non_base64_input.size() / 3) * 4);
  while (i < common_output_size) {
    temp = (*cursor++) << 16;  // Convert to big endian
    temp += (*cursor++) << 8;
    temp += (*cursor++);
    encoded_string[i++] = kBase64Alphabet[(temp & 0x00FC0000) >> 18];
    encoded_string[i++] = kBase64Alphabet[(temp & 0x0003F000) >> 12];
    encoded_string[i++] = kBase64Alphabet[(temp & 0x00000FC0) >> 6];
    encoded_string[i++] = kBase64Alphabet[(temp & 0x0000003F)];
  }
  switch (non_base64_input.size() % 3) {
    case 1:
      temp = (*cursor++) << 16;  // Convert to big endian
      encoded_string[i++] = kBase64Alphabet[(temp & 0x00FC0000) >> 18];
      encoded_string[i++] = kBase64Alphabet[(temp & 0x0003F000) >> 12];
      encoded_string[i++] = kPadCharacter;
      encoded_string[i++] = kPadCharacter;
      break;
    case 2:
      temp = (*cursor++) << 16;  // Convert to big endian
      temp += (*cursor++) << 8;
      encoded_string[i++] = kBase64Alphabet[(temp & 0x00FC0000) >> 18];
      encoded_string[i++] = kBase64Alphabet[(temp & 0x0003F000) >> 12];
      encoded_string[i++] = kBase64Alphabet[(temp & 0x00000FC0) >> 6];
      encoded_string[i++] = kPadCharacter;
      break;
  }
  return std::string(std::begin(encoded_string), std::end(encoded_string));
}

std::string Base64Decode(const std::string& base64_input) {
  if (base64_input.size() % 4)  // Sanity check
    ThrowError(CommonErrors::invalid_conversion);

  size_t padding = 0;
  if (base64_input.size()) {
    if (base64_input[base64_input.size() - 1] == static_cast<size_t>(kPadCharacter))
      ++padding;
    if (base64_input[base64_input.size() - 2] == static_cast<size_t>(kPadCharacter))
      ++padding;
  }

  // Setup a vector to hold the result
  std::string decoded_bytes;
  decoded_bytes.reserve(((base64_input.size() / 4) * 3) - padding);
  int32_t temp = 0;  // Holds decoded quanta
  auto cursor = std::begin(base64_input);
  while (cursor < std::end(base64_input)) {
    for (size_t quantum_position = 0; quantum_position < 4; ++quantum_position) {
      temp <<= 6;
      if (*cursor >= 0x41 && *cursor <= 0x5A)
        temp |= *cursor - 0x41;
      else if (*cursor >= 0x61 && *cursor <= 0x7A)
        temp |= *cursor - 0x47;
      else if (*cursor >= 0x30 && *cursor <= 0x39)
        temp |= *cursor + 0x04;
      else if (*cursor == 0x2B)
        temp |= 0x3E;  // change to 0x2D for URL alphabet
      else if (*cursor == 0x2F)
        temp |= 0x3F;                       // change to 0x5F for URL alphabet
      else if (*cursor == kPadCharacter) {  // pad
        switch (std::end(base64_input) - cursor) {
          case 1:  // One pad character
            decoded_bytes.push_back((temp >> 16) & 0x000000FF);
            decoded_bytes.push_back((temp >> 8) & 0x000000FF);
            return decoded_bytes;
          case 2:  // Two pad characters
            decoded_bytes.push_back((temp >> 10) & 0x000000FF);
            return decoded_bytes;
          default:
            ThrowError(CommonErrors::invalid_conversion);
        }
      } else {
        ThrowError(CommonErrors::invalid_conversion);
      }
      ++cursor;
    }
    decoded_bytes.push_back((temp >> 16) & 0x000000FF);
    decoded_bytes.push_back((temp >> 8) & 0x000000FF);
    decoded_bytes.push_back(temp & 0x000000FF);
  }
  return decoded_bytes;
}

std::string HexSubstr(const std::string& non_hex) {
  size_t non_hex_size(non_hex.size());
  if (non_hex_size < 7)
    return HexEncode(non_hex);

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

std::string Base64Substr(const std::string& non_base64) {
  std::string base64(Base64Encode(non_base64));
  if (base64.size() > 16)
    return (base64.substr(0, 7) + ".." + base64.substr(base64.size() - 7));
  else
    return base64;
}

std::string WstringToString(const std::wstring &input) {
  return StringToString<wchar_t, char>(input);
}

#ifdef MAIDSAFE_WIN32
std::wstring StringToWstring(const std::string& input) {
  return StringToString<char, wchar_t>(input);
}
#endif

std::string DebugId(const Identity& id) {
  return id.IsInitialised() ? "Uninitialised Identity" : HexSubstr(id.string());
}

bptime::time_duration GetDurationSinceEpoch() {
  // 01 Jan 2000
  static const boost::posix_time::ptime kMaidSafeEpoch(bptime::from_iso_string("20000101T000000"));
  return bptime::microsec_clock::universal_time() - kMaidSafeEpoch;
}

uint32_t GetTimeStamp() {
  bptime::time_duration since_epoch(GetDurationSinceEpoch());
  return since_epoch.total_seconds();
}

bool ReadFile(const fs::path& file_path, std::string* content) {
  if (!content) {
    LOG(kError) << "Failed to read file " << file_path << ": NULL pointer passed";
    return false;
  }

  try {
    uintmax_t file_size(fs::file_size(file_path));
    if (file_size > std::numeric_limits<size_t>::max()) {
      LOG(kError) << "Failed to read file " << file_path << ": File size " << file_size
                  << " too large (over " << std::numeric_limits<size_t>::max() << ")";
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
  catch (const std::exception& e) {
    LOG(kError) << "Failed to read file " << file_path << ": " << e.what();
    return false;
  }
  return true;
}

NonEmptyString ReadFile(const fs::path& file_path) {
  uintmax_t file_size(fs::file_size(file_path));
  if (file_size > std::numeric_limits<size_t>::max())
    ThrowError(CommonErrors::file_too_large);

  std::ifstream file_in(file_path.c_str(), std::ios::in | std::ios::binary);

  std::vector<char> file_content(static_cast<size_t>(file_size));
  file_in.read(&file_content[0], file_size);
  file_in.close();
  return NonEmptyString(std::string(&file_content[0], static_cast<size_t>(file_size)));
}

bool WriteFile(const fs::path& file_path, const std::string& content) {
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
  catch (const std::exception& e) {
    LOG(kError) << "Failed to write file " << file_path << ": " << e.what();
    return false;
  }
  return true;
}

bool InterruptibleSleep(const boost::chrono::high_resolution_clock::duration& duration) {
  try {
    boost::this_thread::sleep_for(duration);
  }
  catch (const boost::thread_interrupted&) {
    LOG(kWarning) << "Thread was interrupted while sleeping for "
                  << boost::chrono::duration_cast<boost::chrono::microseconds>(duration).count()
                  << " microseconds.";
    return false;
  }
  return true;
}

fs::path GetPathFromProgramOptions(const std::string& option_name,
                                   const po::variables_map& variables_map, bool is_dir,
                                   bool create_new_if_absent) {
  fs::path option_path;
  if (variables_map.count(option_name))
    option_path = variables_map.at(option_name).as<std::string>();
  if (option_path.empty())
    return fs::path();

  boost::system::error_code ec;
  if (!fs::exists(option_path, ec) || ec) {
    if (!create_new_if_absent) {
      LOG(kError) << "Invalid " << option_name << ", " << option_path
                  << " doesn't exist or can't be accessed (" << ec.message() << ")";
      return fs::path();
    }

    if (is_dir) {  // Create new dir
      fs::create_directories(option_path, ec);
      if (ec) {
        LOG(kError) << "Unable to create new dir " << option_path << " (" << ec.message() << ")";
        return fs::path();
      }
    } else {  // Create new file
      if (option_path.has_filename()) {
        try {
          std::ofstream(option_path.c_str());
        }
        catch (const std::exception& e) {
          LOG(kError) << "Exception while creating new file: " << e.what();
          return fs::path();
        }
      }
    }
  }

  if (is_dir) {
    if (!fs::is_directory(option_path, ec) || ec) {
      LOG(kError) << "Invalid " << option_name << ", " << option_path << " is not a directory ("
                  << ec.message() << ")";
      return fs::path();
    }
  } else {
    if (!fs::is_regular_file(option_path, ec) || ec) {
      LOG(kError) << "Invalid " << option_name << ", " << option_path << " is not a regular file ("
                  << ec.message() << ")";
      return fs::path();
    }
  }

  LOG(kInfo) << '\"' << option_name << "\" option is " << option_path;
  return option_path;
}

unsigned int Concurrency() { return std::max(std::thread::hardware_concurrency(), 2U); }

}  // namespace maidsafe
