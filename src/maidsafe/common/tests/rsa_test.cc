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


#include "maidsafe/common/rsa.h"

#include <future>
#include <thread>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace rsa {

namespace test {

class RSATest : public testing::Test {
 public:
  RSATest() : keys_() {}
  ~RSATest() {}
 protected:
  Keys keys_;
  void SetUp() {
    ASSERT_NO_THROW(keys_ = GenerateKeyPair());
  }
  void RunInParallel(std::function<void()>, int num_threads = 6);
};

void RSATest::RunInParallel(std::function<void()> f, int num_threads) {
  std::vector<std::future<void>> vec;
  for (int i = 0; i < num_threads; ++i)
    vec.push_back(std::async(std::launch::async, f));
  // wait for all threads to finish
  for (auto &i : vec)
    i.get();
}

TEST_F(RSATest, BEH_RsaKeyPair) {
  auto f([&] {
    EXPECT_TRUE(ValidateKey(keys_.public_key));
    EXPECT_TRUE(ValidateKey(keys_.private_key));
    std::string encoded_private_key(EncodePrivateKey(keys_.private_key));
    std::string encoded_public_key(EncodePublicKey(keys_.public_key));
    PrivateKey private_key(DecodePrivateKey(encoded_private_key));
    PublicKey public_key(DecodePublicKey(encoded_public_key));
    EXPECT_TRUE(CheckRoundtrip(public_key, keys_.private_key));
    EXPECT_TRUE(CheckRoundtrip(public_key, private_key));
  });
  RunInParallel(f, 100);
}

TEST_F(RSATest, BEH_ValidateKeys) {
  auto f([&] {
    PublicKey public_key;
    EXPECT_FALSE(ValidateKey(public_key));
    EXPECT_FALSE(ValidateKey(public_key, 0));
    EXPECT_ANY_THROW(DecodePublicKey("Just some string"));

    PrivateKey private_key;
    EXPECT_FALSE(ValidateKey(private_key));
    EXPECT_FALSE(ValidateKey(private_key, 0));
    EXPECT_ANY_THROW(DecodePrivateKey("Just some string"));

    Keys keys;
    EXPECT_NO_THROW(keys = GenerateKeyPair());
    public_key = keys.public_key;
    private_key = keys.private_key;
    EXPECT_TRUE(ValidateKey(private_key));
    EXPECT_TRUE(ValidateKey(public_key));
  });
  RunInParallel(f);
}

TEST_F(RSATest, BEH_AsymEncryptDecrypt) {
  auto f([&] {
    const crypto::NonEmptyString kSmallData(RandomString(21));
    const crypto::NonEmptyString kLargeData(RandomString(1024 * 1024));
    const Keys empty_keys;
    const Keys other_keys(GenerateKeyPair());
    EXPECT_EQ(kSmallData.string(),
              Decrypt(Encrypt(kSmallData, keys_.public_key), keys_.private_key));
    EXPECT_EQ(kLargeData.string(),
              Decrypt(Encrypt(kLargeData, keys_.public_key), keys_.private_key));
    EXPECT_THROW(Encrypt(kSmallData, empty_keys.public_key), std::exception);
  });
  RunInParallel(f);
}

TEST_F(RSATest, FUNC_SignValidate) {
  auto f([&] {
    EXPECT_NO_THROW(Keys keys(GenerateKeyPair()));
    Keys keys(GenerateKeyPair());
    PrivateKey empty_priv_key;
    PublicKey empty_pub_key;
    const std::string kData(RandomString(RandomUint32() % (1024 * 1024)));

    EXPECT_NO_THROW(std::string signature(Sign(kData, keys.private_key)));
    std::string signature(Sign(kData, keys.private_key));
    EXPECT_TRUE(CheckSignature(kData, signature, keys.public_key));

    std::string empty_data;
    EXPECT_THROW(Sign(empty_data, keys.private_key), std::exception);
    EXPECT_THROW(CheckSignature(empty_data, signature, keys.public_key), std::exception);

    EXPECT_THROW(Sign(kData, empty_priv_key), std::exception);
    EXPECT_THROW(CheckSignature(kData, signature, empty_pub_key), std::exception);

    std::string empty_signature;
    EXPECT_THROW(CheckSignature(kData, empty_signature, keys.public_key), std::exception);

    std::string bad_signature("bad");
    EXPECT_FALSE(CheckSignature(kData, bad_signature, keys.public_key));
  });
  RunInParallel(f, 10);
}
//
// TEST_F(RSATest, BEH_SignFileValidate) {
//   auto f([&] {
//     Keys keys;
//     EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
//     const std::string kData(RandomString(20 * 1024 * 1024));
//     maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestRSA"));
//     std::string file_name("signtest" + RandomAlphaNumericString(5));
//     boost::filesystem::path test_file(*test_path / file_name);
//     EXPECT_TRUE(WriteFile(test_file, kData));
//     ASSERT_FALSE(test_path->empty());
//
//     std::string signature, empty_signature, bad_signature("bad");
//     PrivateKey empty_private_key;
//     EXPECT_EQ(kSuccess, SignFile(test_file, keys.private_key, signature));
//     EXPECT_EQ(kInvalidPrivateKey,
//               SignFile(test_file.string(), empty_private_key, signature));
//     EXPECT_EQ(kRSAEmptyFileError,
//               SignFile(boost::filesystem::path(RandomAlphaNumericString(9)),
//                        keys.private_key,
//                        signature));
//
//     PublicKey empty_public_key;
//     EXPECT_EQ(kSuccess, CheckFileSignature(test_file, signature, keys.public_key));
//     EXPECT_EQ(kInvalidPublicKey,
//               CheckFileSignature(test_file.string(), signature, empty_public_key));
//     EXPECT_EQ(kRSASignatureEmpty,
//               CheckFileSignature(test_file.string(), empty_signature, keys.public_key));
//     EXPECT_EQ(kRSAInvalidSignature,
//               CheckFileSignature(test_file.string(), bad_signature, keys.public_key));
//   });
//   RunInParallel(f, 3);
// }
//
// TEST_F(RSATest, BEH_Serialise) {
//   auto f([] {
//     Keys keys;
//     EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
//     const Keys kOriginalKeys(keys);
//     std::string serialised;
//     EXPECT_TRUE(SerialiseKeys(kOriginalKeys, serialised));
//     Keys recovered_keys;
//     EXPECT_FALSE(ParseKeys("Rubbish", recovered_keys));
//     EXPECT_FALSE(ValidateKey(recovered_keys.private_key));
//
//     EXPECT_TRUE(ParseKeys(serialised, recovered_keys));
//     EXPECT_TRUE(ValidateKey(recovered_keys.public_key));
//     EXPECT_TRUE(ValidateKey(recovered_keys.private_key));
//
//     EXPECT_TRUE(CheckRoundtrip(recovered_keys.public_key, kOriginalKeys.private_key));
//     EXPECT_TRUE(CheckRoundtrip(recovered_keys.public_key, recovered_keys.private_key));
//   });
//   RunInParallel(f, 3);
// }
//
// TEST_F(RSATest, BEH_RsaKeysComparing) {
//   auto f([&] {
//     Keys k1, k2;
//     EXPECT_TRUE(MatchingPublicKeys(k1.public_key, k2.public_key));
//     EXPECT_TRUE(MatchingPrivateKeys(k1.private_key, k2.private_key));
//
//     EXPECT_EQ(kSuccess, GenerateKeyPair(&k1));
//     k2.public_key = k1.public_key;
//     k2.private_key = k1.private_key;
//     EXPECT_TRUE(MatchingPublicKeys(k1.public_key, k2.public_key));
//     EXPECT_TRUE(MatchingPrivateKeys(k1.private_key, k2.private_key));
//   });
//   RunInParallel(f);
// }
//
// TEST_F(RSATest, BEH_RsaKeysSerialisationAndParsing) {
//   auto f([&] {
//     Keys keys;
//     std::string serialised_keys_1, serialised_keys_2;
//     ASSERT_FALSE(SerialiseKeys(keys, serialised_keys_1));
//
//     GenerateKeyPair(&keys);
//     keys.identity = RandomString(64);
//     keys.validation_token = RandomString(128);
//     ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_1));
//     ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_2));
//     ASSERT_EQ(serialised_keys_1, serialised_keys_2);
//
//     keys.identity += keys.identity;
//     ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_1));
//     ASSERT_NE(serialised_keys_1, serialised_keys_2);
//   });
//   RunInParallel(f);
// }

}  // namespace test

}  // namespace rsa

}  // namespace maidsafe
