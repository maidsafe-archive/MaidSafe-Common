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

#include <omp.h>
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace rsa {

namespace test {
  
TEST(RSATest, BEH_RsaKeyPair) {
#pragma omp parallel
  { // NOLINT (dirvine)
    RSA rsa;
    std::shared_ptr<RSAkeys> keys(new RSAkeys);
    EXPECT_TRUE(rsa.GenerateKeyPair(keys));
  }   
}


TEST(RSATest, BEH_AsymEncryptDecrypt) {
  #pragma omp parallel
  { // NOLINT (dirvine)
    // Set up data
    RSA rsa;
    std::shared_ptr<RSAkeys> keys(new RSAkeys);
    std::shared_ptr<RSAkeys> keys_other(new RSAkeys);
    EXPECT_TRUE(rsa.GenerateKeyPair(keys));
    EXPECT_TRUE(rsa.GenerateKeyPair(keys_other));

    const std::string plain_data(RandomString(470));
    std::string recovered_data;
    std::string recovered_data_other;
    const std::string plain_data_other(RandomString(470));
    // Encryption and decryption
    EXPECT_EQ(plain_data,
      rsa.Decrypt(rsa.Encrypt(plain_data, keys->pub_key), keys->priv_key));

    // yes you can encrypt and decrypt with private key
    // but not with a pub key 
    EXPECT_EQ(plain_data,
              rsa.Decrypt(rsa.Encrypt(plain_data, keys->priv_key),
                          keys->priv_key));
    
    EXPECT_NE(plain_data,
      rsa.Decrypt(rsa.Encrypt(plain_data, keys->priv_key),
                  keys_other->priv_key));

    EXPECT_NE(plain_data,
     rsa.Decrypt(rsa.Encrypt(plain_data, keys_other->pub_key), keys->priv_key));

    EXPECT_NE(plain_data,rsa.Encrypt(plain_data, keys_other->pub_key));
    EXPECT_NE(plain_data,rsa.Encrypt(plain_data, keys->pub_key));
  }
}

TEST(RSATest, BEH_SignValidate) {
  #pragma omp parallel
  { // NOLINT (dirvine)
    // Set up data
    RSA rsa;
    std::shared_ptr<RSAkeys> keys(new RSAkeys);
    std::shared_ptr<RSAkeys> keys_other(new RSAkeys);
    std::shared_ptr<RSAkeys> keys_blank(new RSAkeys);
    EXPECT_TRUE(rsa.GenerateKeyPair(keys));
    EXPECT_TRUE(rsa.GenerateKeyPair(keys_other));
    const std::string data(RandomString(470));
    std::shared_ptr<std::string> signature(new std::string);
    EXPECT_TRUE(rsa.Sign(data, signature, keys->priv_key));
    const std::string validsig(*signature);
    EXPECT_TRUE(rsa.CheckSignature(data, validsig , keys->pub_key));
    EXPECT_FALSE(rsa.CheckSignature(data, validsig , keys_other->pub_key));
    EXPECT_FALSE(rsa.Sign(data, signature, keys_blank->priv_key));
    
  }
}

}  // namespace test

}  // namespace crypto

}  // namespace maidsafe
