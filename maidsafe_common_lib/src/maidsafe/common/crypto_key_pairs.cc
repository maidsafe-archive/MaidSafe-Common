/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Class that generates in thread RSA key pairs and keeps a buffer
                full
* Version:      1.0
* Created:      2010-03-18-00.23.23
* Revision:     none
* Author:       Jose Cisneros
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

#include "maidsafe/common/crypto_key_pairs.h"

#include <functional>

#include "maidsafe/common/rsa.h"

namespace maidsafe {

CryptoKeyPairs::CryptoKeyPairs(AsioService &asio_service,  // NOLINT (Fraser)
                               const uint16_t &rsa_key_size)
    : asio_service_(asio_service),
      kRsaKeySize_(rsa_key_size < 16 ? 16 : rsa_key_size),
      keypairs_todo_(0),
      keypairs_(),
      mutex_(),
      cond_var_(),
      stopping_(false) {}

CryptoKeyPairs::~CryptoKeyPairs() {
  Stop();
}

void CryptoKeyPairs::CreateKeyPairs(const int16_t &no_of_keypairs) {
  boost::mutex::scoped_lock lock(mutex_);
  stopping_ = false;
  keypairs_todo_ += no_of_keypairs;
  for (int16_t i = 0; i != no_of_keypairs; ++i)
    asio_service_.post(std::bind(&CryptoKeyPairs::CreateKeyPair, this));
}

void CryptoKeyPairs::CreateKeyPair() {
  {
    boost::mutex::scoped_lock lock(mutex_);
    if (stopping_) {
      --keypairs_todo_;
      cond_var_.notify_all();
      return;
    }
  }
  rsa::Keys keys;
  GenerateKeyPair(&keys);
  boost::mutex::scoped_lock lock(mutex_);
  if (!stopping_)
    keypairs_.push_back(keys);
  --keypairs_todo_;
  cond_var_.notify_all();
}

bool CryptoKeyPairs::GetKeyPair(rsa::Keys *keypair) {
  if (!keypair)
    return false;

  boost::mutex::scoped_lock lock(mutex_);
  if (stopping_)
    return false;

  if (keypairs_.empty()) {
    if (keypairs_todo_ == 0)
      return false;
    else
      cond_var_.timed_wait(lock, boost::posix_time::seconds(30),
                           std::bind(&CryptoKeyPairs::KeysReady, this));
  }

  if (keypairs_.empty())
    return false;

  *keypair = keypairs_.front();
  keypairs_.pop_front();
  return true;
}

void CryptoKeyPairs::Stop() {
  boost::mutex::scoped_lock lock(mutex_);
  stopping_ = true;
  keypairs_.clear();
  cond_var_.notify_all();
  cond_var_.timed_wait(lock, boost::posix_time::seconds(30),
                       std::bind(&CryptoKeyPairs::DoneCreatingKeyPairs, this));
}

bool CryptoKeyPairs::KeysReady() {
  return !keypairs_.empty() || keypairs_todo_ == 0;
}

bool CryptoKeyPairs::DoneCreatingKeyPairs() {
  return keypairs_todo_ == 0;
}

}  // namespace maidsafe
