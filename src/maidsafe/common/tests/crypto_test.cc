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

#include <algorithm>
#include <cstdlib>
#include <random>
#include <string>

#include "boost/lexical_cast.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/fstream.hpp"

#include "maidsafe/common/encode.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace crypto {

namespace test {

TEST(CryptoTest, BEH_SecurePasswordGeneration) {
  const NonEmptyString kKnownPassword1(hex::DecodeToBytes("70617373776f7264"));
  const Salt kKnownSalt1(hex::DecodeToBytes("1234567878563412"));
  const uint32_t kKnownIterations1(5);
  const SecurePassword kKnownDerived1(SecurePassword::value_type(hex::DecodeToBytes(
      "4391697b647773d2ac29693853dc66c21f036d36256a8b1e6"
      "17b2364af10aee1e53d7d4ef0c237f40c539769e4f162e0")));
  EXPECT_THROW(CreateSecurePassword(NonEmptyString(), kKnownSalt1, kKnownIterations1),
               common_error);
  EXPECT_THROW(CreateSecurePassword(kKnownPassword1, Salt(), kKnownIterations1), common_error);

  SecurePassword password(CreateSecurePassword(kKnownPassword1, kKnownSalt1, kKnownIterations1));
  EXPECT_EQ(kKnownDerived1, password);
  const NonEmptyString kKnownPassword2(hex::DecodeToBytes(
      "416c6c206e2d656e746974696573206d75737420636f6"
      "d6d756e69636174652077697468206f74686572206e2d656e74697469657320766961206e2d3120656e746974656"
      "568656568656573"));
  const Salt kKnownSalt2(hex::DecodeToBytes("1234567878563412"));
  const uint32_t kKnownIterations2(500);
  const SecurePassword kKnownDerived2(SecurePassword::value_type(hex::DecodeToBytes(
      "c1999230ef5e0196b71598bb945247391fa3d53ca46e5bcf9"
      "c697256c7b131d3bcf310b523e05c3ffc14d7fd8511c840")));
  password = CreateSecurePassword(kKnownPassword2, kKnownSalt2, kKnownIterations2);
  EXPECT_EQ(kKnownDerived2, password);
}

template <typename T>
struct ShaTestData {
  ShaTestData(const std::string& input_data, const std::string& SHA1_hex_res,
              const std::string& SHA256_hex_res, const std::string& SHA384_hex_res,
              const std::string& SHA512_hex_res)
      : input(input_data.begin(), input_data.end()),
        SHA1_hex_result(SHA1_hex_res),
        SHA256_hex_result(SHA256_hex_res),
        SHA384_hex_result(SHA384_hex_res),
        SHA512_hex_result(SHA512_hex_res),
        SHA1_raw_result(Decode(SHA1_hex_res)),
        SHA256_raw_result(Decode(SHA256_hex_res)),
        SHA384_raw_result(Decode(SHA384_hex_res)),
        SHA512_raw_result(Decode(SHA512_hex_res)) {}
  T input;
  std::string SHA1_hex_result;
  std::string SHA256_hex_result;
  std::string SHA384_hex_result;
  std::string SHA512_hex_result;
  T SHA1_raw_result;
  T SHA256_raw_result;
  T SHA384_raw_result;
  T SHA512_raw_result;

 private:
  T Decode(const std::string& data);
};

template <>
std::string ShaTestData<std::string>::Decode(const std::string& data) {
  return hex::DecodeToString(data);
}

template <>
std::vector<byte> ShaTestData<std::vector<byte>>::Decode(const std::string& data) {
  return hex::DecodeToBytes(data);
}

template <typename T>
struct UnderlyingString;

template <>
struct UnderlyingString<std::string> {
  using Type = std::string;
};

template <>
struct UnderlyingString<std::vector<byte>> {
  using Type = std::vector<byte>;
};

template <>
struct UnderlyingString<detail::BoundedString<1, static_cast<std::size_t>(-1), std::string>> {
  using Type = std::string;
};

template <>
struct UnderlyingString<NonEmptyString> {
  using Type = std::vector<byte>;
};

template <typename T>
class ShaTest : public testing::Test {
 protected:
  using UnderlyingStringType = typename UnderlyingString<T>::Type;
};

using ShaTestTypes =
    testing::Types<std::string, std::vector<byte>,
                   detail::BoundedString<1, static_cast<std::size_t>(-1), std::string>,
                   NonEmptyString>;

TYPED_TEST_CASE(ShaTest, ShaTestTypes);

TYPED_TEST(ShaTest, BEH_All) {
  using ShaTestData = ShaTestData<typename TestFixture::UnderlyingStringType>;

  // Set up industry standard test data
  std::vector<ShaTestData> test_data;
  test_data.push_back(
      ShaTestData("abc", "a9993e364706816aba3e25717850c26c9cd0d89d",
                  "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                  "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2"
                  "358baeca134c825a7",
                  "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a"
                  "836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f"));
  test_data.push_back(
      ShaTestData("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                  "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
                  "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", "", ""));
  test_data.push_back(
      ShaTestData(std::string(64 * 15625, 'a'), "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
                  "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
                  "9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4e"
                  "bae97ddd87f3d8985",
                  "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60"
                  "a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"));
  test_data.push_back(ShaTestData(
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmno"
      "pqrsmnopqrstnopqrstu",
      "", "",
      "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa9174"
      "6039",
      "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5"
      "433ac7d329eeb6dd26545e96e55b874be909"));

  // Run tests
  for (size_t j = 0; j < test_data.size(); ++j) {
    TypeParam input(test_data.at(j).input);
    if (!test_data.at(j).SHA1_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA1_hex_result, hex::Encode(Hash<SHA1>(input)));
      EXPECT_EQ(test_data.at(j).SHA1_raw_result, Hash<SHA1>(input).string());
    }

    if (!test_data.at(j).SHA256_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA256_hex_result, hex::Encode(Hash<SHA256>(input)));
      EXPECT_EQ(test_data.at(j).SHA256_raw_result, Hash<SHA256>(input).string());
    }

