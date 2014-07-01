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

#include "maidsafe/common/rsa.h"

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace rsa {

namespace test {

class RsaTest : public testing::Test {
 public:
  RsaTest() : keys_() {}
  ~RsaTest() {}

 protected:
  Keys keys_;
  void SetUp() {
    ASSERT_NO_THROW(keys_ = GenerateKeyPair());
    keys_ = GenerateKeyPair();
  }
};

TEST_F(RsaTest, BEH_RsaEncodeKeys) {
  Keys keys = GenerateKeyPair();
  maidsafe::test::RunInParallel(100, [&] {
    EncodedPrivateKey encoded_private_key(EncodeKey(keys.private_key));
    EncodedPublicKey encoded_public_key(EncodeKey(keys.public_key));
    PrivateKey private_key(DecodeKey(encoded_private_key));
    PublicKey public_key(DecodeKey(encoded_public_key));
  });
  EXPECT_THROW(EncodeKey(PrivateKey()), std::exception);
  EXPECT_THROW(EncodeKey(PublicKey()), std::exception);
  EXPECT_THROW(DecodeKey(EncodedPrivateKey()), std::exception);
  EXPECT_THROW(DecodeKey(EncodedPublicKey()), std::exception);
}

TEST_F(RsaTest, BEH_AsymEncryptDecrypt) {
  maidsafe::test::RunInParallel(6, [&] {
    const PlainText kSmallData(RandomString(21));
    const PlainText kLargeData(RandomString(1024 * 1024));
    const Keys empty_keys;
    for (int i(0); i < 10; ++i) {
      PlainText enc_small_data(Encrypt(kSmallData, keys_.public_key));
      EXPECT_EQ(kSmallData.string(), Decrypt(enc_small_data, keys_.private_key).string());
      PlainText enc_large_data(Encrypt(kLargeData, keys_.public_key));
      EXPECT_EQ(kLargeData, Decrypt(enc_large_data, keys_.private_key));
    }
  });
}

TEST_F(RsaTest, FUNC_SignValidate) {
  maidsafe::test::RunInParallel(10, [&] {
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

    Signature bad_signature(RandomString(Keys::kSignatureByteSize));
    EXPECT_FALSE(CheckSignature(kData, bad_signature, keys.public_key));
  });
}

TEST_F(RsaTest, FUNC_SignFileValidate) {
  maidsafe::test::RunInParallel(3, [&] {
    Keys keys;
    EXPECT_NO_THROW(keys = GenerateKeyPair());
    const std::string kData(RandomString(20 * 1024 * 1024));
    maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestRSA"));
    std::string file_name("signtest" + RandomAlphaNumericString(5));
    boost::filesystem::path test_file(*test_path / file_name);
    EXPECT_TRUE(WriteFile(test_file, kData));
    ASSERT_FALSE(test_path->empty());

    Signature bad_signature(RandomString(Keys::kSignatureByteSize));
    PrivateKey empty_private_key;
    EXPECT_NO_THROW(SignFile(test_file, keys.private_key));
    Signature signature = SignFile(test_file, keys.private_key);
    EXPECT_THROW(SignFile(test_file.string(), empty_private_key), std::exception);
    EXPECT_THROW(SignFile(boost::filesystem::path(RandomAlphaNumericString(9)), keys.private_key),
                 std::exception);

    PublicKey empty_public_key;
    EXPECT_TRUE(CheckFileSignature(test_file, signature, keys.public_key));
    EXPECT_THROW(CheckFileSignature(test_file.string(), signature, empty_public_key),
                 std::exception);
    EXPECT_FALSE(CheckFileSignature(test_file.string(), bad_signature, keys.public_key));
  });
}

TEST_F(RsaTest, BEH_RsaKeysComparing) {
  maidsafe::test::RunInParallel(6, [&] {
    Keys k1, k2, k3;
    EXPECT_TRUE(MatchingKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingKeys(k1.private_key, k2.private_key));

    EXPECT_NO_THROW(k1 = GenerateKeyPair());
    EXPECT_NO_THROW(k3 = GenerateKeyPair());
    k2.public_key = k1.public_key;
    k2.private_key = k1.private_key;
    EXPECT_TRUE(MatchingKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingKeys(k1.private_key, k2.private_key));
    EXPECT_FALSE(MatchingKeys(k1.public_key, k3.public_key));
    EXPECT_FALSE(MatchingKeys(k1.private_key, k3.private_key));
    EXPECT_FALSE(MatchingKeys(k1.private_key, k3.public_key));
    EXPECT_FALSE(MatchingKeys(k1.public_key, k3.private_key));
  });
}

TEST_F(RsaTest, BEH_SignatureSize) {
  maidsafe::test::RunInParallel(6, [&] {
    Keys k1;
    EXPECT_NO_THROW(k1 = GenerateKeyPair());
    for (int n(0); n != 21; ++n) {
      size_t string_size(static_cast<size_t>(std::pow(2, n)));
      PlainText random_string(RandomString(string_size));
      Signature signature(Sign(random_string, k1.private_key));
      EXPECT_EQ(Keys::kSignatureByteSize, static_cast<int>(signature.string().size()));
    }
  });
}

}  //  namespace test

}  //  namespace rsa

}  //  namespace maidsafe
