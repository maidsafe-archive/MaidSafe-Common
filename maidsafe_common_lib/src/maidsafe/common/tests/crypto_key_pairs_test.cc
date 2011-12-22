/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Test for CryptoKeyPairs class
* Version:      1.0
* Created:      2010-03-15-17.21.51
* Revision:     none
* Compiler:     gcc
* Author:       Fraser Hutchison (fh), fraser.hutchison@maidsafe.net
* Company:      maidsafe.net limited
*
* The following source code is property of maidsafe.net limited and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of maidsafe.net.
*
* ============================================================================
*/

#include <cstdint>
#include "boost/scoped_ptr.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/crypto_key_pairs.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace bptime = boost::posix_time;

namespace maidsafe {

namespace test {

class CryptoKeyPairsTest : public testing::Test {
 public:
  CryptoKeyPairsTest()
      : asio_service_(),
        work_(new boost::asio::io_service::work(asio_service_)),
        kRsaKeySize_(4096),
        threads_(),
        crypto_key_pairs_(asio_service_, kRsaKeySize_) {}

 protected:
  void SetUp() {
    for (int i(0); i != 5; ++i) {
      threads_.create_thread(
          std::bind(static_cast<size_t(boost::asio::io_service::*)()>(
              &boost::asio::io_service::run), &asio_service_));
    }
  }
  void TearDown() {
    work_.reset();
    asio_service_.stop();
    threads_.join_all();
  }
  AsioService asio_service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  const uint16_t kRsaKeySize_;
  boost::thread_group threads_;
  CryptoKeyPairs crypto_key_pairs_;
};

TEST_F(CryptoKeyPairsTest, BEH_GetCryptoKey) {
  rsa::Keys rsa_key_pair;
  ASSERT_FALSE(crypto_key_pairs_.GetKeyPair(&rsa_key_pair));
  crypto_key_pairs_.CreateKeyPairs(1);
  ASSERT_TRUE(crypto_key_pairs_.GetKeyPair(&rsa_key_pair));
  ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
  ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
}

TEST_F(CryptoKeyPairsTest, FUNC_GetMultipleCryptoKeys) {
  int16_t no_of_keys = 20;
  std::vector<rsa::Keys> rsa_key_pairs;
  crypto_key_pairs_.CreateKeyPairs(no_of_keys / 2);
  crypto_key_pairs_.CreateKeyPairs(no_of_keys - (no_of_keys / 2));
  rsa::Keys rsa_key_pair;
  while (crypto_key_pairs_.GetKeyPair(&rsa_key_pair)) {
    rsa_key_pairs.push_back(rsa_key_pair);
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
    rsa_key_pair.identity.clear();
    rsa_key_pair.private_key = rsa::PrivateKey();
    rsa_key_pair.public_key = rsa::PublicKey();
    rsa_key_pair.validation_token.clear();
  }
  ASSERT_EQ(static_cast<size_t>(no_of_keys), rsa_key_pairs.size());
}

TEST_F(CryptoKeyPairsTest, FUNC_ReuseObject) {
  int16_t no_of_keys(5);
  std::vector<rsa::Keys> rsa_key_pairs;
  crypto_key_pairs_.CreateKeyPairs(no_of_keys);

  rsa::Keys rsa_key_pair;
  int16_t i(0), keys_rec(3);
  while (i != keys_rec && crypto_key_pairs_.GetKeyPair(&rsa_key_pair)) {
    rsa_key_pairs.push_back(rsa_key_pair);
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
    rsa_key_pair.identity.clear();
    rsa_key_pair.private_key = rsa::PrivateKey();
    rsa_key_pair.public_key = rsa::PublicKey();
    rsa_key_pair.validation_token.clear();
    ++i;
  }

  crypto_key_pairs_.CreateKeyPairs(no_of_keys);

  while (crypto_key_pairs_.GetKeyPair(&rsa_key_pair)) {
    rsa_key_pairs.push_back(rsa_key_pair);
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
    rsa_key_pair.identity.clear();
    rsa_key_pair.private_key = rsa::PrivateKey();
    rsa_key_pair.public_key = rsa::PublicKey();
    rsa_key_pair.validation_token.clear();
  }
  ASSERT_EQ(static_cast<size_t>(2 * no_of_keys), rsa_key_pairs.size());
}

void GetKeys(CryptoKeyPairs *crypto_key_pairs,
             int *counter,
             const int16_t &total) {
  for (int i = 0; i < total; ++i) {
    rsa::Keys rsa_key_pair;
    EXPECT_TRUE(crypto_key_pairs->GetKeyPair(&rsa_key_pair));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
    ++(*counter);
  }
}

TEST_F(CryptoKeyPairsTest, FUNC_AccessFromDifferentThreads) {
  int16_t no_of_keys(6), no_of_threads(4);
  std::vector<rsa::Keys> rsa_key_pairs;
  crypto_key_pairs_.CreateKeyPairs(no_of_keys * no_of_threads);
  boost::thread_group threads;
  std::vector<int> keys_gen(no_of_threads, 0);
  for (int i = 0; i != no_of_threads; ++i) {
    threads.create_thread(std::bind(&GetKeys, &crypto_key_pairs_, &keys_gen[i],
                                    no_of_keys));
  }
  threads.join_all();
  for (int i = 0; i != no_of_threads; ++i)
    ASSERT_EQ(no_of_keys, keys_gen[i]);
}

TEST_F(CryptoKeyPairsTest, BEH_DestroyObjectWhileGeneratingKeys) {
  boost::scoped_ptr<CryptoKeyPairs> crypto_key_pairs_(
      new CryptoKeyPairs(asio_service_, kRsaKeySize_));
  crypto_key_pairs_->CreateKeyPairs(20);
  Sleep(bptime::seconds(3));
}

void GetKeyPair(std::shared_ptr<CryptoKeyPairs> crypto_key_pairs,
                int *counter) {
  rsa::Keys rsa_key_pair;
  if (crypto_key_pairs->GetKeyPair(&rsa_key_pair)) {
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.public_key));
    ASSERT_TRUE(rsa::ValidateKey(rsa_key_pair.private_key));
    ++(*counter);
  }
}

TEST_F(CryptoKeyPairsTest, BEH_DestroyObjectWhileGettingKeys) {
  boost::thread_group threads;
  int counter(0);
  {
    std::shared_ptr<CryptoKeyPairs> crypto_key_pairs_(
        new CryptoKeyPairs(asio_service_, kRsaKeySize_));
    crypto_key_pairs_->CreateKeyPairs(1);
    for (int i = 0; i < 3; ++i) {
      threads.create_thread(std::bind(&GetKeyPair, crypto_key_pairs_,
                                      &counter));
    }
    Sleep(bptime::seconds(1));
  }
  threads.join_all();
  ASSERT_EQ(1, counter);
}

}  // namespace test

}  // namespace maidsafe
