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

#include "maidsafe/common/crypto.h"

#include <cstdlib>
#include <string>

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/fstream.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace crypto {

namespace test {

TEST(CryptoTest, FUNC_Obfuscation) {
  const size_t kStringSize(1024 * 256);
  detail::BoundedString<kStringSize, kStringSize> str1(RandomString(kStringSize));
  detail::BoundedString<kStringSize, kStringSize> str2(RandomString(kStringSize));
  detail::BoundedString<kStringSize, kStringSize> obfuscated(XOR(str1, str2));
  EXPECT_EQ(kStringSize, obfuscated.string().size());
  EXPECT_EQ(obfuscated, XOR(str2, str1));
  EXPECT_EQ(str1, XOR(obfuscated, str2));
  EXPECT_EQ(str2, XOR(obfuscated, str1));

  const detail::BoundedString<kStringSize, kStringSize> kZeros(std::string(kStringSize, 0));
  EXPECT_EQ(kZeros, XOR(str1, str1));
  EXPECT_EQ(str1, XOR(kZeros, str1));

  const detail::BoundedString<2, 2> kKnown1("\xa5\x5a");
  const detail::BoundedString<2, 2> kKnown2("\x5a\xa5");
  EXPECT_EQ(std::string("\xff\xff"), XOR(kKnown1, kKnown2).string());
}

TEST(CryptoTest, BEH_Xor) {
  EXPECT_THROW(XOR("A", ""), std::exception);
  EXPECT_THROW(XOR("", "B"), std::exception);
  EXPECT_THROW(XOR("A", "BB"), std::exception);
  const size_t kStringSize(1024 * 256);
  std::string str1 = RandomString(kStringSize);
  std::string str2 = RandomString(kStringSize);
  std::string obfuscated = XOR(str1, str2);
  EXPECT_EQ(kStringSize, obfuscated.size());
  EXPECT_EQ(obfuscated, XOR(str2, str1));
  EXPECT_EQ(str1, XOR(obfuscated, str2));
  EXPECT_EQ(str2, XOR(obfuscated, str1));
  const std::string kZeros(kStringSize, 0);
  EXPECT_EQ(kZeros, XOR(str1, str1));
  EXPECT_EQ(str1, XOR(kZeros, str1));
  const std::string kKnown1("\xa5\x5a");
  const std::string kKnown2("\x5a\xa5");
  EXPECT_EQ(std::string("\xff\xff"), XOR(kKnown1, kKnown2));
}

TEST(CryptoTest, BEH_SecurePasswordGeneration) {
  EXPECT_THROW(SecurePassword(SecurePassword::value_type("")), std::exception);
  EXPECT_THROW(NonEmptyString(""), std::exception);
  EXPECT_THROW(Salt(""), std::exception);
  const NonEmptyString kKnownPassword1(HexDecode("70617373776f7264"));
  const Salt kKnownSalt1(HexDecode("1234567878563412"));
  const uint32_t kKnownIterations1(5);
  const SecurePassword kKnownDerived1(SecurePassword::value_type(HexDecode(
      "4391697b647773d2ac29693853dc66c21f036d36256a8b1e6"
      "17b2364af10aee1e53d7d4ef0c237f40c539769e4f162e0")));
  EXPECT_THROW(CreateSecurePassword(NonEmptyString(), kKnownSalt1, kKnownIterations1),
               std::exception);
  EXPECT_THROW(CreateSecurePassword(kKnownPassword1, Salt(), kKnownIterations1), std::exception);

  SecurePassword password(CreateSecurePassword(kKnownPassword1, kKnownSalt1, kKnownIterations1));
  EXPECT_EQ(kKnownDerived1, password);
  const NonEmptyString kKnownPassword2(HexDecode(
      "416c6c206e2d656e746974696573206d75737420636f6"
      "d6d756e69636174652077697468206f74686572206e2d656e74697469657320766961206e2d3120656e746974656"
      "568656568656573"));
  const Salt kKnownSalt2(HexDecode("1234567878563412"));
  const uint32_t kKnownIterations2(500);
  const SecurePassword kKnownDerived2(SecurePassword::value_type(HexDecode(
      "c1999230ef5e0196b71598bb945247391fa3d53ca46e5bcf9"
      "c697256c7b131d3bcf310b523e05c3ffc14d7fd8511c840")));
  password = CreateSecurePassword(kKnownPassword2, kKnownSalt2, kKnownIterations2);
  EXPECT_EQ(kKnownDerived2, password);
}

struct HashTestData {
  HashTestData(std::string input_data, const std::string& SHA1_hex_res,
               const std::string& SHA256_hex_res, const std::string& SHA384_hex_res,
               const std::string& SHA512_hex_res)
      : input(std::move(input_data)),
        SHA1_hex_result(SHA1_hex_res),
        SHA256_hex_result(SHA256_hex_res),
        SHA384_hex_result(SHA384_hex_res),
        SHA512_hex_result(SHA512_hex_res),
        SHA1_raw_result(HexDecode(SHA1_hex_res)),
        SHA256_raw_result(HexDecode(SHA256_hex_res)),
        SHA384_raw_result(HexDecode(SHA384_hex_res)),
        SHA512_raw_result(HexDecode(SHA512_hex_res)) {}
  std::string input;
  std::string SHA1_hex_result;
  std::string SHA256_hex_result;
  std::string SHA384_hex_result;
  std::string SHA512_hex_result;
  std::string SHA1_raw_result;
  std::string SHA256_raw_result;
  std::string SHA384_raw_result;
  std::string SHA512_raw_result;
};

TEST(CryptoTest, BEH_Hash) {
  // Set up industry standard test data
  std::vector<HashTestData> test_data;
  test_data.push_back(
      HashTestData("abc", "a9993e364706816aba3e25717850c26c9cd0d89d",
                   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                   "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed"
                   "8086072ba1e7cc2358baeca134c825a7",
                   "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
                   "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f"));
  test_data.push_back(
      HashTestData("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                   "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
                   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", "", ""));
  test_data.push_back(
      HashTestData(std::string(64 * 15625, 'a'), "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
                   "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
                   "9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b"
                   "07b8b3dc38ecc4ebae97ddd87f3d8985",
                   "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
                   "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"));
  test_data.push_back(HashTestData(
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
      "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
      "", "",
      "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712"
      "fcc7c71a557e2db966c3e9fa91746039",
      "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
      "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909"));

  // Set up temp test dir and files
  std::shared_ptr<fs::path> test_dir(maidsafe::test::CreateTestPath("MaidSafe_TestCrypto"));
  EXPECT_FALSE(test_dir->empty());
  std::vector<fs::path> input_files;

  // Run tests
  for (size_t j = 0; j < test_data.size(); ++j) {
    std::string input(test_data.at(j).input);
    if (!test_data.at(j).SHA1_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA1_hex_result, HexEncode(Hash<SHA1>(input)));
      EXPECT_EQ(test_data.at(j).SHA1_raw_result, Hash<SHA1>(input).string());
    }

    if (!test_data.at(j).SHA256_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA256_hex_result, HexEncode(Hash<SHA256>(input)));
      EXPECT_EQ(test_data.at(j).SHA256_raw_result, Hash<SHA256>(input).string());
    }

    if (!test_data.at(j).SHA384_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA384_hex_result, HexEncode(Hash<SHA384>(input)));
      EXPECT_EQ(test_data.at(j).SHA384_raw_result, Hash<SHA384>(input).string());
    }

    if (!test_data.at(j).SHA512_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA512_hex_result, HexEncode(Hash<SHA512>(input)));
      EXPECT_EQ(test_data.at(j).SHA512_raw_result, Hash<SHA512>(input).string());
    }
  }

  // Check using default-constructed BoundedStrings
  EXPECT_THROW(Hash<SHA1>(NonEmptyString()), std::exception);
  EXPECT_THROW(Hash<SHA256>(Identity()), std::exception);
}