    if (!test_data.at(j).SHA384_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA384_hex_result, hex::Encode(Hash<SHA384>(input)));
      EXPECT_EQ(test_data.at(j).SHA384_raw_result, Hash<SHA384>(input).string());
    }

    if (!test_data.at(j).SHA512_hex_result.empty()) {
      EXPECT_EQ(test_data.at(j).SHA512_hex_result, hex::Encode(Hash<SHA512>(input)));
      EXPECT_EQ(test_data.at(j).SHA512_raw_result, Hash<SHA512>(input).string());
    }
  }

  // Check using default-constructed BoundedStrings
  EXPECT_THROW(Hash<SHA1>(NonEmptyString()), common_error);
  EXPECT_THROW(Hash<SHA256>(Identity()), common_error);
}

std::vector<byte> CorruptData(std::vector<byte> input) {
  // Replace a single char of input to a different random char.
  ++input[RandomUint32() % input.size()];
  return input;
}

TEST(CryptoTest, BEH_SymmEncrypt) {
  // Set up data
  const AES256KeyAndIV kKeyAndIV(hex::DecodeToBytes(
      "0a89927670e292af98080a3c3e2bdee4289b768de74570f9f470282756390fe392af98080a3c3e2bdee4289b768d"
      "e7af"));
  const PlainText kUnencrypted(hex::DecodeToBytes(
      "8b4a84c8f409d8c8b4a8e70f49867c63661f2b31d6e4c984a6a01b2015e48a47bc46af231d2b146e54a87db43f51"
      "c2a5"));
  const CipherText kEncrypted(NonEmptyString(hex::DecodeToBytes(
      "f7b043e78bc86c801a9f37850238d07702ffc59945473f5c88ff1854fcbeedb2c1fe6fdfc7ccb040ee608e8f60b3"
      "00e4b969aef8ac9a7b1d00c52d9133c6b1d9")));
  const AES256KeyAndIV kBadKeyOrIV(CorruptData(kKeyAndIV.string()));
  const PlainText kBadUnencrypted(CorruptData(kUnencrypted.string()));
  const CipherText kBadEncrypted(NonEmptyString(CorruptData(kEncrypted->string())));

  // Encryption
  EXPECT_EQ(kEncrypted, SymmEncrypt(kUnencrypted, kKeyAndIV));
  EXPECT_NE(kEncrypted, SymmEncrypt(kBadUnencrypted, kKeyAndIV));
  EXPECT_NE(kEncrypted, SymmEncrypt(kUnencrypted, kBadKeyOrIV));
  EXPECT_THROW(SymmEncrypt(PlainText(), kKeyAndIV), common_error);
  EXPECT_THROW(SymmEncrypt(kUnencrypted, AES256KeyAndIV()), common_error);

  // Decryption
  EXPECT_EQ(kUnencrypted, SymmDecrypt(kEncrypted, kKeyAndIV));
  EXPECT_THROW(SymmDecrypt(kBadEncrypted, kKeyAndIV), common_error);
  EXPECT_THROW(SymmDecrypt(kEncrypted, kBadKeyOrIV), common_error);
  EXPECT_THROW(SymmDecrypt(CipherText(NonEmptyString()), kKeyAndIV), common_error);
  EXPECT_THROW(SymmDecrypt(kEncrypted, AES256KeyAndIV()), common_error);
}

