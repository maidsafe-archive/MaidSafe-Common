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
    keys_ = GenerateKeyPair();
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

TEST_F(RSATest, BEH_RsaEncodeKeys) {
    Keys keys = GenerateKeyPair();
  // auto f([&] {
    EncodedPrivateKey encoded_private_key(EncodeKey(keys.private_key));
    EncodedPublicKey encoded_public_key(EncodeKey(keys.public_key));
    PrivateKey private_key(DecodeKey(encoded_private_key));
    PublicKey public_key(DecodeKey(encoded_public_key));
  // });
  // RunInParallel(f, 100);
}

TEST_F(RSATest, BEH_AsymEncryptDecrypt) {
   auto f([&] {
    const PlainText kSmallData(RandomString(21));
    const PlainText kLargeData(RandomString(1024 * 1024));
    const Keys empty_keys;
    PlainText enc_small_data(Encrypt(kSmallData, keys_.public_key));
    for(int i(0); i < 10 ; ++i) {
    EXPECT_EQ(kSmallData.string(), Decrypt(enc_small_data, keys_.private_key).string());
    // PlainText enc_large_data(Encrypt(kLargeData, keys_.public_key));
    // EXPECT_EQ(kLargeData, Decrypt(enc_large_data, keys_.private_key));
    }
   });
   RunInParallel(f);
}

TEST_F(RSATest, FUNC_SignValidate) {
  auto f([&] {
    EXPECT_NO_THROW(Keys keys(GenerateKeyPair()));
    Keys keys(GenerateKeyPair());
    PrivateKey empty_priv_key;
    PublicKey empty_pub_key;
    const PlainText kData(RandomString(RandomUint32() % (1024 * 1024)));

    EXPECT_NO_THROW(Signature signature(Sign(kData, keys.private_key)));
    Signature signature(Sign(kData, keys.private_key));
    EXPECT_TRUE(CheckSignature(kData, signature, keys.public_key));

    EXPECT_THROW(Sign(kData, empty_priv_key), std::exception);
    EXPECT_THROW(CheckSignature(kData, signature, empty_pub_key), std::exception);

    Signature bad_signature("bad");
    EXPECT_FALSE(CheckSignature(kData, bad_signature, keys.public_key));
  });
  RunInParallel(f, 10);
}

TEST_F(RSATest, FUNC_SignFileValidate) {
  auto f([&] {
    Keys keys;
    EXPECT_NO_THROW(keys = GenerateKeyPair());
    const std::string kData(RandomString(20 * 1024 * 1024));
    maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestRSA"));
    std::string file_name("signtest" + RandomAlphaNumericString(5));
    boost::filesystem::path test_file(*test_path / file_name);
    EXPECT_TRUE(WriteFile(test_file, kData));
    ASSERT_FALSE(test_path->empty());

    Signature bad_signature("bad");
    PrivateKey empty_private_key;
    EXPECT_NO_THROW(SignFile(test_file, keys.private_key));
    Signature signature = SignFile(test_file, keys.private_key);
    EXPECT_THROW(SignFile(test_file.string(), empty_private_key), std::exception);
    EXPECT_THROW(SignFile(boost::filesystem::path(RandomAlphaNumericString(9)),
                       keys.private_key), std::exception);

    PublicKey empty_public_key;
    EXPECT_TRUE(CheckFileSignature(test_file, signature, keys.public_key));
    EXPECT_THROW(CheckFileSignature(test_file.string(), signature, empty_public_key),
                 std::exception);
    EXPECT_FALSE(CheckFileSignature(test_file.string(), bad_signature, keys.public_key));
  });
  RunInParallel(f, 3);
}

TEST_F(RSATest, BEH_RsaKeysComparing) {
  auto f([&] {
    Keys k1, k2;
    EXPECT_TRUE(MatchingKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingKeys(k1.private_key, k2.private_key));

    EXPECT_NO_THROW(k1 = GenerateKeyPair());
    k2.public_key = k1.public_key;
    k2.private_key = k1.private_key;
    EXPECT_TRUE(MatchingKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingKeys(k1.private_key, k2.private_key));
  });
  RunInParallel(f);
}

}  //  namespace test

}  //  namespace rsa

}  //  namespace maidsafe
