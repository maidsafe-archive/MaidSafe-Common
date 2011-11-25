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

#ifndef MAIDSAFE_COMMON_CRYPTO_KEY_PAIRS_H_
#define MAIDSAFE_COMMON_CRYPTO_KEY_PAIRS_H_

#include <cstdint>
#include <memory>
#include <list>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/thread.hpp"

#include "maidsafe/common/version.h"
#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the maidsafe-passport library.
#endif


namespace maidsafe {

namespace rsa {
struct Keys;
}  // namespace rsa

namespace test { class CachePassport; }

typedef boost::asio::io_service AsioService;

class CryptoKeyPairs {
 public:
  CryptoKeyPairs(AsioService &asio_service,  // NOLINT (Fraser)
                 const uint16_t &rsa_key_size);
  ~CryptoKeyPairs();
  void CreateKeyPairs(const int16_t &no_of_keypairs);
  bool GetKeyPair(rsa::Keys *keypair);
  void Stop();
  friend class test::CachePassport;
 private:
  CryptoKeyPairs &operator=(const CryptoKeyPairs&);
  CryptoKeyPairs(const CryptoKeyPairs&);
  void CreateKeyPair();
  bool KeysReady();
  bool DoneCreatingKeyPairs();
  AsioService &asio_service_;
  const uint16_t kRsaKeySize_;
  int16_t keypairs_todo_;
  std::list<rsa::Keys> keypairs_;
  boost::mutex mutex_;
  boost::condition_variable cond_var_;
  bool stopping_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_KEY_PAIRS_H_