TEST(CryptoTest, BEH_Compress) {
  EXPECT_THROW(Compress(UncompressedText(), 1), common_error);
  EXPECT_THROW(Uncompress(CompressedText()), common_error);

  const size_t kTestDataSize(10000);
  const size_t kTolerance(kTestDataSize / 200);
  std::vector<byte> initial_data(kTestDataSize / 2, 'A');
  std::vector<byte> random_data(RandomBytes(kTestDataSize / 2));
  initial_data.insert(initial_data.end(), random_data.begin(), random_data.end());

  std::mt19937 rng(RandomUint32());
  std::shuffle(initial_data.begin(), initial_data.end(), rng);
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
  EXPECT_THROW(Compress(kTestData, kMaxCompressionLevel + 1), common_error);

  // Try to uncompress uncompressed data
  EXPECT_THROW(Uncompress(CompressedText(kTestData)), common_error);
}

TEST(CryptoTest, BEH_GzipSHA512Deterministic) {
  // if the algorithm changes this test will start failing as it is a bit of a sledgehammer approach
  std::string test_data = "11111111111111122222222222222222222333333333333";
  std::vector<std::string> answer;

  answer.emplace_back(
      "b29c3470f1241f1d05393d2bf6c5b72201459ae43dc0da850ef3550480a7f884d1d2a03d0e25"
      "832af90d545b3b283f93fd29d89d7d5975ebcdd697048f550134");
  answer.emplace_back(
      "cb67021cf302f59eee8f593d7705261ab3d41f353eadf8d911e087f36d9a0de6f0489ab7546e"
      "3d06a81e6a4ccc75d49184bd81ad8d4ab5eaeebde637e2f7cb05");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "b72d4948dcee2878432f1044b39bbb541ba5ac412ea5602b4cc5d3b6760bc864cdfc94d6a8e1"
      "31e5fd06603db357b03752cad7080def2eed1854267bf42328d1");
  answer.emplace_back(
      "d3261fe3c660734571787e5aa730c2e5bf18886e28e2b346cfe7b8dd4c44e6d01a88526647df"
      "8c7555330f3d347e1ac3735e1a73c79c258e9fa7094f9ab07e33");

  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(hex::Encode(Hash<SHA512>(
                  Compress(UncompressedText(std::vector<byte>(test_data.begin(), test_data.end())),
                           static_cast<uint16_t>(i))->string())),
              answer.at(i));
  }

  for (int i = 1; i < 20; ++i)
    test_data += test_data;
  // 23 Mb approx
  std::string answer2 =
      ("fb5e2660c5a6f5c59ef8379df0862c4fa8504e55ba1eed54c92ffe335cb126b12c8"
       "171815f0d17bf31e21c9fd3979b543ad91df08370a44a66e7a010d2b6e02f");
  // For large data sets only compression levels 0 and 1 are deterministic!
  EXPECT_EQ(hex::Encode(Hash<SHA512>(
                Compress(UncompressedText(std::vector<byte>(test_data.begin(), test_data.end())), 1)
                    ->string())),
            answer2);
}

class InformationDispersalTest : public testing::Test {
 protected:
  InformationDispersalTest()
      : data_size_(1),
        threshold_(0),
        number_of_shares_(0),
        random_data_(),
        dispersed_data_parts_(),
        secret_data_parts_() {}

  enum class PartsType { kDispersed, kSecret };

  DataParts GetRandomParts(uint8_t count, DataParts& all_parts) {
    DataParts parts(std::begin(all_parts), std::begin(all_parts) + count);
    std::mt19937 rng(RandomUint32());
    std::shuffle(std::begin(parts), std::end(parts), rng);
    return parts;
  }

  size_t data_size_;
  uint8_t threshold_, number_of_shares_;
  PlainText random_data_;
  DataParts dispersed_data_parts_, secret_data_parts_;
};

