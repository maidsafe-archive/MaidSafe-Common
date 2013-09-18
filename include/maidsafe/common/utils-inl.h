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
