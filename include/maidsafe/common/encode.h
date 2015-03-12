/*  Copyright 2015 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_ENCODE_H_
#define MAIDSAFE_COMMON_ENCODE_H_

#include <cstdint>
#include <string>
#include <vector>

namespace maidsafe {

namespace detail {

std::string GetSubstr(const std::string& input);

template <std::size_t min, std::size_t max, typename String>
class BoundedString;

}  // namespace detail

namespace hex {

namespace detail {

const unsigned char alphabet[] = "0123456789abcdef";

const unsigned char lookup[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,  8,  9,  0,  0,  0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

}  // namespace detail

template <typename T>
std::string Encode(const T& non_hex_input) {
  auto size(non_hex_input.size());
  std::string hex_output(size * 2, 0);
  for (std::size_t i(0), j(0); i != size; ++i) {
    hex_output[j++] = detail::alphabet[static_cast<unsigned char>(non_hex_input[i]) / 16];
    hex_output[j++] = detail::alphabet[static_cast<unsigned char>(non_hex_input[i]) % 16];
  }
  return hex_output;
}

// Implemented in bounded_string.h
template <std::size_t min, std::size_t max, typename String>
std::string Encode(const maidsafe::detail::BoundedString<min, max, String>& non_hex_input);

std::string DecodeToString(const std::string& hex_input);

std::vector<unsigned char> DecodeToBytes(const std::string& hex_input);

// Returns an abbreviated hex representation of 'non_hex_input'.
template <typename T>
std::string Substr(const T& non_hex_input) {
  return maidsafe::detail::GetSubstr(Encode(non_hex_input));
}

}  // namespace hex



namespace base64 {

namespace detail {

const unsigned char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const unsigned char pad_character('=');

}  // namespace detail

// hacked from https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
template <typename T>
std::string Encode(const T& non_base64_input) {
  std::string encoded_string(
      ((non_base64_input.size() / 3) + (non_base64_input.size() % 3 > 0)) * 4, 0);
  std::int32_t temp;
  auto cursor = non_base64_input.begin();
  std::size_t i = 0;
  std::size_t common_output_size((non_base64_input.size() / 3) * 4);
  while (i < common_output_size) {
    temp = static_cast<std::uint8_t>(*cursor++) << 16;  // Convert to big endian
    temp += static_cast<std::uint8_t>(*cursor++) << 8;
    temp += static_cast<std::uint8_t>(*cursor++);
    encoded_string[i++] = detail::alphabet[(temp & 0x00FC0000) >> 18];
    encoded_string[i++] = detail::alphabet[(temp & 0x0003F000) >> 12];
    encoded_string[i++] = detail::alphabet[(temp & 0x00000FC0) >> 6];
    encoded_string[i++] = detail::alphabet[(temp & 0x0000003F)];
  }
  switch (non_base64_input.size() % 3) {
    case 1:
      temp = static_cast<std::uint8_t>(*cursor++) << 16;  // Convert to big endian
      encoded_string[i++] = detail::alphabet[(temp & 0x00FC0000) >> 18];
      encoded_string[i++] = detail::alphabet[(temp & 0x0003F000) >> 12];
      encoded_string[i++] = detail::pad_character;
      encoded_string[i++] = detail::pad_character;
      break;
    case 2:
      temp = static_cast<std::uint8_t>(*cursor++) << 16;  // Convert to big endian
      temp += static_cast<std::uint8_t>(*cursor++) << 8;
      encoded_string[i++] = detail::alphabet[(temp & 0x00FC0000) >> 18];
      encoded_string[i++] = detail::alphabet[(temp & 0x0003F000) >> 12];
      encoded_string[i++] = detail::alphabet[(temp & 0x00000FC0) >> 6];
      encoded_string[i++] = detail::pad_character;
      break;
  }
  return encoded_string;
}

// Implemented in bounded_string.h
template <std::size_t min, std::size_t max, typename String>
std::string Encode(const maidsafe::detail::BoundedString<min, max, String>& non_base64_input);

std::string DecodeToString(const std::string& base64_input);

std::vector<unsigned char> DecodeToBytes(const std::string& base64_input);

// Returns an abbreviated base-64 representation of 'non_base64_input'.
template <typename T>
std::string Substr(const T& non_base64_input) {
  return maidsafe::detail::GetSubstr(Encode(non_base64_input));
}

}  // namespace base64

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ENCODE_H_
