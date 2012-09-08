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
#include "maidsafe/common/return_codes.h"
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
  void SetUp() {
    ASSERT_EQ(kSuccess, GenerateKeyPair(&keys_));
  }
  Keys keys_;
  void RunInParallel(std::function<void()>, int num_threads = 6);
};

void RSATest::RunInParallel(std::function<void()> f, int num_threads) {
  // TODO(divrine) FIXME
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
    std::string encoded_private_key, encoded_public_key;
    PrivateKey private_key;
    PublicKey public_key;
    EncodePrivateKey(keys_.private_key, &encoded_private_key);
    EncodePublicKey(keys_.public_key, &encoded_public_key);
    DecodePrivateKey(encoded_private_key, &private_key);
    DecodePublicKey(encoded_public_key, &public_key);
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
    DecodePublicKey("Just some string", &public_key);
    EXPECT_FALSE(ValidateKey(public_key));

    PrivateKey private_key;
    EXPECT_FALSE(ValidateKey(private_key));
    EXPECT_FALSE(ValidateKey(private_key, 0));
    DecodePublicKey("Just some string", &private_key);
    EXPECT_FALSE(ValidateKey(private_key));

    Keys keys;
    EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
    public_key = keys.public_key;
    private_key = keys.private_key;
    EXPECT_TRUE(ValidateKey(private_key));
    EXPECT_TRUE(ValidateKey(public_key));
  });
  RunInParallel(f);
}

TEST_F(RSATest, BEH_AsymEncryptDecrypt) {
  auto f([&] {
    const std::string kSmallData(RandomString(21));
    std::string encrypted_data, recovered_data;
    EXPECT_EQ(kSuccess, Encrypt(kSmallData, keys_.public_key, &encrypted_data));
    EXPECT_NE(kSmallData, encrypted_data);
    EXPECT_EQ(kSuccess, Decrypt(encrypted_data, keys_.private_key,
                                &recovered_data));
    EXPECT_EQ(kSmallData, recovered_data);

    const std::string kLargeData(RandomString(1024 * 1024));
    EXPECT_EQ(kSuccess, Encrypt(kLargeData, keys_.public_key, &encrypted_data));
    EXPECT_NE(kLargeData, encrypted_data);
    EXPECT_EQ(kSuccess, Decrypt(encrypted_data, keys_.private_key,
                                &recovered_data));
    EXPECT_EQ(kLargeData, recovered_data);

    EXPECT_EQ(kNullParameter, Encrypt(kLargeData, keys_.public_key, NULL));
    EXPECT_EQ(kNullParameter, Decrypt(encrypted_data, keys_.private_key, NULL));

    recovered_data = "Not empty";
    EXPECT_EQ(kDataEmpty, Encrypt("", keys_.public_key, &recovered_data));
    EXPECT_TRUE(recovered_data.empty());
    recovered_data = "Not empty";
    EXPECT_EQ(kDataEmpty, Decrypt("", keys_.private_key, &recovered_data));
    EXPECT_TRUE(recovered_data.empty());

    recovered_data = "Not empty";
    EXPECT_EQ(kInvalidPublicKey,
              Encrypt(kLargeData, PublicKey(), &recovered_data));
    EXPECT_TRUE(recovered_data.empty());
    recovered_data = "Not empty";
    EXPECT_EQ(kInvalidPrivateKey,
              Decrypt(kLargeData, PrivateKey(), &recovered_data));
    EXPECT_TRUE(recovered_data.empty());
  });
  RunInParallel(f);
}

TEST_F(RSATest, BEH_SignValidate) {
  auto f([&] {
    Keys keys;
    EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
    PrivateKey empty_priv_key;
    PublicKey empty_pub_key;
    const std::string kData(RandomString(RandomUint32() % (1024 * 1024)));
    std::string signature;

    EXPECT_EQ(kSuccess, Sign(kData, keys.private_key, &signature));
    EXPECT_EQ(kSuccess, CheckSignature(kData, signature, keys.public_key));

    std::string empty_data;
    EXPECT_EQ(kDataEmpty, Sign(empty_data, keys.private_key, &signature));
    EXPECT_EQ(kDataEmpty, CheckSignature(empty_data, signature,
                                         keys.public_key));

    EXPECT_EQ(kInvalidPrivateKey, Sign(kData, empty_priv_key, &signature));
    EXPECT_EQ(kInvalidPublicKey,
              CheckSignature(kData, signature, empty_pub_key));

    std::string empty_signature;
    EXPECT_EQ(kRSASignatureEmpty,
              CheckSignature(kData, empty_signature, keys.public_key));

    std::string bad_signature("bad");
    EXPECT_EQ(kRSAInvalidSignature,
              CheckSignature(kData, bad_signature, keys.public_key));
  });
  RunInParallel(f, 10);
}