std::string CorruptData(const std::string& input) {
  // Replace a single char of input to a different random char.
  std::string output(input);
  output.back() += (RandomUint32() % 254) + 1;
  return output;
}

TEST(CryptoTest, BEH_SymmEncrypt) {
  // Set up data
  const AES256Key kKey(
      HexDecode("0a89927670e292af98080a3c3e2bdee4289b768de74570f9f470282756390fe3"));
  const AES256InitialisationVector kIV(HexDecode("92af98080a3c3e2bdee4289b768de7af"));
  const PlainText kUnencrypted(HexDecode(
      "8b4a84c8f409d8c8b4a8e70f49867c63661f2b31d6e4c984"
      "a6a01b2015e48a47bc46af231d2b146e54a87db43f51c2a5"));
  const CipherText kEncrypted(NonEmptyString(HexDecode(
      "441f907b71a14c2f482c4d1fef61f3d7ffc0f14953f4f575"
      "601803ffa6f13e9f3097a0d93c85f1dd10b815822b969644")));
  const AES256Key kBadKey(CorruptData(kKey.string()));
  const AES256InitialisationVector kBadIV(CorruptData(kIV.string()));
  const PlainText kBadUnencrypted(CorruptData(kUnencrypted.string()));
  const CipherText kBadEncrypted(NonEmptyString(CorruptData(kEncrypted->string())));

  EXPECT_THROW(CipherText(NonEmptyString("")), std::exception);
  EXPECT_THROW(PlainText(""), std::exception);
  EXPECT_THROW(AES256Key(std::string(AES256_KeySize - 1, 0)), std::exception);
  EXPECT_THROW(AES256InitialisationVector(std::string(AES256_IVSize - 1, 0)), std::exception);
  EXPECT_NO_THROW(AES256Key(std::string(AES256_KeySize, 0)));
  EXPECT_NO_THROW(AES256InitialisationVector(std::string(AES256_IVSize, 0)));
  EXPECT_THROW(AES256Key(std::string(AES256_KeySize + 1, 0)), std::exception);
  EXPECT_THROW(AES256InitialisationVector(std::string(AES256_IVSize + 1, 0)), std::exception);

  // Encryption string to string
  EXPECT_EQ(kEncrypted, SymmEncrypt(kUnencrypted, kKey, kIV));
  EXPECT_NE(kEncrypted, SymmEncrypt(kBadUnencrypted, kKey, kIV));
  EXPECT_NE(kEncrypted, SymmEncrypt(kUnencrypted, kBadKey, kBadIV));
  EXPECT_THROW(SymmEncrypt(PlainText(), kKey, kIV), std::exception);
  EXPECT_THROW(SymmEncrypt(kUnencrypted, AES256Key(), kIV), std::exception);
  EXPECT_THROW(SymmEncrypt(kUnencrypted, kKey, AES256InitialisationVector()), std::exception);

  // Decryption string to string
  EXPECT_EQ(kUnencrypted, SymmDecrypt(kEncrypted, kKey, kIV));
  EXPECT_NE(kUnencrypted, SymmDecrypt(kBadEncrypted, kKey, kIV));
  EXPECT_NE(kUnencrypted, SymmDecrypt(kEncrypted, kBadKey, kBadIV));
  EXPECT_THROW(SymmDecrypt(CipherText(NonEmptyString()), kKey, kIV), std::exception);
  EXPECT_THROW(SymmDecrypt(kEncrypted, AES256Key(), kIV), std::exception);
  EXPECT_THROW(SymmDecrypt(kEncrypted, kKey, AES256InitialisationVector()), std::exception);
}

