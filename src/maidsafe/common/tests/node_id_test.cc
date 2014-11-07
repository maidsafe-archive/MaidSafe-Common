/*  Copyright 2008 MaidSafe.net limited

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

#include "maidsafe/common/node_id.h"

#include <algorithm>
#include <bitset>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

size_t BitToByteCount(size_t bit_count) {
  return static_cast<size_t>(0.999999 + static_cast<double>(bit_count) / 8);
}

TEST(NodeIdTest, BEH_BitToByteCount) {
  for (size_t i = 0; i < NodeId::kSize; ++i) {
    EXPECT_EQ(i, BitToByteCount(8 * i));
    for (size_t j = 1; j < 8; ++j)
      ASSERT_EQ((i + 1), BitToByteCount((8 * i) + j));
  }
}

NodeId IncreaseId(const NodeId& kad_id) {
  std::string raw(kad_id.string());
  std::string::reverse_iterator rit = raw.rbegin();
  while (rit != raw.rend()) {
    if (++(*rit) == 0)
      ++rit;
    else
      break;
  }
  return NodeId(raw);
}

NodeId MaxNodeId() { return NodeId(std::string(NodeId::kSize, -1)); }

const std::string ToBinary(const std::string& raw_id) {
  std::string hex_encoded(HexEncode(raw_id));
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
  return result;
}

TEST(NodeIdTest, BEH_DefaultConstructor) {
  auto id = NodeId{};
  EXPECT_FALSE(id.IsValid());
}

TEST(NodeIdTest, BEH_RandomIdConstructor) {
  auto id = NodeId{RandomIdTag{}};
  EXPECT_TRUE(id.IsValid());

  auto node_ids = std::set<NodeId>{};
  auto success = bool{true};
  for (int i(0); i < 100000; ++i)
    success &= node_ids.emplace(RandomIdTag{}).second;
  ASSERT_TRUE(success);
}

TEST(NodeIdTest, BEH_CopyAndMove) {
  const auto id = NodeId{RandomIdTag{}};

  // Copy c'tor
  auto copy1 = NodeId{id};
  EXPECT_EQ(id, copy1);

  // Copy assignment
  auto copy2 = NodeId{};
  copy2 = id;
  EXPECT_EQ(id, copy2);

  // Move c'tor
  auto move1 = NodeId{std::move(copy1)};
  EXPECT_EQ(id, move1);

  // Move assignment
  auto move2 = NodeId{};
  move2 = std::move(copy2);
  EXPECT_EQ(id, move2);
}

TEST(NodeIdTest, BEH_StringConstructor) {
  const auto rand_str = RandomString(NodeId::kSize);
  auto id = NodeId{rand_str};
  EXPECT_EQ(id.string(), rand_str);
  EXPECT_THROW(NodeId{rand_str.substr(0, NodeId::kSize - 1)}, maidsafe_error);
}

TEST(NodeIdTest, BEH_HashConstructor) {
  const auto hash = crypto::Hash<crypto::SHA512>(RandomString(10));
  auto id = NodeId{hash};
  EXPECT_EQ(id.string(), hash.string());
}

TEST(NodeIdTest, BEH_EncodingConstructor) {
  auto known_raw = std::string(NodeId::kSize, 0);
  for (char c = 0; c < NodeId::kSize; ++c)
    known_raw.at(static_cast<uint8_t>(c)) = c;
  for (int i = 0; i < 3; ++i) {
    auto rand_str = RandomString(NodeId::kSize);
    std::string bad_encoded("Bad Encoded"), encoded, known_encoded;
    NodeId::EncodingType type = static_cast<NodeId::EncodingType>(i);
    switch (type) {
      case NodeId::EncodingType::kBinary:
        encoded = ToBinary(rand_str);
        known_encoded = ToBinary(known_raw);
        break;
      case NodeId::EncodingType::kHex:
        encoded = HexEncode(rand_str);
        known_encoded = HexEncode(known_raw);
        break;
      case NodeId::EncodingType::kBase64:
        encoded = Base64Encode(rand_str);
        known_encoded = Base64Encode(known_raw);
        break;
      default:
        break;
    }

    EXPECT_THROW(NodeId(bad_encoded, type), common_error);
    auto rand_id = NodeId{encoded, type};
    EXPECT_EQ(rand_str, rand_id.string());
    EXPECT_EQ(encoded, rand_id.ToStringEncoded(type));
    auto known_id = NodeId{known_encoded, type};
    EXPECT_EQ(known_raw, known_id.string());
    EXPECT_EQ(known_encoded, known_id.ToStringEncoded(type));
    switch (i) {
      case static_cast<int>(NodeId::EncodingType::kBinary):
        EXPECT_EQ(
            known_encoded,
            "00000000000000010000001000000011000001000000010100000110000001110000100000001001000010"
            "10000010110000110000001101000011100000111100010000000100010001001000010011000101000001"
            "01010001011000010111000110000001100100011010000110110001110000011101000111100001111100"
            "10000000100001001000100010001100100100001001010010011000100111001010000010100100101010"
            "00101011001011000010110100101110001011110011000000110001001100100011001100110100001101"
            "0100110110001101110011100000111001001110100011101100111100001111010011111000111111");
        break;
      case static_cast<int>(NodeId::EncodingType::kHex):
        EXPECT_EQ(
            known_encoded,
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a"
            "2b2c2d2e2f303132333435363738393a3b3c3d3e3f");
        break;
      case static_cast<int>(NodeId::EncodingType::kBase64):
        EXPECT_EQ(
            known_encoded,
            "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+Pw"
            "==");
        break;
      default:
        break;
    }
  }
}

TEST(NodeIdTest, BEH_Operators) {
  const auto max_id = MaxNodeId();

  auto id1 = NodeId{ RandomIdTag{} };
  while (id1 == max_id)
    id1 = NodeId{ RandomIdTag{} };
  const auto id2 = NodeId{ id1 };
  const auto invalid_id1 = NodeId{};
  const auto invalid_id2 = NodeId{};

  // operator==
  EXPECT_TRUE(id1 == id2);
  EXPECT_TRUE(invalid_id1 == invalid_id2);

  EXPECT_FALSE(id1 == max_id);
  EXPECT_FALSE(max_id == id1);

  EXPECT_FALSE(id1 == invalid_id1);
  EXPECT_FALSE(invalid_id1 == id1);

  // operator!=
  EXPECT_FALSE(id1 != id2);
  EXPECT_FALSE(invalid_id1 != invalid_id2);

  EXPECT_TRUE(id1 != max_id);
  EXPECT_TRUE(max_id != id1);

  EXPECT_TRUE(id1 != invalid_id1);
  EXPECT_TRUE(invalid_id1 != id1);

  // operator<
  EXPECT_FALSE(id1 < id2);
  EXPECT_FALSE(invalid_id1 < invalid_id2);

  EXPECT_TRUE(id1 < max_id);
  EXPECT_FALSE(max_id < id1);

  EXPECT_FALSE(id1 < invalid_id1);
  EXPECT_TRUE(invalid_id1 < id1);

  // operator>
  EXPECT_FALSE(id1 > id2);
  EXPECT_FALSE(invalid_id1 > invalid_id2);

  EXPECT_FALSE(id1 > max_id);
  EXPECT_TRUE(max_id > id1);

  EXPECT_TRUE(id1 > invalid_id1);
  EXPECT_FALSE(invalid_id1 > id1);

  // operator<=
  EXPECT_TRUE(id1 <= id2);
  EXPECT_TRUE(invalid_id1 <= invalid_id2);

  EXPECT_TRUE(id1 <= max_id);
  EXPECT_FALSE(max_id <= id1);

  EXPECT_FALSE(id1 <= invalid_id1);
  EXPECT_TRUE(invalid_id1 <= id1);

  // operator>=
  EXPECT_TRUE(id1 >= id2);
  EXPECT_TRUE(invalid_id1 >= invalid_id2);

  EXPECT_FALSE(id1 >= max_id);
  EXPECT_TRUE(max_id >= id1);

  EXPECT_TRUE(id1 >= invalid_id1);
  EXPECT_FALSE(invalid_id1 >= id1);

  // operator^

}


/*

TEST(NodeIdTest, BEH_OperatorXOR) {
  NodeId kadid1(NodeId::IdType::kRandomId), kadid2(NodeId::IdType::kRandomId);
  NodeId kadid3(kadid1 ^ kadid2);
  std::string binid1(kadid1.ToStringEncoded(NodeId::EncodingType::kBinary));
  std::string binid2(kadid2.ToStringEncoded(NodeId::EncodingType::kBinary));
  std::string binresult;
  for (size_t i = 0; i < binid1.size(); ++i) {
    if (binid1[i] == binid2[i]) {
      binresult += "0";
    } else {
      binresult += "1";
    }
  }
  std::string binzero;
  while (binzero.size() < binid1.size())
    binzero += "0";
  ASSERT_NE(binzero, kadid3.ToStringEncoded(NodeId::EncodingType::kBinary));
  ASSERT_EQ(binresult, kadid3.ToStringEncoded(NodeId::EncodingType::kBinary));
  NodeId kadid4(kadid2 ^ kadid1);
  ASSERT_EQ(binresult, kadid4.ToStringEncoded(NodeId::EncodingType::kBinary));
  NodeId kadid5(kadid1.string());
  NodeId kadid6(kadid1 ^ kadid5);
  ASSERT_EQ(binzero, kadid6.ToStringEncoded(NodeId::EncodingType::kBinary));
  std::string zero(kadid6.string());
  ASSERT_EQ(BitToByteCount(NodeId::kSize * 8), zero.size());
  for (size_t i = 0; i < zero.size(); ++i)
    ASSERT_EQ('\0', zero[i]);
}

TEST(NodeIdTest, BEH_CommonLeadingBits) {
  const NodeId kThisNode{ NodeId::IdType::kRandomId };

  // Check for two equal IDs
  NodeId other{ kThisNode };
  EXPECT_EQ(NodeId::kSize * 8, kThisNode.CommonLeadingBits(other));

  // Iterate through a copy of ID starting at the least significant bit, flipping a bit each time,
  // checking the function, then flipping the bit back.
  std::bitset<NodeId::kSize * 8> id_as_binary{
      kThisNode.ToStringEncoded(NodeId::EncodingType::kBinary) };
  for (size_t i(0); i != id_as_binary.size(); ++i) {
    id_as_binary.flip(i);
    NodeId modified_id{ id_as_binary.to_string(), NodeId::EncodingType::kBinary };
    EXPECT_EQ((NodeId::kSize * 8) - i - 1, kThisNode.CommonLeadingBits(modified_id));
    id_as_binary.flip(i);
  }
}

still to test:
- CloserToTarget
- string
- IsValid
- DebugId
- swap
- operator<<

*/

}  // namespace test

}  // namespace maidsafe
