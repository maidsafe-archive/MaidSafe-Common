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

#include "maidsafe/common/encode.h"

#include "maidsafe/common/error.h"

namespace maidsafe {

namespace detail {

std::string GetSubstr(const std::string& input) {
  if (input.size() > 14)
    return (input.substr(0, 6) + ".." + input.substr(input.size() - 6));
  else
    return input;
}

}  // namespace detail



namespace hex {

namespace {

template <typename T>
T Decode(const std::string& hex_input) {
  auto size(hex_input.size());
  if (size % 2)  // Sanity check
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_conversion));

  T non_hex_output(size / 2, 0);
  for (size_t i(0), j(0); i != size / 2; ++i) {
    non_hex_output[i] = (detail::lookup[static_cast<int>(hex_input[j++])] << 4);
    non_hex_output[i] |= detail::lookup[static_cast<int>(hex_input[j++])];
  }
  return non_hex_output;
}

}  // unnamed namespace

std::string DecodeToString(const std::string& hex_input) { return Decode<std::string>(hex_input); }

std::vector<unsigned char> DecodeToBytes(const std::string& hex_input) {
  return Decode<std::vector<unsigned char>>(hex_input);
}

}  // namespace hex



namespace base64 {

namespace {

template <typename T>
T Decode(const std::string& base64_input) {
  if (base64_input.size() % 4)  // Sanity check
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_conversion));

  std::size_t padding = 0;
  if (base64_input.size()) {
    if (base64_input[base64_input.size() - 1] == detail::pad_character)
      ++padding;
    if (base64_input[base64_input.size() - 2] == detail::pad_character)
      ++padding;
  }

  // Setup a vector to hold the result
  T decoded_bytes;
  decoded_bytes.reserve(((base64_input.size() / 4) * 3) - padding);
  std::uint32_t temp = 0;  // Holds decoded quanta
  auto cursor = base64_input.begin();
  while (cursor < base64_input.end()) {
    for (std::size_t quantum_position = 0; quantum_position < 4; ++quantum_position) {
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
        temp |= 0x3F;  // change to 0x5F for URL alphabet
      else if (*cursor == detail::pad_character) {
        // pad
        switch (base64_input.end() - cursor) {
          case 1:  // One pad character
            decoded_bytes.push_back((temp >> 16) & 0x000000FF);
            decoded_bytes.push_back((temp >> 8) & 0x000000FF);
            return decoded_bytes;
          case 2:  // Two pad characters
            decoded_bytes.push_back((temp >> 10) & 0x000000FF);
            return decoded_bytes;
          default:
            BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_conversion));
        }
      } else {
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_conversion));
      }
      ++cursor;
    }
    decoded_bytes.push_back((temp >> 16) & 0x000000FF);
    decoded_bytes.push_back((temp >> 8) & 0x000000FF);
    decoded_bytes.push_back(temp & 0x000000FF);
  }
  return decoded_bytes;
}

}  // unnamed namespace

std::string DecodeToString(const std::string& base64_input) {
  return Decode<std::string>(base64_input);
}

std::vector<unsigned char> DecodeToBytes(const std::string& base64_input) {
  return Decode<std::vector<unsigned char>>(base64_input);
}

}  // namespace base64

}  // namespace maidsafe
