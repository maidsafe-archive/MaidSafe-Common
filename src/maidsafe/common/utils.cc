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
#include "pwd.h"  // NOLINT (dirvine)
#include "sys/param.h"
#endif

#ifdef _MSC_VER
#include "windows.h"  // NOLINT - Viv
#endif

#include "boost/config.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/format.hpp"
#include "boost/token_functions.hpp"
#include "boost/variant/apply_visitor.hpp"

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

struct BinaryUnit;
struct DecimalUnit;

template <typename Units>
struct UnitType {};

template <>
struct UnitType<BinaryUnit> {
  static const uint64_t kKilo = 1024;
  // static const uint64_t kExaThreshold = 11529215046068469760U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = {{" B", " KiB", " MiB", " GiB", " TiB", " PiB", " EiB"}};
    return temp;
  }
};

template <>
struct UnitType<DecimalUnit> {
  static const uint64_t kKilo = 1000;
  // static const uint64_t kExaThreshold = 9500000000000000000U;
  static std::array<std::string, 7> Qualifier() {
    std::array<std::string, 7> temp = {{" B", " kB", " MB", " GB", " TB", " PB", " EB"}};
    return temp;
  }
};

template <typename Units>
std::string BytesToSiUnits(uint64_t num) {
  static const auto to_string = [](double d) { return (boost::format("%.2f") % d).str(); };
  const uint64_t kKilo(UnitType<Units>::kKilo);
  std::array<std::string, 7> qualifier = UnitType<Units>::Qualifier();

  if (num < kKilo)
    return std::to_string(num) + qualifier[0];

  size_t count(1);
  uint64_t threshold(0), midpoint(kKilo / 2), divisor(kKilo);
  for (; count != 6; midpoint *= kKilo, divisor *= kKilo, ++count) {
    threshold = (divisor * kKilo) - midpoint;
    if (num < threshold)
      return to_string(double(num) / divisor) + qualifier[count];
  }

  return to_string(double(num) / divisor) + qualifier[6];
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
  const auto result(converter.out(state, &input[0], &input[input.size()], from_next, &output[0],
                                  &output[output.size()], to_next));
  if (result != Converter::ok && result != Converter::noconv)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_conversion));

  output.resize(to_next - &output[0]);
  return output;
}

uint32_t& rng_seed() {
#ifdef _MSC_VER
  // Work around high_resolution_clock being the lowest resolution clock pre-VS14
  static uint32_t seed([] {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return static_cast<uint32_t>(t.LowPart);
  }());
#else
  static uint32_t seed(
      static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
#endif
  return seed;
}

template <typename IntType>
IntType RandomInt() {
  static std::uniform_int_distribution<IntType> distribution(std::numeric_limits<IntType>::min(),
                                                             std::numeric_limits<IntType>::max());
  std::lock_guard<std::mutex> lock(detail::random_number_generator_mutex());
  return distribution(detail::random_number_generator());
}

}  // unnamed namespace

namespace detail {

std::mt19937& random_number_generator() {
  static std::mt19937 random_number_generator(rng_seed());
  return random_number_generator;
}

std::mutex& random_number_generator_mutex() {
  static std::mutex random_number_generator_mutex;
  return random_number_generator_mutex;
}

#ifdef TESTING

uint32_t random_number_generator_seed() { return rng_seed(); }

void set_random_number_generator_seed(uint32_t seed) {
  std::lock_guard<std::mutex> lock(random_number_generator_mutex());
  rng_seed() = seed;
  random_number_generator().seed(seed);
}

#endif

fs::path GetFileName(const Data::NameAndTypeId& name_and_type_id) {
  return (hex::Encode(name_and_type_id.name) + '_' + std::to_string(name_and_type_id.type_id.data));
}

Data::NameAndTypeId GetDataNameAndTypeId(const boost::filesystem::path& file_name) {
  std::string file_name_str(file_name.string());
  size_t index(file_name_str.rfind('_'));
  auto type_id(static_cast<DataTypeId>(std::stoul(file_name_str.substr(index + 1))));
  Identity name(hex::DecodeToBytes(file_name_str.substr(0, index)));
  return Data::NameAndTypeId(std::move(name), type_id);
}

}  // namespace detail

