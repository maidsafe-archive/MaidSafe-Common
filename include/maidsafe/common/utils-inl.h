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

#ifndef MAIDSAFE_COMMON_UTILS_INL_H_
#define MAIDSAFE_COMMON_UTILS_INL_H_

#include <string>
#include <type_traits>

#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/variate_generator.hpp"

namespace maidsafe {

template<typename StringType>
StringType RandomSafeString(const size_t& length) {
  static boost::mt19937 random_number_generator(static_cast<unsigned int>(
                boost::posix_time::microsec_clock::universal_time().time_of_day().
                total_microseconds()));
  static std::mutex random_number_generator_mutex;
  boost::uniform_int<> uniform_distribution(0, 255);
  StringType random_string(length, 0);
  {
    std::lock_guard<std::mutex> lock(random_number_generator_mutex);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        random_number_generator, uniform_distribution);
    std::generate(random_string.begin(), random_string.end(), uni);
  }
  return random_string;
}

template<typename StringType>
StringType RandomAlphaNumericSafeString(const size_t& length) {
  static boost::mt19937 random_number_generator(static_cast<unsigned int>(
                boost::posix_time::microsec_clock::universal_time().time_of_day().
                total_microseconds()));
  static std::mutex random_number_generator_mutex;
  static const char alpha_numerics[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  boost::uniform_int<> uniform_distribution(0, 61);
  StringType random_string(length, 0);
  {
    std::lock_guard<std::mutex> lock(random_number_generator_mutex);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uni(
        random_number_generator, uniform_distribution);
    for (auto it = random_string.begin(); it != random_string.end(); ++it)
      *it = alpha_numerics[uni()];
  }
  return random_string;
}

template<typename StringType>
StringType EncodeStringToHex(const StringType& non_hex_input) {
  static const char kHexAlphabet[] = "0123456789abcdef";
  auto size(non_hex_input.size());
  StringType hex_output(size * 2, 0);
  for (size_t i(0), j(0); i != size; ++i) {
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) / 16];
    hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(non_hex_input[i]) % 16];
  }
  return hex_output;
}

template<size_t min, size_t max, typename StringType>
StringType EncodeStringToHex(const detail::BoundedString<min, max, StringType> &non_hex_input) {
  return EncodeToHex(non_hex_input.string());
}

template<typename StringType>
StringType HexStringSubstr(const StringType& non_hex) {
  static const char kHexAlphabet[] = "0123456789abcdef";
  size_t non_hex_size(non_hex.size());
  if (non_hex_size < 7)
    return EncodeStringToHex(non_hex);

  StringType hex(14, 0);
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

template<size_t min, size_t max, typename StringType>
StringType HexStringSubstr(const detail::BoundedString<min, max, StringType> &non_hex) {
  return HexStringSubstr(non_hex.string());
}

template<typename Rep, typename Period>
void Sleep(const std::chrono::duration<Rep, Period>& duration) {
  std::this_thread::sleep_for(duration);
}


}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_UTILS_INL_H_
