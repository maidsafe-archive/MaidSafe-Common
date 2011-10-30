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
  RSATest()
          : key_one_(), key_two_() {

    GenerateKeyPair(&key_one_);
    GenerateKeyPair(&key_two_);
  }
  
  ~RSATest() {}
 protected:
  RSAKeys key_one_;
  RSAKeys key_two_;
};

TEST(RSAKeGenTest, BEH_RsaKeyPair) {
#pragma omp parallel
  { // NOLINT (dirvine)
    RSAKeys keys;
    EXPECT_EQ(CommonReturnCode::kSuccess,
              GenerateKeyPair(&keys));
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
    const std::string plain_data_other(RandomString(470));
    // Encryption and decryption
    EXPECT_EQ(CommonReturnCode::kSuccess,
             Encrypt(plain_data,
                         key_one_.pub_key,
                         &recovered_data));
    
    EXPECT_NE(plain_data, recovered_data);
    
    EXPECT_EQ(CommonReturnCode::kSuccess,
             Decrypt(recovered_data,
                          key_one_.priv_key,
                          &recovered_data_other));
    EXPECT_EQ(plain_data, recovered_data_other);

   
    EXPECT_EQ(recovered_data_other, plain_data);
    
    EXPECT_NE(recovered_data, plain_data);

    EXPECT_EQ(CommonReturnCode::kDataEmpty,
             Encrypt(empty_data, key_one_.pub_key, &recovered_data));
    
    EXPECT_EQ(CommonReturnCode::kDataEmpty,
             Decrypt(empty_data, key_one_.priv_key, &empty_data));

    EXPECT_EQ(CommonReturnCode::kInvalidPrivateKey,
             Decrypt(plain_data, empty_priv_key, &recovered_data));

    EXPECT_EQ(CommonReturnCode::kInvalidPublicKey,
             Encrypt(plain_data, empty_pub_key, &recovered_data));
  }
}

TEST_F(RSATest, BEH_SignValidate) {
  #pragma omp parallel
  { // NOLINT (dirvine)
    // Set up data
;
    CryptoPP::RSA::PrivateKey empty_priv_key;
    CryptoPP::RSA::PublicKey empty_pub_key;
    std::string empty_data;
    std::string empty_signature("");
    std::string bad_signature("bad");
    const std::string data(RandomString(470));
    std::string signature;
    
    EXPECT_EQ(CommonReturnCode::kSuccess,
             Sign(data, key_one_.priv_key, &signature));
    
    EXPECT_EQ(CommonReturnCode::kSuccess,
             CheckSignature(data, signature, key_one_.pub_key));
    
    EXPECT_EQ(CommonReturnCode::kDataEmpty ,
             Sign(empty_data, key_one_.priv_key, &signature));
    
    EXPECT_EQ(CommonReturnCode::kDataEmpty ,
             CheckSignature(empty_data,
                                       signature,
                                       key_one_.pub_key));
    
    EXPECT_EQ(CommonReturnCode::kInvalidPrivateKey ,
             Sign(data, empty_priv_key, &signature));
    
    EXPECT_EQ(CommonReturnCode::kInvalidPublicKey ,
             CheckSignature(data, signature , empty_pub_key));
    
    EXPECT_EQ(CommonReturnCode::kRSASignatureEmpty ,
             CheckSignature(data, empty_signature , key_one_.pub_key));
    
    EXPECT_EQ(CommonReturnCode::kRSAInvalidsignature ,
             CheckSignature(data, bad_signature , key_one_.pub_key));
  }
}

}  // namespace test
}  // namespace rsa
}  // namespace maidsafe
