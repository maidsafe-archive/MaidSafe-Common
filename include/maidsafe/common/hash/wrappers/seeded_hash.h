/*  Copyright 2014 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_HASH_WRAPPERS_SEEDED_HASH_H_
#define MAIDSAFE_COMMON_HASH_WRAPPERS_SEEDED_HASH_H_

#include <array>

#include "maidsafe/common/types.h"
#include "maidsafe/common/crypto.h"

namespace maidsafe {

template <typename HashAlgorithm>
class SeededHash {
 public:
  SeededHash() : seed_128bit_() {
    CryptoPP::RandomNumberGenerator& random = maidsafe::crypto::random_number_generator();
    random.GenerateBlock(seed_128bit_.data(), seed_128bit_.size());
  }

  template <typename Type, typename... Types>
  decltype(std::declval<HashAlgorithm>().Finalize()) operator()(Type&& value,
                                                                Types&&... values) const {
    HashAlgorithm hash{seed_128bit_};
    hash(std::forward<Type>(value), std::forward<Types>(values)...);
    return hash.Finalize();
  }

 private:
  std::array<byte, 16> seed_128bit_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_WRAPPERS_SEEDED_HASH_H_
