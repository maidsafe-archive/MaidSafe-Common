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

#include "maidsafe/common/identity.h"

#include <algorithm>
#include <bitset>
#include <sstream>
#include <string>
#include <vector>

#include "maidsafe/common/encode.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace test {

template <typename T>
class IdentityFactoryTest : public testing::Test {
 protected:
  IdentityFactoryTest()
      : random_bytes_(RandomBytes(identity_size)),
        known_bytes_(identity_size, 0),
        random_encoded_(Encode(random_bytes_)),
        known_encoded_(),
        bad_encoded_("Bad Encoded") {
    for (char c = 0; c < static_cast<char>(identity_size); ++c)
      known_bytes_[static_cast<std::size_t>(c)] = c;
    known_encoded_ = Encode(known_bytes_);
  }

  template <typename Input>
  T Encode(const Input& input) const;

  T KnownEncoded() const;

  std::vector<byte> random_bytes_, known_bytes_;
  T random_encoded_, known_encoded_, bad_encoded_;
};

template <>
template <>
binary::String IdentityFactoryTest<binary::String>::Encode<std::vector<byte>>(
    const std::vector<byte>& input) const {
  std::string hex_encoded(hex::Encode(input));
  std::string result;
  for (auto& elem : hex_encoded) {
    std::string temp;
    switch (elem) {
      case '0':
        temp = "0000";
        break;
      case '1':
        temp = "0001";
        break;
      case '2':
        temp = "0010";
        break;
      case '3':
        temp = "0011";
        break;
      case '4':
        temp = "0100";
        break;
      case '5':
        temp = "0101";
        break;
      case '6':
        temp = "0110";
        break;
      case '7':
        temp = "0111";
        break;
      case '8':
        temp = "1000";
        break;
      case '9':
        temp = "1001";
        break;
      case 'a':
        temp = "1010";
        break;
      case 'b':
        temp = "1011";
        break;
      case 'c':
        temp = "1100";
        break;
      case 'd':
        temp = "1101";
        break;
      case 'e':
        temp = "1110";
        break;
      case 'f':
        temp = "1111";
        break;
      default:
        LOG(kError) << "Invalid hex format";
    }
    result += temp;
  }
  return binary::String(std::move(result));
}

template <>
template <>
binary::String IdentityFactoryTest<binary::String>::Encode<Identity>(const Identity& input) const {
  return binary::String(binary::Encode(input));
}

template <>
template <typename Input>
hex::String IdentityFactoryTest<hex::String>::Encode(const Input& input) const {
  return hex::String(hex::Encode(input));
}

template <>
template <typename Input>
base64::String IdentityFactoryTest<base64::String>::Encode(const Input& input) const {
  return base64::String(base64::Encode(input));
}

template <>
binary::String IdentityFactoryTest<binary::String>::KnownEncoded() const {
  return binary::String(
      "00000000000000010000001000000011000001000000010100000110000001110000100000001001000010100000"
      "10110000110000001101000011100000111100010000000100010001001000010011000101000001010100010110"
      "00010111000110000001100100011010000110110001110000011101000111100001111100100000001000010010"
      "00100010001100100100001001010010011000100111001010000010100100101010001010110010110000101101"
      "00101110001011110011000000110001001100100011001100110100001101010011011000110111001110000011"
      "1001001110100011101100111100001111010011111000111111");
}

template <>
hex::String IdentityFactoryTest<hex::String>::KnownEncoded() const {
  return hex::String(
      "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d"
      "2e2f303132333435363738393a3b3c3d3e3f");
}

template <>
base64::String IdentityFactoryTest<base64::String>::KnownEncoded() const {
  return base64::String(
      "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+Pw==");
}

using IdentityFactoryTestTypes = testing::Types<binary::String, hex::String, base64::String>;
TYPED_TEST_CASE(IdentityFactoryTest, IdentityFactoryTestTypes);

