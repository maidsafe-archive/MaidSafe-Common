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

#ifndef MAIDSAFE_COMMON_UTILS_INL_H_
#define MAIDSAFE_COMMON_UTILS_INL_H_

#include <string>

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

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_UTILS_INL_H_