const int kInvalidVersion(-1);
const uint16_t kLivePort(5483);
const bptime::ptime kMaidSafeEpoch(bptime::from_iso_string("20000101T000000"));  // 01 Jan 2000

asio::ip::address GetLocalIp(asio::ip::udp::endpoint peer_endpoint) {
  asio::io_service io_service;
  try {
    asio::ip::udp::socket socket(io_service);
    socket.connect(peer_endpoint);
    if (socket.local_endpoint().address().is_unspecified() ||
        socket.local_endpoint().address().is_loopback())
      return asio::ip::address();
    return socket.local_endpoint().address();
  } catch (const std::exception& e) {
    LOG(kError) << "Failed trying to connect to " << peer_endpoint << " - "
                << boost::diagnostic_information(e);
    return asio::ip::address();
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
  } catch (const std::logic_error& e) {
    LOG(kWarning) << "Invalid version " << version << ": " << boost::diagnostic_information(e);
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

int32_t RandomInt32() { return RandomInt<int32_t>(); }

uint32_t RandomUint32() { return RandomInt<uint32_t>(); }

std::string RandomString(size_t size) { return GetRandomString<std::string>(size); }

std::string RandomString(uint32_t min, uint32_t max) {
  return GetRandomString<std::string>((RandomUint32() % max - min + 1) + min);
}

std::vector<byte> RandomBytes(size_t size) { return GetRandomString<std::vector<byte>>(size); }

std::vector<byte> RandomBytes(uint32_t min, uint32_t max) {
  return GetRandomString<std::vector<byte>>((RandomUint32() % max - min + 1) + min);
}

std::string RandomAlphaNumericString(size_t size) {
  return GetRandomAlphaNumericString<std::string>(size);
}

std::string WstringToString(const std::wstring& input) {
  return StringToString<wchar_t, char>(input);
}

#ifdef MAIDSAFE_WIN32
std::wstring StringToWstring(const std::string& input) {
  return StringToString<char, wchar_t>(input);
}
#endif

uint64_t GetTimeStamp() {
  return (bptime::microsec_clock::universal_time() - kMaidSafeEpoch).total_milliseconds();
}

boost::posix_time::ptime TimeStampToPtime(uint64_t timestamp) {
  return kMaidSafeEpoch + bptime::milliseconds(timestamp);
}

boost::expected<std::vector<byte>, common_error> ReadFile(const fs::path& file_path) {
  try {
    uintmax_t file_size(fs::file_size(file_path));
    if (file_size > std::numeric_limits<size_t>::max()) {
      LOG(kError) << "Failed to read file " << file_path << ": File size " << file_size
        << " too large (over " << std::numeric_limits<size_t>::max() << ")";
      return boost::make_unexpected(MakeError(CommonErrors::file_too_large));
    }

    std::basic_ifstream<byte> file_in(file_path.c_str(), std::ios::in | std::ios::binary);

    std::vector<byte> file_content(static_cast<size_t>(file_size));
    file_in.read(&file_content[0], file_size);
    file_in.close();
    return file_content;
  }
  catch (const std::exception& e) {
    LOG(kError) << "Failed to read file " << file_path << ": " << e.what();
  }
  return boost::make_unexpected(MakeError(CommonErrors::filesystem_io_error));
}

bool WriteFile(const boost::filesystem::path& file_path, const std::vector<byte>& content) {
  try {
    if (!file_path.has_filename()) {
      LOG(kError) << "Failed to write: file_path " << file_path << " has no filename";
      return false;
    }
    std::basic_ofstream<byte> file_out(file_path.c_str(),
                                       std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file_out.good()) {
      LOG(kError) << "Can't get ofstream created for " << file_path;
      return false;
    }
    file_out.write(content.data(), content.size());
    file_out.close();
  } catch (const std::exception& e) {
    LOG(kError) << "Failed to write file " << file_path << ": " << boost::diagnostic_information(e);
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
        } catch (const std::exception& e) {
          LOG(kError) << "Exception while creating new file: " << boost::diagnostic_information(e);
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
