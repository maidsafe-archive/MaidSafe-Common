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

#ifndef MAIDSAFE_COMMON_UTILS_H_
#define MAIDSAFE_COMMON_UTILS_H_

#include <chrono>
#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <ratio>
#include <string>

#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/udp.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/program_options.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"

#include "maidsafe/common/data_types/data_name_variant.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace detail {

boost::mt19937& random_number_generator();
std::mutex& random_number_generator_mutex();
boost::filesystem::path GetFileName(const DataNameVariant& data_name_variant);
DataNameVariant GetDataNameVariant(const boost::filesystem::path& file_name);

}  // namespace detail

extern const int kInvalidVersion;
extern const uint16_t kLivePort;
extern const boost::posix_time::ptime kMaidSafeEpoch;

// SI units.  (note MS windows will report on the 'old' system, for drive space)
// this is (cheekily) using chrono::duration
typedef std::chrono::duration<uint64_t> Bytes;
typedef std::chrono::duration<uint64_t, std::kilo> KiloBytes;
typedef std::chrono::duration<uint64_t, std::mega> MegaBytes;
typedef std::chrono::duration<uint64_t, std::giga> GigaBytes;
typedef std::chrono::duration<uint64_t, std::tera> TeraBytes;
typedef std::chrono::duration<uint64_t, std::peta> PetaBytes;
typedef std::chrono::duration<uint64_t, std::exa> ExaBytes;

// Makes a UDP socket connection to peer_endpoint.  Note, no data is sent, so no information about
// the validity or availability of the peer is deduced.  If the retrieved local endpoint is
// unspecified or is the loopback address, the function returns a default-constructed (unspecified)
// address.
boost::asio::ip::address GetLocalIp(
    boost::asio::ip::udp::endpoint peer_endpoint =
        boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string("203.0.113.0"),
                                       80));

// Takes a version as a string and returns the int form, e.g. "0.09.01" returns 901
int VersionToInt(const std::string& version);

// Converts num bytes to nearest integral decimal SI value.
std::string BytesToDecimalSiUnits(uint64_t num);

// Converts num bytes to nearest integral binary SI value.
std::string BytesToBinarySiUnits(uint64_t num);

// Borrowed from http://burtleburtle.net/bob/rand/smallprng.html
// Useful for a wait free very fast prng which passes DIEHARD
namespace smallprng {
  typedef uint32_t  u4;
  typedef struct ranctx { u4 a; u4 b; u4 c; u4 d; } ranctx;

#define smallprng_rot(x, k) (((x) << (k))|((x) >> (32 - (k))))
  inline u4 ranval(ranctx *x) {
    u4 e = x->a - smallprng_rot(x->b, 27);
    x->a = x->b ^ smallprng_rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
  }
#undef smallprng_rot

  inline void raninit(ranctx *x, u4 seed) {
    u4 i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i = 0; i < 20; ++i) {
        (void) ranval(x);
    }
  }
}  // namespace smallprng

// Generates a non-cryptographically-secure 32bit signed integer.
int32_t RandomInt32();

// Generates a non-cryptographically-secure 32bit unsigned integer.
uint32_t RandomUint32();

// Generates a non-cryptographically-secure random string.
std::string RandomString(size_t size);

// Generates a non-cryptographically-secure random string.
template <typename String>
String GetRandomString(size_t size) {
  boost::uniform_int<> uniform_distribution(0, 255);
  String random_string(size, 0);
  {
    std::lock_guard<std::mutex> lock(detail::random_number_generator_mutex());
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        detail::random_number_generator(), uniform_distribution);
    std::generate(random_string.begin(), random_string.end(), uni);
  }
  return random_string;
}

// Generates a non-cryptographically-secure random string containing only
// alphanumeric characters.
std::string RandomAlphaNumericString(size_t size);

template <typename String>
String GetRandomAlphaNumericString(size_t size) {
  static const char alpha_numerics[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  boost::uniform_int<> uniform_distribution(0, 61);
  String random_string(size, 0);
  {
    std::lock_guard<std::mutex> lock(detail::random_number_generator_mutex());
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        detail::random_number_generator(), uniform_distribution);
    for (auto it = random_string.begin(); it != random_string.end(); ++it)
      *it = alpha_numerics[uni()];
  }
  return random_string;
}

std::string HexEncode(const std::string& non_hex_input);

template <size_t min, size_t max>
std::string HexEncode(const detail::BoundedString<min, max>& non_hex_input) {
  return HexEncode(non_hex_input.string());
}

std::string HexDecode(const std::string& hex_input);

// hacked from https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
std::string Base64Encode(const std::string& non_base64_input);

template <size_t min, size_t max>
std::string Base64Encode(const detail::BoundedString<min, max>& non_base64_input) {
  return Base64Encode(non_base64_input.string());
}

std::string Base64Decode(const std::string& base64_input);

// Returns an appreviated hex representation of a hash or other small data.
std::string HexSubstr(const std::string& non_hex);

template <size_t min, size_t max>
std::string HexSubstr(const detail::BoundedString<min, max>& non_hex) {
  return HexSubstr(non_hex.string());
}

// Returns an appreviated Base64 representation of a hash or other small data.
std::string Base64Substr(const std::string& non_base64);

template <size_t min, size_t max>
std::string Base64Substr(detail::BoundedString<min, max> non_base64) {
  return Base64Substr(non_base64.string());
}

#ifdef MAIDSAFE_WIN32
// Throws if any char of 'input' can't be converted.
std::string WstringToString(const std::wstring& input);

// Throws if any char of 'input' can't be converted.
std::wstring StringToWstring(const std::string& input);
#endif

// Returns an abbreviated hex representation of id.  Throws if 'id' is uninitialised.
std::string DebugId(const Identity& id);

// Returns the number of milliseconds since kMaidsafeEpoch (1st January 2000).
uint64_t GetTimeStamp();

// Converts 'timestamp' to ptime where 'timestamp' is the result of a call to 'GetTimeStamp()'.
boost::posix_time::ptime TimeStampToPtime(uint64_t timestamp);

// Reads the given file and returns the contents as a string.
bool ReadFile(const boost::filesystem::path& file_path, std::string* content);
NonEmptyString ReadFile(const boost::filesystem::path& file_path);

// Writes the given content string to a file, overwriting if applicable.
bool WriteFile(const boost::filesystem::path& file_path, const std::string& content);

// For use with std::chrono durations - provides a non-interruptible sleep.
template <typename Rep, typename Period>
void Sleep(const std::chrono::duration<Rep, Period>& duration) {
  std::this_thread::sleep_for(duration);
}

boost::filesystem::path GetPathFromProgramOptions(
    const std::string& option_name, const boost::program_options::variables_map& variables_map,
    bool is_dir, bool create_new_if_absent);

// Returns max of (2, hardware_concurrency)
unsigned int Concurrency();

template <typename T>
bool IsReady(std::future<T>& future) {
  return future.wait_for(std::chrono::seconds::zero()) == std::future_status::ready;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_UTILS_H_
