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

#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

size_t BitToByteCount(const size_t& bit_count) {
  return static_cast<size_t>(0.999999 + static_cast<double>(bit_count) / 8);
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
  NodeId zero_id;
  EXPECT_TRUE(zero_id.IsZero());
  EXPECT_EQ(zero_id, NodeId(std::string(NodeId::kSize, 0)));
}

TEST(NodeIdTest, BEH_DistanceCheck) {
  for (size_t i(0); i < 10000; ++i) {
    NodeId one(NodeId::kRandomId);
    NodeId two(NodeId::kRandomId);
    EXPECT_FALSE(one == two);
    EXPECT_TRUE((one ^ two) == (two ^ one));
  }
}

TEST(NodeIdTest, BEH_BitToByteCount) {
  for (size_t i = 0; i < NodeId::kSize; ++i) {
    ASSERT_EQ(i, BitToByteCount(8 * i));
    for (size_t j = 1; j < 8; ++j) {
      ASSERT_EQ(i + 1, BitToByteCount((8 * i) + j));
    }
  }
}

TEST(NodeIdTest, BEH_DefaultCtr) {
  NodeId node_id;
  ASSERT_EQ(NodeId::kSize, node_id.string().size());
  for (size_t i = 0; i < node_id.string().size(); ++i)
    ASSERT_EQ('\0', node_id.string()[i]);
  std::string hex_id(NodeId::kSize * 2, '0');
  ASSERT_EQ(hex_id, node_id.ToStringEncoded(NodeId::EncodingType::kHex));
  std::string bin_id(NodeId::kSize * 8, '0');
  ASSERT_EQ(bin_id, node_id.ToStringEncoded(NodeId::EncodingType::kBinary));
  EXPECT_THROW(NodeId dave("not64long"), maidsafe_error);
}

TEST(NodeIdTest, BEH_CopyCtr) {
  NodeId kadid1(NodeId::kRandomId);
  NodeId kadid2(kadid1);
  ASSERT_TRUE(kadid1 == kadid2);
  for (size_t i = 0; i < kadid1.string().size(); ++i)
    ASSERT_EQ(kadid1.string()[i], kadid2.string()[i]);
  ASSERT_EQ(kadid1.ToStringEncoded(NodeId::EncodingType::kBinary),
            kadid2.ToStringEncoded(NodeId::EncodingType::kBinary));
  ASSERT_EQ(kadid1.ToStringEncoded(NodeId::EncodingType::kHex),
            kadid2.ToStringEncoded(NodeId::EncodingType::kHex));
  ASSERT_EQ(kadid1.string(), kadid2.string());
}

TEST(NodeIdTest, BEH_KadIdTypeCtr) {
  std::string min_id(NodeId::kSize, 0);
  ASSERT_EQ(NodeId::kSize, min_id.size());
  for (int i = 0; i < NodeId::kSize; ++i)
    ASSERT_EQ(min_id[i], '\0');
  NodeId max_id(NodeId::kMaxId);
  ASSERT_EQ(NodeId::kSize, max_id.string().size());
  for (int i = 0; i < NodeId::kSize; ++i)
    ASSERT_EQ(-1, max_id.string()[i]);
  NodeId rand_id(NodeId::kRandomId);
  ASSERT_EQ(NodeId::kSize, rand_id.string().size());
  // TODO(Fraser#5#): 2010-06-06 - Test for randomness properly
  ASSERT_NE(rand_id.string(), NodeId(NodeId::kRandomId).string());
  EXPECT_THROW(NodeId(static_cast<NodeId::IdType>(-999)), maidsafe_error);
}

TEST(NodeIdTest, BEH_StringCtr) {
  std::string rand_str(RandomString(NodeId::kSize));
  NodeId id1(rand_str);
  ASSERT_TRUE(id1.string() == rand_str);
  EXPECT_THROW(NodeId id2(rand_str.substr(0, NodeId::kSize - 1)), maidsafe_error);
}

TEST(NodeIdTest, BEH_EncodingCtr) {
  std::string known_raw(NodeId::kSize, 0);
  for (char c = 0; c < NodeId::kSize; ++c)
    known_raw.at(static_cast<uint8_t>(c)) = c;
  for (int i = 0; i < 3; ++i) {
    std::string rand_str(RandomString(NodeId::kSize));
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

    EXPECT_THROW(NodeId bad_id(bad_encoded, type), maidsafe_error);
    //    ASSERT_TRUE(bad_id.string().empty());
    //    ASSERT_FALSE(bad_id.IsValid());
    //    ASSERT_TRUE(bad_id.ToStringEncoded(type).empty());
    NodeId rand_id(encoded, type);
    ASSERT_EQ(rand_str, rand_id.string());
    ASSERT_EQ(encoded, rand_id.ToStringEncoded(type));
    NodeId known_id(known_encoded, type);
    ASSERT_EQ(known_raw, known_id.string());
    ASSERT_EQ(known_encoded, known_id.ToStringEncoded(type));
    switch (i) {
      case static_cast<int>(NodeId::EncodingType::kBinary) :
        ASSERT_EQ(
            "000000000000000100000010000000110000010000000101000001100000"
            "011100001000000010010000101000001011000011000000110100001110"
            "000011110001000000010001000100100001001100010100000101010001"
            "011000010111000110000001100100011010000110110001110000011101"
            "000111100001111100100000001000010010001000100011001001000010"
            "010100100110001001110010100000101001001010100010101100101100"
            "001011010010111000101111001100000011000100110010001100110011"
            "010000110101001101100011011100111000001110010011101000111011"
            "00111100001111010011111000111111",
            known_encoded);
        break;
      case static_cast<int>(NodeId::EncodingType::kHex) :
        ASSERT_EQ(
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d"
            "1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b"
            "3c3d3e3f",
            known_encoded);
        break;
      case static_cast<int>(NodeId::EncodingType::kBase64) :
        ASSERT_EQ(
            "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKiss"
            "LS4vMDEyMzQ1Njc4OTo7PD0+Pw==",
            known_encoded);
        break;
      default:
        break;
    }
  }
}

