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
#ifndef MAIDSAFE_COMMON_HASH_HASH_PAIR_H_
#define MAIDSAFE_COMMON_HASH_HASH_PAIR_H_

#include <type_traits>
#include <utility>

#include "maidsafe/common/hash/hash_contiguous.h"

namespace maidsafe {

// If both elements can be "hashed over", and there is no padding
template<typename First, typename Second>
struct IsContiguousHashable<std::pair<First, Second>>
  : std::integral_constant<
      bool,
      IsContiguousHashable<First>::value &&
      IsContiguousHashable<Second>::value &&
      sizeof(std::pair<First, Second>) == sizeof(First) + sizeof(Second)> {};


// This is simple, avoid bringing in cereal headers. Invoked when pair has
// padding or internal types cannot be hashed over directly.
template<typename HashAlgorithm, typename First, typename Second>
typename std::enable_if<!IsContiguousHashable<std::pair<First, Second>>::value>::type HashAppend(
    HashAlgorithm& hash, const std::pair<First, Second>& value) {
  hash(value.first, value.second);
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_HASH_PAIR_H_
