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
#include <sstream>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

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

TEST(NodeIdBasicTest, BEH_DefaultConstructor) {
  auto id = NodeId{};
  EXPECT_FALSE(id.IsValid());
}

TEST(NodeIdBasicTest, BEH_CopyAndMove) {
  const auto id = NodeId{RandomString(NodeId::kSize)};

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

TEST(NodeIdBasicTest, BEH_StringConstructor) {
  const auto rand_str = RandomString(NodeId::kSize);
  auto id = NodeId{rand_str};
  EXPECT_EQ(id.string(), rand_str);
  EXPECT_THROW(NodeId{rand_str.substr(0, NodeId::kSize - 1)}, common_error);
}

TEST(NodeIdBasicTest, BEH_HashConstructor) {
  const auto hash = crypto::Hash<crypto::SHA512>(RandomString(10));
  auto id = NodeId{hash};
  EXPECT_EQ(id.string(), hash.string());
}

TEST(NodeIdBasicTest, BEH_EncodingConstructor) {
  auto known_raw = std::string(NodeId::kSize, 0);
  for (char c = 0; c < static_cast<char>(NodeId::kSize); ++c)
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
        EXPECT_EQ(known_encoded,
                  "00000000000000010000001000000011000001000000010100000110000001110000100000001001"
                  "00001010000010110000110000001101000011100000111100010000000100010001001000010011"
                  "00010100000101010001011000010111000110000001100100011010000110110001110000011101"
                  "00011110000111110010000000100001001000100010001100100100001001010010011000100111"
                  "00101000001010010010101000101011001011000010110100101110001011110011000000110001"
                  "00110010001100110011010000110101001101100011011100111000001110010011101000111011"
                  "00111100001111010011111000111111");
        break;
      case static_cast<int>(NodeId::EncodingType::kHex):
        EXPECT_EQ(known_encoded,
                  "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f2021222324252627"
                  "28292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f");
        break;
      case static_cast<int>(NodeId::EncodingType::kBase64):
        EXPECT_EQ(known_encoded,
                  "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7"
                  "PD0+Pw==");
        break;
      default:
        break;
    }
  }
}

TEST(NodeIdBasicTest, BEH_String) {
  const auto rand_str = RandomString(NodeId::kSize);
  auto id = NodeId{rand_str};
  EXPECT_EQ(id.string(), rand_str);
  EXPECT_THROW(NodeId{}.string(), common_error);
}

TEST(NodeIdBasicTest, BEH_IsValid) {
  EXPECT_TRUE(NodeId{RandomString(NodeId::kSize)}.IsValid());
  EXPECT_FALSE(NodeId{}.IsValid());
}

class NodeIdTest : public testing::Test {
 protected:
  NodeIdTest()
      : max_id_(std::string(NodeId::kSize, -1)),
        id1_([this]() -> NodeId {
          auto id = NodeId{RandomString(NodeId::kSize)};
          while (id == max_id_)
            id = NodeId{RandomString(NodeId::kSize)};
          return id;
        }()),
        id2_([this]() -> NodeId {
          auto id = NodeId{RandomString(NodeId::kSize)};
          while (id == max_id_ || id == id1_)
            id = NodeId{RandomString(NodeId::kSize)};
          return id;
        }()),
        invalid_id_() {}
  const NodeId max_id_, id1_, id2_, invalid_id_;
};