TEST(NodeIdTest, BEH_OperatorEqual) {
  NodeId kadid1(NodeId::kRandomId);
  std::string id(kadid1.string());
  NodeId kadid2(id);
  ASSERT_TRUE(kadid1 == kadid2) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid2 = "
                                << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
  std::string id1;
  for (size_t i = 0; i < BitToByteCount(NodeId::kSize * 8) * 2; ++i) {
    id1 += "f";
  }
  NodeId kadid3(id1, NodeId::EncodingType::kHex);
  ASSERT_FALSE(kadid1 == kadid3) << "kadid1 = "
                                 << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                 << std::endl << "kadid3 = "
                                 << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                 << std::endl;
}

TEST(NodeIdTest, BEH_OperatorDifferent) {
  NodeId kadid1(NodeId::kRandomId);
  std::string id(kadid1.string());
  NodeId kadid2(id);
  ASSERT_FALSE(kadid1 != kadid2) << "kadid1 = "
                                 << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                 << std::endl << "kadid2 = "
                                 << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                 << std::endl;
  std::string id1;
  for (size_t i = 0; i < BitToByteCount(NodeId::kSize * 8) * 2; ++i)
    id1 += "f";
  NodeId kadid3(id1, NodeId::EncodingType::kHex);
  ASSERT_TRUE(kadid1 != kadid3) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid3 = "
                                << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
}

TEST(NodeIdTest, BEH_OperatorGreaterThan) {
  NodeId kadid1(NodeId::kRandomId);
  while (kadid1 == NodeId(NodeId::kMaxId))
    kadid1 = NodeId(NodeId::kRandomId);
  NodeId kadid2(kadid1);
  ASSERT_FALSE(kadid1 > kadid2) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid2 = "
                                << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
  NodeId kadid3(IncreaseId(kadid1));
  ASSERT_TRUE(kadid3 > kadid1) << "kadid3 = "
                               << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary) << std::endl
                               << "kadid1 = "
                               << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                               << std::endl;
  ASSERT_FALSE(kadid1 > kadid3) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid3 = "
                                << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
}

TEST(NodeIdTest, BEH_OperatorLessThan) {
  NodeId kadid1(NodeId::kRandomId);
  while (kadid1 == NodeId(NodeId::kMaxId))
    kadid1 = NodeId(NodeId::kRandomId);
  NodeId kadid2(kadid1);
  ASSERT_FALSE(kadid1 < kadid2) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid2 = "
                                << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
  NodeId kadid3(IncreaseId(kadid1));
  ASSERT_TRUE(kadid1 < kadid3) << "kadid1 = "
                               << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary) << std::endl
                               << "kadid3 = "
                               << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                               << std::endl;
  ASSERT_FALSE(kadid3 < kadid1) << "kadid3 = "
                                << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
}

TEST(NodeIdTest, BEH_OperatorGreaterEqual) {
  NodeId kadid1(NodeId::kRandomId);
  while (kadid1 == NodeId(NodeId::kMaxId))
    kadid1 = NodeId(NodeId::kRandomId);
  NodeId kadid2(kadid1);
  ASSERT_TRUE(kadid1 >= kadid2) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid2 = "
                                << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
  NodeId kadid3(IncreaseId(kadid1));
  ASSERT_TRUE(kadid3 >= kadid1) << "kadid3 = "
                                << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
}

TEST(NodeIdTest, BEH_OperatorLessEqual) {
  NodeId kadid1(NodeId::kRandomId);
  while (kadid1 == NodeId(NodeId::kMaxId))
    kadid1 = NodeId(NodeId::kRandomId);
  NodeId kadid2(kadid1);
  ASSERT_TRUE(kadid1 <= kadid2) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid2 = "
                                << kadid2.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
  NodeId kadid3(IncreaseId(kadid1));
  ASSERT_TRUE(kadid1 <= kadid3) << "kadid1 = "
                                << kadid1.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl << "kadid3 = "
                                << kadid3.ToStringEncoded(NodeId::EncodingType::kBinary)
                                << std::endl;
}

TEST(NodeIdTest, BEH_OperatorXOR) {
  NodeId kadid1(NodeId::kRandomId), kadid2(NodeId::kRandomId);
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

TEST(NodeIdTest, BEH_OperatorEql) {
  NodeId kadid1(NodeId::kRandomId), kadid2;
  kadid2 = kadid1;
  ASSERT_TRUE(kadid1 == kadid2);
  for (size_t i = 0; i < kadid1.string().size(); ++i)
    ASSERT_EQ(kadid1.string()[i], kadid2.string()[i]);
  ASSERT_EQ(kadid1.ToStringEncoded(NodeId::EncodingType::kBinary),
            kadid2.ToStringEncoded(NodeId::EncodingType::kBinary));
  ASSERT_EQ(kadid1.ToStringEncoded(NodeId::EncodingType::kHex),
            kadid2.ToStringEncoded(NodeId::EncodingType::kHex));
  ASSERT_EQ(kadid1.string(), kadid2.string());
}

}  // namespace test

}  // namespace maidsafe