TEST(CryptoTest, BEH_Compress) {
  EXPECT_THROW(CompressedText(NonEmptyString("")), std::exception);
  EXPECT_THROW(UncompressedText(""), std::exception);
  EXPECT_THROW(Compress(UncompressedText(), 1), std::exception);
  EXPECT_THROW(Uncompress(CompressedText()), std::exception);

  const size_t kTestDataSize(10000);
  const size_t kTolerance(kTestDataSize / 200);
  std::string initial_data(kTestDataSize, 'A');
  initial_data.replace(0, kTestDataSize / 2, RandomString(kTestDataSize / 2));
  std::random_shuffle(initial_data.begin(), initial_data.end());
  const UncompressedText kTestData(initial_data);

  // Compress
  std::vector<CompressedText> compressed_strings;
  for (uint16_t level = 0; level <= kMaxCompressionLevel; ++level) {
    compressed_strings.push_back(Compress(kTestData, level));
    if (level > 0) {
      EXPECT_GE(compressed_strings.at(level - 1)->string().size() + kTolerance,
                compressed_strings.at(level)->string().size());
    }
  }
  EXPECT_GT(kTestData.string().size(),
            compressed_strings.at(kMaxCompressionLevel)->string().size());

  // Uncompress
  for (uint16_t level = 0; level <= kMaxCompressionLevel; ++level)
    EXPECT_EQ(kTestData, Uncompress(compressed_strings.at(level)));

  // Try to compress with invalid compression level
  EXPECT_THROW(Compress(kTestData, kMaxCompressionLevel + 1), std::exception);

  // Try to uncompress uncompressed data
  EXPECT_THROW(Uncompress(CompressedText(kTestData)), std::exception);
}

