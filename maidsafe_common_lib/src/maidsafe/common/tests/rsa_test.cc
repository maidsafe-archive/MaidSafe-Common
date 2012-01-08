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

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/omp.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/return_codes.h"
namespace fs = boost::filesystem;

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
};

TEST_F(RSATest, FUNC_RsaKeyPair) {
#pragma omp parallel
  { // NOLINT (dirvine)
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
  }
}

TEST_F(RSATest, BEH_AsymEncryptDecrypt) {
  #pragma omp parallel
  { // NOLINT (dirvine)
    const std::string plain_data(RandomString(470));
    std::string recovered_data("");
    std::string recovered_data_other("");
    std::string empty_data("");
    CryptoPP::RSA::PrivateKey empty_priv_key;
    CryptoPP::RSA::PublicKey empty_pub_key;
    const std::string plain_data_other(RandomString(47000));
    // Encryption and decryption
    EXPECT_EQ(kSuccess, Encrypt(plain_data, keys_.public_key, &recovered_data));
    EXPECT_NE(plain_data, recovered_data);
    EXPECT_EQ(kSuccess, Decrypt(recovered_data, keys_.private_key,
                                &recovered_data_other));
    EXPECT_EQ(plain_data, recovered_data_other);
    EXPECT_EQ(recovered_data_other, plain_data);
    EXPECT_NE(recovered_data, plain_data);
    EXPECT_EQ(kDataEmpty,
              Encrypt(empty_data, keys_.public_key, &recovered_data));
    EXPECT_EQ(kDataEmpty, Decrypt(empty_data, keys_.private_key, &empty_data));
    EXPECT_EQ(kInvalidPrivateKey,
              Decrypt(plain_data, empty_priv_key, &recovered_data));
    EXPECT_EQ(kInvalidPublicKey,
              Encrypt(plain_data, empty_pub_key, &recovered_data));
  }
}

TEST_F(RSATest, BEH_SignValidate) {
#pragma omp parallel
  {  // NOLINT (dirvine)
    CryptoPP::RSA::PrivateKey empty_priv_key;
    CryptoPP::RSA::PublicKey empty_pub_key;
    const std::string kData(RandomString(470));
    std::string signature;

    EXPECT_EQ(kSuccess, Sign(kData, keys_.private_key, &signature));
    EXPECT_EQ(kSuccess, CheckSignature(kData, signature, keys_.public_key));

    std::string empty_data;
    EXPECT_EQ(kDataEmpty, Sign(empty_data, keys_.private_key, &signature));
    EXPECT_EQ(kDataEmpty, CheckSignature(empty_data, signature,
                                         keys_.public_key));

    EXPECT_EQ(kInvalidPrivateKey, Sign(kData, empty_priv_key, &signature));
    EXPECT_EQ(kInvalidPublicKey,
              CheckSignature(kData, signature, empty_pub_key));

    std::string empty_signature;
    EXPECT_EQ(kRSASignatureEmpty,
              CheckSignature(kData, empty_signature, keys_.public_key));

    std::string bad_signature("bad");
    EXPECT_EQ(kRSAInvalidSignature,
              CheckSignature(kData, bad_signature, keys_.public_key));
  }
}

TEST_F(RSATest, BEH_Serialise) {
  EXPECT_TRUE(ValidateKey(keys_.public_key));
  EXPECT_TRUE(ValidateKey(keys_.private_key));

  std::ostringstream oss;
  boost::archive::text_oarchive output_archive(oss);
  const PrivateKey kOriginalPrivateKey(keys_.private_key);
  const PublicKey kOriginalPublicKey(keys_.public_key);
  output_archive << kOriginalPrivateKey << kOriginalPublicKey;
  std::string encoded(oss.str());

  PrivateKey recovered_private_key;
  PublicKey recovered_public_key;
  std::istringstream iss1(encoded);
  boost::archive::text_iarchive input_archive1(iss1);
  input_archive1 >> recovered_public_key >> recovered_private_key;
  EXPECT_FALSE(ValidateKey(recovered_public_key));
  EXPECT_FALSE(ValidateKey(recovered_private_key));

  try {
    input_archive1 >> recovered_private_key >> recovered_public_key;
  }
  catch(const boost::archive::archive_exception &e) {
    EXPECT_EQ(boost::archive::archive_exception::input_stream_error, e.code);
    EXPECT_FALSE(ValidateKey(recovered_public_key));
    EXPECT_FALSE(ValidateKey(recovered_private_key));
  }

  try {
    std::istringstream iss2(encoded);
    boost::archive::text_iarchive input_archive2(iss2);
    input_archive2 >> recovered_private_key >> recovered_public_key;
  }
  catch(const std::exception &e) {
    FAIL() << e.what();
  }
  EXPECT_TRUE(ValidateKey(recovered_public_key));
  EXPECT_TRUE(ValidateKey(recovered_private_key));

  EXPECT_TRUE(CheckRoundtrip(recovered_public_key, kOriginalPrivateKey));
  EXPECT_TRUE(CheckRoundtrip(recovered_public_key, recovered_private_key));
}

}  // namespace test
}  // namespace rsa
}  // namespace maidsafe