TEST_F(InformationDispersalTest, BEH_Basic) {
  // Basic test setup
  threshold_ = 2;
  number_of_shares_ = 3;
  random_data_ = PlainText(std::vector<byte>(100, 'A'));

  // Test IDA
  dispersed_data_parts_ = InfoDisperse(threshold_, number_of_shares_, random_data_);
  EXPECT_EQ(number_of_shares_, dispersed_data_parts_.size());
  auto dispersed_parts(GetRandomParts(threshold_, dispersed_data_parts_));
  EXPECT_EQ(random_data_, InfoRetrieve(dispersed_parts));

  // Test Secret Shared
  secret_data_parts_ = SecretShareData(threshold_, number_of_shares_, random_data_);
  EXPECT_EQ(number_of_shares_, secret_data_parts_.size());
  auto secret_parts(GetRandomParts(threshold_, secret_data_parts_));
  EXPECT_EQ(random_data_, SecretRecoverData(secret_parts));

  // Test with threshold too low
  EXPECT_THROW(InfoDisperse(1, number_of_shares_, random_data_), common_error);
  EXPECT_THROW(SecretShareData(1, number_of_shares_, random_data_), common_error);

  // Test with threshold > number of shares
  EXPECT_THROW(InfoDisperse(4, number_of_shares_, random_data_), common_error);
  EXPECT_THROW(SecretShareData(4, number_of_shares_, random_data_), common_error);

  // Test with number of shares too low
  EXPECT_THROW(InfoDisperse(threshold_, 2, random_data_), common_error);
  EXPECT_THROW(SecretShareData(threshold_, 2, random_data_), common_error);

  // Test with too few parts
  dispersed_parts.pop_back();
  secret_parts.pop_back();
  EXPECT_NE(random_data_, InfoRetrieve(dispersed_parts));
  EXPECT_NE(random_data_, SecretRecoverData(secret_parts));

  // Test with too many parts
  EXPECT_NE(random_data_, InfoRetrieve(dispersed_data_parts_));
  EXPECT_EQ(random_data_, SecretRecoverData(secret_data_parts_));
}

TEST_F(InformationDispersalTest, FUNC_MultipleValues) {
  // Iterate through increasing sizes of input data starting at 1 B and up to 2 MB max.
  do {
    random_data_ = PlainText(RandomBytes(data_size_));

    if (data_size_ < 1024 * 100) {  // otherwise test takes too long
      // Use random number of shares in range [3, 102] with minimum threshold
      number_of_shares_ = (RandomUint32() % 100) + 3;
      threshold_ = 2;
      // IDA
      dispersed_data_parts_ = InfoDisperse(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, dispersed_data_parts_.size());
      auto dispersed_parts(GetRandomParts(threshold_, dispersed_data_parts_));
      EXPECT_EQ(random_data_, InfoRetrieve(dispersed_parts));
      // Secret Share
      secret_data_parts_ = SecretShareData(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, secret_data_parts_.size());
      auto secret_parts(GetRandomParts(threshold_, secret_data_parts_));
      EXPECT_EQ(random_data_, SecretRecoverData(secret_parts));

      // Use same number of shares, but with maximum threshold
      threshold_ = number_of_shares_;
      // IDA
      dispersed_data_parts_ = InfoDisperse(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, dispersed_data_parts_.size());
      dispersed_parts = GetRandomParts(threshold_, dispersed_data_parts_);
      EXPECT_EQ(random_data_, InfoRetrieve(dispersed_parts));
      // Secret Share
      secret_data_parts_ = SecretShareData(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, secret_data_parts_.size());
      secret_parts = GetRandomParts(threshold_, secret_data_parts_);
      EXPECT_EQ(random_data_, SecretRecoverData(secret_parts));


      // Use same number of shares, but with threshold between max and min.
      threshold_ = (RandomUint32() % (number_of_shares_ - 2)) + 3;
      // IDA
      dispersed_data_parts_ = InfoDisperse(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, dispersed_data_parts_.size());
      dispersed_parts = GetRandomParts(threshold_, dispersed_data_parts_);
      EXPECT_EQ(random_data_, InfoRetrieve(dispersed_parts));
      // Secret Share
      secret_data_parts_ = SecretShareData(threshold_, number_of_shares_, random_data_);
      EXPECT_EQ(number_of_shares_, secret_data_parts_.size());
      secret_parts = GetRandomParts(threshold_, secret_data_parts_);
      EXPECT_EQ(random_data_, SecretRecoverData(secret_parts));
    }

    // Use 32 shares with threshold of 29 since that's likely to be used by Routing
    number_of_shares_ = 32;
    threshold_ = 29;
    // IDA
    dispersed_data_parts_ = InfoDisperse(threshold_, number_of_shares_, random_data_);
    EXPECT_EQ(number_of_shares_, dispersed_data_parts_.size());
    auto dispersed_parts(GetRandomParts(threshold_, dispersed_data_parts_));
    EXPECT_EQ(random_data_, InfoRetrieve(dispersed_parts));
    // Secret Share
    secret_data_parts_ = SecretShareData(threshold_, number_of_shares_, random_data_);
    EXPECT_EQ(number_of_shares_, secret_data_parts_.size());
    auto secret_parts(GetRandomParts(threshold_, secret_data_parts_));
    EXPECT_EQ(random_data_, SecretRecoverData(secret_parts));

    if (data_size_ == 1)
      data_size_ = (RandomUint32() % 100) + 1;
    else
      data_size_ *= (RandomUint32() % 100) + 10;
  } while (data_size_ < 2 * 1024 * 1024);
}

}  // namespace test

}  // namespace crypto

}  // namespace maidsafe
