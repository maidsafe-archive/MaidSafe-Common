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
#include <mutex>
#include <random>
#include <ratio>
#include <string>

#include "asio/ip/address.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/address_v6.hpp"
#include "asio/ip/udp.hpp"
#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/address_v4.hpp"
#include "boost/asio/ip/address_v6.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/expected/expected.hpp"
#include "boost/program_options.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/data_types/data.h"

namespace maidsafe {

namespace detail {

class Spinlock {
 public:
  Spinlock() : flag(false) {}
  void lock() {
    bool v;
    while (v = 0, !flag.compare_exchange_weak(v, 1, std::memory_order_acquire,
          std::memory_order_acquire))
      std::this_thread::yield();
  }
  void unlock() {
    flag.store(false, std::memory_order_release);
  }
 private:
  std::atomic<bool> flag;
};

std::mt19937& random_number_generator();
std::mutex& random_number_generator_mutex();
#ifdef TESTING
uint32_t random_number_generator_seed();
void set_random_number_generator_seed(uint32_t seed);
#endif
boost::filesystem::path GetFileName(const Data::NameAndTypeId& name_and_type_id);
Data::NameAndTypeId GetDataNameAndTypeId(const boost::filesystem::path& file_name);

}  // namespace detail

extern const int kInvalidVersion;
extern const uint16_t kLivePort;
extern const boost::posix_time::ptime kMaidSafeEpoch;

// Makes a UDP socket connection to peer_endpoint.  Note, no data is sent, so no information about
// the validity or availability of the peer is deduced.  If the retrieved local endpoint is
// unspecified or is the loopback address, the function returns a default-constructed (unspecified)
// address.
asio::ip::address GetLocalIp(
    asio::ip::udp::endpoint peer_endpoint =
        asio::ip::udp::endpoint(asio::ip::make_address_v4("203.0.113.0"), 80));

// Takes a version as a string and returns the int form, e.g. "0.09.01" returns 901
int VersionToInt(const std::string& version);

// Converts num bytes to nearest integral decimal SI value.
std::string BytesToDecimalSiUnits(uint64_t num);

// Converts num bytes to nearest integral binary SI value.
std::string BytesToBinarySiUnits(uint64_t num);

// Borrowed from http://burtleburtle.net/bob/rand/smallprng.html
// Useful for a wait free very fast prng which passes DIEHARD
namespace small_prng {

using u4 = uint32_t;

struct RandomContext { u4 a, b, c, d; };

#define SMALL_PRNG_ROTATION(x, k) (((x) << (k))|((x) >> (32 - (k))))

inline u4 RandomValue(RandomContext *x) {
  u4 e = x->a - SMALL_PRNG_ROTATION(x->b, 27);
  x->a = x->b ^ SMALL_PRNG_ROTATION(x->c, 17);
  x->b = x->c + x->d;
  x->c = x->d + e;
  x->d = e + x->a;
  return x->d;
}

#undef SMALL_PRNG_ROTATION

inline void Initialise(RandomContext *x, u4 seed) {
  u4 i;
  x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
  for (i = 0; i < 20; ++i)
    static_cast<void>(RandomValue(x));
}

}  // namespace small_prng

// Generates a non-cryptographically-secure 32bit signed integer.
int32_t RandomInt32();

// Generates a non-cryptographically-secure 32bit unsigned integer.
uint32_t RandomUint32();

// Generates a non-cryptographically-secure random string of exact size.
std::string RandomString(size_t size);

// Generates a non-cryptographically-secure random string of random size between 'min' and 'max'
// inclusive.
std::string RandomString(uint32_t min, uint32_t max);

// Generates a non-cryptographically-secure random byte vector of exact size.
std::vector<byte> RandomBytes(size_t size);

// Generates a non-cryptographically-secure random byte vector of random size between 'min' and
// 'max' inclusive.
std::vector<byte> RandomBytes(uint32_t min, uint32_t max);

// Generates a non-cryptographically-secure random string of exact size.
template <typename String>
String GetRandomString(size_t size) {
  std::uniform_int_distribution<> distribution(0, 255);
  String random_string(size, 0);
  {
    std::lock_guard<std::mutex> lock(detail::random_number_generator_mutex());
    std::generate(random_string.begin(), random_string.end(),
                  [&] { return distribution(detail::random_number_generator()); });
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
  static std::uniform_int_distribution<> distribution(0, 61);
  String random_string(size, 0);
  {
    std::lock_guard<std::mutex> lock(detail::random_number_generator_mutex());
    for (auto it = random_string.begin(); it != random_string.end(); ++it)
      *it = alpha_numerics[distribution(detail::random_number_generator())];
  }
  return random_string;
}

#ifdef MAIDSAFE_WIN32
// Throws if any char of 'input' can't be converted.
std::string WstringToString(const std::wstring& input);

// Throws if any char of 'input' can't be converted.
std::wstring StringToWstring(const std::string& input);
#endif

// Returns the number of milliseconds since kMaidsafeEpoch (1st January 2000).
uint64_t GetTimeStamp();

// Converts 'timestamp' to ptime where 'timestamp' is the result of a call to 'GetTimeStamp()'.
boost::posix_time::ptime TimeStampToPtime(uint64_t timestamp);

// Reads the given file and returns the contents as a byte vector.  Doesn't throw.
boost::expected<std::vector<byte>, common_error> ReadFile(const boost::filesystem::path& file_path);

// Writes the given content string to a file, overwriting if applicable.  Doesn't throw.
bool WriteFile(const boost::filesystem::path& file_path, const std::vector<byte>& content);

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

// Performs a bitwise XOR on each char of 'lhs' with the corresponding char of 'rhs'.  Throws if
// 'lhs' and 'rhs' are not of equal size.
template <typename String>
String operator^(const String& lhs, const String& rhs) {
  const std::size_t size(lhs.size());
  if (size != rhs.size()) {
    LOG(kError) << "Cannot XOR two strings of different sizes (lhs.size() is " << size
                << " and rhs.size() is " << rhs.size() << ")";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  String result(size, 0);
  for (std::size_t i(0); i < size; ++i)
    result[i] = lhs[i] ^ rhs[i];
  return result;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_UTILS_H_