TEST(CryptoTest, BEH_GzipSHA512Deterministic) {
  // if the algorithm changes this test will start failing as it is a bit of a sledgehammer approach
  std::string test_data = "11111111111111122222222222222222222333333333333";
  std::vector<std::string> answer;
  answer.emplace_back("b29c3470f1241f1d05393d2bf6c5b72201459ae43dc0da850ef3550480a"
                      "7f884d1d2a03d0e25832af90d545b3b283f93fd29d89d7d5975ebcdd697048f550134");
  answer.emplace_back("cb67021cf302f59eee8f593d7705261ab3d41f353eadf8d911e087f36d9a0de6f0489ab7546e"
                      "3d06a81e6a4ccc75d49184bd81ad8d4ab5eaeebde637e2f7cb05");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e"
                       "131e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back("d3261fe3c660734571787e5aa730c2e5bf18886e28e2b346cfe7b8dd4c44e6"
                      "d01a88526647df8c7555330f3d347e1ac3735e1a73c79c258e9fa7094f9ab07e33");
  for (int i = 0; i < 10; ++i ) {
    EXPECT_EQ(HexEncode(Hash<SHA512>(Compress(UncompressedText(test_data), i)->string())),
              answer.at(i));
  }

  for (int i = 1; i < 20; ++i)
    test_data += test_data;
  // 23 Mb approx
  std::string answer2 = ("fb5e2660c5a6f5c59ef8379df0862c4fa8504e55ba1eed54c92ffe335cb126b12c8"
      "171815f0d17bf31e21c9fd3979b543ad91df08370a44a66e7a010d2b6e02f");
  // For large data sets only compression levels 0 and 1 are deterministic!
  EXPECT_EQ(HexEncode(Hash<SHA512>(Compress(UncompressedText(test_data), 1)->string())), answer2);
}

TEST(CryptoTest, BEH_SecretSharing) {
  std::string rand_string(RandomString(64));
  uint8_t num_shares(20);
  uint8_t threshold(10);
  std::vector<std::string> data_parts(SecretShareData(threshold, num_shares, rand_string));
  std::string recovered(SecretRecoverData(threshold, data_parts));
  EXPECT_EQ(recovered, rand_string);
  uint8_t not_enough(9);
  recovered = SecretRecoverData(not_enough, data_parts);
  EXPECT_NE(recovered, rand_string);
  uint8_t too_many(100);
  recovered = SecretRecoverData(too_many, data_parts);
  EXPECT_EQ(recovered, rand_string);
}

}  // namespace test

}  // namespace crypto

}  // namespace maidsafe