TEST_F(NodeIdTest, BEH_Operators) {
  const auto copy_of_id1 = NodeId{id1_};

  // operator==
  EXPECT_TRUE(id1_ == copy_of_id1);
  EXPECT_TRUE(invalid_id_ == NodeId{});

  EXPECT_FALSE(id1_ == max_id_);
  EXPECT_FALSE(max_id_ == id1_);

  EXPECT_FALSE(id1_ == invalid_id_);
  EXPECT_FALSE(invalid_id_ == id1_);

  // operator!=
  EXPECT_FALSE(id1_ != copy_of_id1);
  EXPECT_FALSE(invalid_id_ != NodeId{});

  EXPECT_TRUE(id1_ != max_id_);
  EXPECT_TRUE(max_id_ != id1_);

  EXPECT_TRUE(id1_ != invalid_id_);
  EXPECT_TRUE(invalid_id_ != id1_);

  // operator<
  EXPECT_FALSE(id1_ < copy_of_id1);
  EXPECT_FALSE(invalid_id_ < NodeId{});

  EXPECT_TRUE(id1_ < max_id_);
  EXPECT_FALSE(max_id_ < id1_);

  EXPECT_FALSE(id1_ < invalid_id_);
  EXPECT_TRUE(invalid_id_ < id1_);

  // operator>
  EXPECT_FALSE(id1_ > copy_of_id1);
  EXPECT_FALSE(invalid_id_ > NodeId{});

  EXPECT_FALSE(id1_ > max_id_);
  EXPECT_TRUE(max_id_ > id1_);

  EXPECT_TRUE(id1_ > invalid_id_);
  EXPECT_FALSE(invalid_id_ > id1_);

  // operator<=
  EXPECT_TRUE(id1_ <= copy_of_id1);
  EXPECT_TRUE(invalid_id_ <= NodeId{});

  EXPECT_TRUE(id1_ <= max_id_);
  EXPECT_FALSE(max_id_ <= id1_);

  EXPECT_FALSE(id1_ <= invalid_id_);
  EXPECT_TRUE(invalid_id_ <= id1_);

  // operator>=
  EXPECT_TRUE(id1_ >= copy_of_id1);
  EXPECT_TRUE(invalid_id_ >= NodeId{});

  EXPECT_FALSE(id1_ >= max_id_);
  EXPECT_TRUE(max_id_ >= id1_);

  EXPECT_TRUE(id1_ >= invalid_id_);
  EXPECT_FALSE(invalid_id_ >= id1_);

  // operator^
  const auto xor_of_id1_and_id2 = id1_ ^ id2_;

  auto binary_id1 = id1_.ToStringEncoded(NodeId::EncodingType::kBinary);
  auto binary_id2 = id2_.ToStringEncoded(NodeId::EncodingType::kBinary);
  auto binary_xor = xor_of_id1_and_id2.ToStringEncoded(NodeId::EncodingType::kBinary);
  auto itr1 = std::begin(binary_id1);
  auto itr2 = std::begin(binary_id2);
  auto itr_xor = std::begin(binary_xor);
  while (itr1 != std::end(binary_id1)) {
    if (*itr1++ == *itr2++)
      EXPECT_EQ('0', *itr_xor++);
    else
      EXPECT_EQ('1', *itr_xor++);
  }

  EXPECT_EQ(xor_of_id1_and_id2, id2_ ^ id1_);
  EXPECT_EQ(NodeId(std::string(NodeId::kSize, 0)), id1_ ^ id1_);

  EXPECT_THROW(id1_ ^ invalid_id_, common_error);
  EXPECT_THROW(invalid_id_ ^ id1_, common_error);
  EXPECT_THROW(invalid_id_ ^ invalid_id_, common_error);

  // operator<<
  std::stringstream sstream;
  sstream << id1_ << invalid_id_;
  EXPECT_EQ(DebugId(id1_) + "Invalid ID", sstream.str());
}