TYPED_TEST(IdentityFactoryTest, BEH_FromEncodedString) {
  EXPECT_THROW(MakeIdentity(this->bad_encoded_), common_error);

  Identity random_id(MakeIdentity(this->random_encoded_));
  EXPECT_EQ(this->random_bytes_, random_id.string());
  EXPECT_EQ(this->random_encoded_, this->Encode(random_id));

  Identity known_id(MakeIdentity(this->known_encoded_));
  EXPECT_EQ(this->known_bytes_, known_id.string());
  EXPECT_EQ(this->known_encoded_, this->Encode(known_id));
  EXPECT_EQ(this->KnownEncoded(), this->Encode(known_id));
}

class IdentityTest : public testing::Test {
 protected:
  IdentityTest()
      : max_id_(std::string(identity_size, -1)),
        id1_([this]() -> Identity {
          auto id(MakeIdentity());
          while (id == max_id_)
            id = MakeIdentity();
          return id;
        }()),
        id2_([this]() -> Identity {
          auto id(MakeIdentity());
          while (id == max_id_ || id == id1_)
            id = MakeIdentity();
          return id;
        }()),
        invalid_id_() {}
  const Identity max_id_, id1_, id2_, invalid_id_;
};

TEST_F(IdentityTest, BEH_CloserToTarget) {
  Identity target(MakeIdentity());
  while (target == id1_ || target == id2_)
    target = MakeIdentity();

  Identity xor_distance1 = id1_ ^ target;
  Identity xor_distance2 = id2_ ^ target;

  if (xor_distance1 < xor_distance2) {
    EXPECT_TRUE(CloserToTarget(id1_, id2_, target));
    EXPECT_FALSE(CloserToTarget(id2_, id1_, target));
  } else {
    EXPECT_FALSE(CloserToTarget(id1_, id2_, target));
    EXPECT_TRUE(CloserToTarget(id2_, id1_, target));
  }

  EXPECT_TRUE(CloserToTarget(target, id1_, target));
  EXPECT_FALSE(CloserToTarget(id1_, target, target));
  EXPECT_FALSE(CloserToTarget(id1_, id1_, target));

  EXPECT_THROW(CloserToTarget(invalid_id_, id1_, target), common_error);
  EXPECT_THROW(CloserToTarget(id1_, invalid_id_, target), common_error);
  EXPECT_THROW(CloserToTarget(id1_, id2_, invalid_id_), common_error);
}

TEST_F(IdentityTest, BEH_CommonLeadingBits) {
  // Check for two equal IDs
  Identity copy_of_id1(id1_);
  EXPECT_EQ(identity_size * 8, CommonLeadingBits(id1_, copy_of_id1));

  // Iterate through a copy of ID starting at the least significant bit, flipping a bit each time,
  // checking the function, then flipping the bit back.
  std::bitset<identity_size * 8> id1_as_binary(binary::Encode(id1_));
  for (size_t i(0); i != id1_as_binary.size(); ++i) {
    id1_as_binary.flip(i);
    Identity modified_id(MakeIdentity(binary::String(id1_as_binary.to_string())));
    EXPECT_EQ((identity_size * 8) - i - 1, CommonLeadingBits(id1_, modified_id));
    id1_as_binary.flip(i);
  }

  EXPECT_THROW(CommonLeadingBits(invalid_id_, id1_), common_error);
  EXPECT_THROW(CommonLeadingBits(id1_, invalid_id_), common_error);
}

TEST_F(IdentityTest, BEH_Serialisation) {
  // Valid Serialisation
  SerialisedData serialised(Serialise(id1_));

  // Valid Deserialisation
  Identity parsed(Parse<Identity>(serialised));
  EXPECT_EQ(id1_, parsed);

  // Invalid Serialisation/deserialisation
  EXPECT_THROW(Serialise(invalid_id_), common_error);

  serialised.pop_back();
  EXPECT_THROW(Parse<Identity>(serialised), common_error);
}

}  // namespace test

}  // namespace maidsafe
