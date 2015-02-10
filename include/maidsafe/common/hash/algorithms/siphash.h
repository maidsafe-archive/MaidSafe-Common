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
#ifndef MAIDSAFE_COMMON_HASH_ALGORITHMS_SIPHASH_H_
#define MAIDSAFE_COMMON_HASH_ALGORITHMS_SIPHASH_H_

#include <array>
#include <cstdint>

#include "maidsafe/common/config.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/hash/algorithms/hash_algorithm_base.h"

namespace maidsafe {

class SipHash : public detail::HashAlgorithmBase<SipHash> {
  static const std::size_t kKeySize = 16;
 public:
  SipHash(const std::array<byte, kKeySize>& seed) MAIDSAFE_NOEXCEPT;

  void Update(const byte* in, std::uint64_t inlen) MAIDSAFE_NOEXCEPT;

  /* Finalizes the hash, but does not modify internal state. More data can be
     added and then properly finalized later. */
  std::uint64_t Finalize() const MAIDSAFE_NOEXCEPT;

 private:
  unsigned Compress(const byte* in, std::uint64_t inlen) MAIDSAFE_NOEXCEPT;

 private:
  std::uint64_t v0;
  std::uint64_t v1;
  std::uint64_t v2;
  std::uint64_t v3;
  unsigned remainder_length_;
  std::array<byte, 8> remainder_;
  std::uint8_t b;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_ALGORITHMS_SIPHASH_H_