TEST_F(NodeIdTest, BEH_CloserToTarget) {
  auto target = NodeId{RandomString(NodeId::kSize)};
  while (target == id1_ || target == id2_)
    target = NodeId{RandomString(NodeId::kSize)};

  auto xor_distance1 = id1_ ^ target;
  auto xor_distance2 = id2_ ^ target;

  if (xor_distance1 < xor_distance2) {
    EXPECT_TRUE(NodeId::CloserToTarget(id1_, id2_, target));
    EXPECT_FALSE(NodeId::CloserToTarget(id2_, id1_, target));
  } else {
    EXPECT_FALSE(NodeId::CloserToTarget(id1_, id2_, target));
    EXPECT_TRUE(NodeId::CloserToTarget(id2_, id1_, target));
  }

  EXPECT_TRUE(NodeId::CloserToTarget(target, id1_, target));
  EXPECT_FALSE(NodeId::CloserToTarget(id1_, target, target));
  EXPECT_FALSE(NodeId::CloserToTarget(id1_, id1_, target));

  EXPECT_THROW(NodeId::CloserToTarget(invalid_id_, id1_, target), common_error);
  EXPECT_THROW(NodeId::CloserToTarget(id1_, invalid_id_, target), common_error);
  EXPECT_THROW(NodeId::CloserToTarget(id1_, id2_, invalid_id_), common_error);
}
TEST_F(NodeIdTest, FUNC_CloserToTarget) {
  auto target = NodeId{RandomString(NodeId::kSize)};
  std::vector<NodeId> nodes(100000, NodeId(RandomString(NodeId::kSize)));
  std::sort(std::begin(nodes), std::end(nodes), [target](const NodeId& lhs, const NodeId& rhs) {
    return NodeId::CloserToTarget(lhs, rhs, target);
  });

  auto closest = nodes.front();

  for (const auto& node : nodes) {
    // not to worry cpmparing same nodes will not make one closer
    EXPECT_FALSE(NodeId::CloserToTarget(node, closest, target));
  }
}
TEST_F(NodeIdTest, BEH_CommonLeadingBits) {
  // Check for two equal IDs
  auto copy_of_id1 = NodeId{id1_};
  EXPECT_EQ(NodeId::kSize * 8, id1_.CommonLeadingBits(copy_of_id1));

  // Iterate through a copy of ID starting at the least significant bit,
  // flipping a bit each time,
  // checking the function, then flipping the bit back.
  std::bitset<NodeId::kSize * 8> id1_as_binary{id1_.ToStringEncoded(NodeId::EncodingType::kBinary)};
  for (size_t i(0); i != id1_as_binary.size(); ++i) {
    id1_as_binary.flip(i);
    auto modified_id = NodeId{id1_as_binary.to_string(), NodeId::EncodingType::kBinary};
    EXPECT_EQ((NodeId::kSize * 8) - i - 1, id1_.CommonLeadingBits(modified_id));
    id1_as_binary.flip(i);
  }
}

TEST_F(NodeIdTest, BEH_DebugId) {
  EXPECT_EQ("Invalid ID", DebugId(invalid_id_));
  EXPECT_EQ(HexSubstr(id1_.string()), DebugId(id1_));

  // Test after calling each non-const function that DebugId is still valid.
  auto id1 = id1_;
  EXPECT_EQ(HexSubstr(id1.string()), DebugId(id1));

  auto id2 = std::move(id1);
  EXPECT_EQ(HexSubstr(id2.string()), DebugId(id2));

  id2 = id2_;
  EXPECT_EQ(HexSubstr(id2_.string()), DebugId(id2));

  id1 = std::move(id2);
  EXPECT_EQ(HexSubstr(id2_.string()), DebugId(id1));

  id1 = id1_;
  id2 = id2_;
  swap(id1, id2);
  EXPECT_EQ(HexSubstr(id1.string()), DebugId(id1));
  EXPECT_EQ(HexSubstr(id2.string()), DebugId(id2));

  id1 ^= id2;
  EXPECT_EQ(HexSubstr(id1.string()), DebugId(id1));
}

TEST_F(NodeIdTest, BEH_Swap) {
  auto id1 = id1_;
  auto id2 = id2_;
  swap(id1, id2);
  EXPECT_EQ(id1_, id2);
  EXPECT_EQ(id2_, id1);

  auto invalid_id = NodeId{};
  swap(invalid_id, id1);
  EXPECT_EQ(invalid_id_, id1);
  EXPECT_EQ(id2_, invalid_id);
}

}  // namespace test

}  // namespace maidsafe
