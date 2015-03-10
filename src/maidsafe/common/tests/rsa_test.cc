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
#include "maidsafe/common/serialisation/serialisation.h"

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

TEST_F(RsaTest, BEH_EncryptDecrypt) {
  maidsafe::test::RunInParallel(5, [&] {
    const PlainText small_data(RandomBytes(5, 21));
    const PlainText large_data(RandomBytes(1024 * 128, 1024 * 1024));
    CipherText encrypted_small_data(Encrypt(small_data, keys_.public_key));
    EXPECT_EQ(small_data, Decrypt(encrypted_small_data, keys_.private_key));
    CipherText encrypted_large_data(Encrypt(large_data, keys_.public_key));
    EXPECT_EQ(large_data, Decrypt(encrypted_large_data, keys_.private_key));
  });
}

TEST_F(RsaTest, BEH_SignValidate) {
  maidsafe::test::RunInParallel(5, [&] {
    Keys keys(GenerateKeyPair());
    const PlainText data(RandomBytes(1, 1024 * 1024));
    Signature signature(Sign(data, keys.private_key));
    EXPECT_TRUE(CheckSignature(data, signature, keys.public_key));

    PrivateKey empty_private_key;
    EXPECT_THROW(Sign(data, empty_private_key), asymm_error);
    PublicKey empty_public_key;
    EXPECT_THROW(CheckSignature(data, signature, empty_public_key), asymm_error);

    Signature bad_signature(RandomBytes(Keys::kSignatureByteSize));
    EXPECT_FALSE(CheckSignature(data, bad_signature, keys.public_key));
  });
}

TEST_F(RsaTest, FUNC_SignFileValidate) {
  Keys keys(GenerateKeyPair());
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestRSA"));
  boost::filesystem::path test_file(*test_path / RandomAlphaNumericString(5));
  EXPECT_TRUE(WriteFile(test_file, RandomBytes(19 * 1024 * 1024, 20 * 1024 * 1024)));
  ASSERT_FALSE(test_path->empty());

  Signature bad_signature(RandomBytes(Keys::kSignatureByteSize));
  PrivateKey empty_private_key;
  EXPECT_NO_THROW(SignFile(test_file, keys.private_key));
  Signature signature = SignFile(test_file, keys.private_key);
  EXPECT_THROW(SignFile(test_file.string(), empty_private_key), asymm_error);
  EXPECT_THROW(SignFile(boost::filesystem::path(RandomAlphaNumericString(9)), keys.private_key),
               asymm_error);

  PublicKey empty_public_key;
  EXPECT_TRUE(CheckFileSignature(test_file, signature, keys.public_key));
  EXPECT_THROW(CheckFileSignature(test_file.string(), signature, empty_public_key), asymm_error);
  EXPECT_FALSE(CheckFileSignature(test_file.string(), bad_signature, keys.public_key));
}

TEST_F(RsaTest, BEH_EncodeKeys) {
  Keys keys(GenerateKeyPair());
  EncodedPrivateKey encoded_private_key(EncodeKey(keys.private_key));
  EncodedPublicKey encoded_public_key(EncodeKey(keys.public_key));
  PrivateKey private_key(DecodeKey(encoded_private_key));
  PublicKey public_key(DecodeKey(encoded_public_key));
  EXPECT_THROW(EncodeKey(PrivateKey()), asymm_error);
  EXPECT_THROW(EncodeKey(PublicKey()), asymm_error);
  EXPECT_THROW(DecodeKey(EncodedPrivateKey()), common_error);
  EXPECT_THROW(DecodeKey(EncodedPublicKey()), common_error);

  auto serialised_keys = Serialise(keys);
  auto parsed_keys = Parse<Keys>(serialised_keys);
  EXPECT_TRUE(MatchingKeys(keys.private_key, parsed_keys.private_key));
  EXPECT_TRUE(MatchingKeys(keys.public_key, parsed_keys.public_key));
}

TEST_F(RsaTest, BEH_ValidateKey) {
  Keys keys;
  EXPECT_FALSE(ValidateKey(keys.private_key));
  EXPECT_FALSE(ValidateKey(keys.public_key));
  EXPECT_TRUE(ValidateKey(keys_.private_key));
  EXPECT_TRUE(ValidateKey(keys_.public_key));
}

TEST_F(RsaTest, BEH_RsaKeysComparing) {
  maidsafe::test::RunInParallel(5, [&] {
    Keys k1, k2, k3;
    EXPECT_TRUE(MatchingKeys(k1.public_key, k2.public_key));
    EXPECT_TRUE(MatchingKeys(k1.private_key, k2.private_key));

    k1 = GenerateKeyPair();
    k3 = GenerateKeyPair();
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

}  //  namespace test

}  //  namespace rsa

}  //  namespace maidsafe
