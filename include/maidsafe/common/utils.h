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

#ifndef MAIDSAFE_COMMON_UTILS_H_
#define MAIDSAFE_COMMON_UTILS_H_

#include <chrono>
#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <ratio>
#include <string>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4127)
#endif

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/udp.hpp"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"


namespace maidsafe {

// 01 Jan 2000
extern const boost::posix_time::ptime kMaidSafeEpoch;

extern const int kInvalidVersion;

extern const int kLivePort;

// SI units !! (note MS windows will report on the 'old' system, for drive space)
// this is (cheeckily using chrono::duration)
typedef std::chrono::duration<uint64_t           > Bytes;
typedef std::chrono::duration<uint64_t, std::kilo> KiloBytes;
typedef std::chrono::duration<uint64_t, std::mega> MegaBytes;
typedef std::chrono::duration<uint64_t, std::giga> GigaBytes;
typedef std::chrono::duration<uint64_t, std::tera> TeraBytes;
typedef std::chrono::duration<uint64_t, std::peta> PetaBytes;
typedef std::chrono::duration<uint64_t, std::exa>  ExaBytes;

// Makes a UDP socket connection to peer_endpoint.  Note, no data is sent, so no information about
// the validity or availability of the peer is deduced.  If the retrieved local endpoint is
// unspecified or is the loopback address, the function returns a default-constructed (unspecified)
// address.
boost::asio::ip::address GetLocalIp(
    boost::asio::ip::udp::endpoint peer_endpoint =
        boost::asio::ip::udp::endpoint(
            boost::asio::ip::address_v4::from_string("203.0.113.0"), 80));

// Takes a version as an int and returns the string form, e.g. 901 returns "0.09.01"
std::string VersionToString(int version,
                            std::string* major_version = nullptr,
                            std::string* minor_version = nullptr,
                            std::string* patch_version = nullptr);

// Takes a version as a string and returns the int form, e.g. "0.09.01" returns 901
int VersionToInt(const std::string& version);

// Return CPU size (i.e. 32 64 bit etc.)
int32_t CpuSize();

// Converts num bytes to nearest integral decimal SI value.
std::string BytesToDecimalSiUnits(const uint64_t &num);

// Converts num bytes to nearest integral binary SI value.
std::string BytesToBinarySiUnits(const uint64_t &num);

// Generates a cryptographically-secure 32bit signed integer.
int32_t SRandomInt32();

// Generates a non-cryptographically-secure 32bit signed integer.
int32_t RandomInt32();

// Generates a cryptographically-secure 32bit unsigned integer.
uint32_t SRandomUint32();

// Generates a non-cryptographically-secure 32bit unsigned integer.
uint32_t RandomUint32();

// Generates a cryptographically-secure random string.
std::string SRandomString(const size_t &length);

// Generates a non-cryptographically-secure random string.
std::string RandomString(const size_t &length);

template<typename StringType>
StringType RandomSafeString(const size_t &length);

// Generates a non-cryptographically-secure random string containing only
// alphanumeric characters.
std::string RandomAlphaNumericString(const size_t &length);

template<typename StringType>
StringType RandomAlphaNumericSafeString(const size_t& length);

// Encodes a string to hex.
std::string EncodeToHex(const std::string &non_hex_input);
template<size_t min, size_t max>
std::string EncodeToHex(const detail::BoundedString<min, max> &non_hex_input) {
  return EncodeToHex(non_hex_input.string());
}

template<typename StringType>
StringType EncodeStringToHex(const StringType& non_hex_input);
template<size_t min, size_t max, typename StringType>
StringType EncodeStringToHex(const detail::BoundedString<min, max, StringType> &non_hex_input);

// Encodes a string to Base64.
std::string EncodeToBase64(const std::string &non_base64_input);
template<size_t min, size_t max>
std::string EncodeToBase64(const detail::BoundedString<min, max> &non_base64_input) {
  return EncodeToBase64(non_base64_input.string());
}

// Encodes a string to Base32.
std::string EncodeToBase32(const std::string &non_base32_input);
template<size_t min, size_t max>
std::string EncodeToBase32(const detail::BoundedString<min, max> &non_base32_input) {
  return EncodeToBase32(non_base32_input.string());
}

// Decodes a string from hex.
std::string DecodeFromHex(const std::string &hex_input);

// Decodes a string from Base64.
std::string DecodeFromBase64(const std::string &base64_input);

// Decodes a string from Base32.
std::string DecodeFromBase32(const std::string &base32_input);

// Returns an appreviated hex representation of a hash or other small data.
std::string HexSubstr(const std::string &non_hex);
template<size_t min, size_t max>
std::string HexSubstr(const detail::BoundedString<min, max> &non_hex) {
  return HexSubstr(non_hex.string());
}

template<typename StringType>
StringType HexStringSubstr(const StringType& non_hex);
template<size_t min, size_t max, typename StringType>
StringType HexStringSubstr(const detail::BoundedString<min, max, StringType> &non_hex);

// Returns an appreviated Base32 representation of a hash or other small data.
std::string Base32Substr(const std::string &non_base32);
template<size_t min, size_t max>
std::string Base32Substr(const detail::BoundedString<min, max> &non_base32) {
  return Base32Substr(non_base32.string());
}

// Returns an appreviated Base64 representation of a hash or other small data.
std::string Base64Substr(const std::string &non_base64);
template<size_t min, size_t max>
std::string Base64Substr(const detail::BoundedString<min, max> &non_base64) {
  return Base64Substr(non_base64.string());
}

// Returns an abbreviated hex representation of id.
std::string DebugId(const Identity& id);

// Returns the duration since kMaidsafeEpoch (1st January 2000).
boost::posix_time::time_duration GetDurationSinceEpoch();

// Returns the number of seconds since kMaidsafeEpoch (1st January 2000).
uint32_t GetTimeStamp();

int64_t MillisecondTimeStamp();

  // Reads the given file and returns the contents as a string.
bool ReadFile(const boost::filesystem::path &file_path, std::string *content);
NonEmptyString ReadFile(const boost::filesystem::path &file_path);

// Writes the given content string to a file, overwriting if applicable.
bool WriteFile(const boost::filesystem::path &file_path, const std::string &content);

// Causes running thread to sleep for specified duration.  Returns true if sleep completes full
// duration, returns false if the sleep is interrupted.
bool Sleep(const boost::posix_time::time_duration &duration);

// Retrieve homedir from environment
boost::filesystem::path GetHomeDir();

// Application support directory in userspace; uses kCompanyName and kApplicationName
boost::filesystem::path GetUserAppDir();

// Application support directory for all users; uses kCompanyName and kApplicationName
boost::filesystem::path GetSystemAppSupportDir();

// Application install directory; uses kCompanyName and kApplicationName
boost::filesystem::path GetAppInstallDir();

// Returns max of (2, hardware_concurrency)
unsigned int Concurrency();

template<typename T>
bool IsReady(std::future<T>& future) {
  return future.wait_for(std::chrono::seconds::zero()) == std::future_status::ready;
}

}  // namespace maidsafe

#include "maidsafe/common/utils-inl.h"

#endif  // MAIDSAFE_COMMON_UTILS_H_