TEST_F(RSATest, BEH_SignFileValidate) {
  auto f([&] {
    Keys keys;
    EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
    const std::string kData(RandomString(20 * 1024 * 1024));
    maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestRSA"));
    std::string file_name("signtest" + RandomAlphaNumericString(5));
    boost::filesystem::path test_file(*test_path / file_name);
    EXPECT_TRUE(WriteFile(test_file, kData));
    ASSERT_FALSE(test_path->empty());

    std::string signature, empty_signature, bad_signature("bad");
    PrivateKey empty_private_key;
    EXPECT_EQ(kSuccess, SignFile(test_file, keys.private_key, signature));
    EXPECT_EQ(kInvalidPrivateKey,
              SignFile(test_file.string(), empty_private_key, signature));
    EXPECT_EQ(kRSAEmptyFileError,
              SignFile(boost::filesystem::path(RandomAlphaNumericString(9)),
                       keys.private_key,
                       signature));

    PublicKey empty_public_key;
    EXPECT_EQ(kSuccess, CheckFileSignature(test_file, signature, keys.public_key));
    EXPECT_EQ(kInvalidPublicKey,
              CheckFileSignature(test_file.string(), signature, empty_public_key));
    EXPECT_EQ(kRSASignatureEmpty,
              CheckFileSignature(test_file.string(), empty_signature, keys.public_key));
    EXPECT_EQ(kRSAInvalidSignature,
              CheckFileSignature(test_file.string(), bad_signature, keys.public_key));
  });
  RunInParallel(f, 3);
}

TEST_F(RSATest, BEH_Serialise) {
  auto f([] {
    Keys keys;
    EXPECT_EQ(kSuccess, GenerateKeyPair(&keys));
    const Keys kOriginalKeys(keys);
    std::string serialised;
    EXPECT_TRUE(SerialiseKeys(kOriginalKeys, serialised));
    Keys recovered_keys;
    EXPECT_FALSE(ParseKeys("Rubbish", recovered_keys));
    EXPECT_FALSE(ValidateKey(recovered_keys.private_key));

    EXPECT_TRUE(ParseKeys(serialised, recovered_keys));
    EXPECT_TRUE(ValidateKey(recovered_keys.public_key));
    EXPECT_TRUE(ValidateKey(recovered_keys.private_key));

    EXPECT_TRUE(CheckRoundtrip(recovered_keys.public_key, kOriginalKeys.private_key));
    EXPECT_TRUE(CheckRoundtrip(recovered_keys.public_key, recovered_keys.private_key));
  });
  RunInParallel(f, 3);
}

TEST_F(RSATest, BEH_RsaKeysComparing) {
  auto f([&] {
    Keys k1, k2;
    EXPECT_TRUE(MatchingPublicKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingPrivateKeys(k1.private_key, k2.private_key));

    EXPECT_EQ(kSuccess, GenerateKeyPair(&k1));
    k2.public_key = k1.public_key;
    k2.private_key = k1.private_key;
    EXPECT_TRUE(MatchingPublicKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingPrivateKeys(k1.private_key, k2.private_key));
  });
  RunInParallel(f);
}

TEST_F(RSATest, BEH_RsaKeysSerialisationAndParsing) {
  auto f([&] {
    Keys keys;
    std::string serialised_keys_1, serialised_keys_2;
    ASSERT_FALSE(SerialiseKeys(keys, serialised_keys_1));

    GenerateKeyPair(&keys);
    keys.identity = RandomString(64);
    keys.validation_token = RandomString(128);
    ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_1));
    ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_2));
    ASSERT_EQ(serialised_keys_1, serialised_keys_2);

    keys.identity += keys.identity;
    ASSERT_TRUE(SerialiseKeys(keys, serialised_keys_1));
    ASSERT_NE(serialised_keys_1, serialised_keys_2);
  });
  RunInParallel(f);
}

}  // namespace test

}  // namespace rsa

}  // namespace maidsafe
