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

#ifndef MAIDSAFE_COMMON_HASH_HASH_DATA_RANGE_H_
#define MAIDSAFE_COMMON_HASH_HASH_DATA_RANGE_H_

#include <type_traits>

#include "maidsafe/common/hash/hash_contiguous.h"
#include "maidsafe/common/hash/hash_range.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

/*
  Types that are a hashable data range must opt-in to the hashing
  implementation. Overload the trait
  template<> IsHashableDataRange<YourType> : std::true_type {};. The type must
  have a .data() function and a .size() function. If the .data() function
  does not return a pointer type, then this optimization is never used.

  If the data() function returns a pointer to a type that can be hashed over,
  then it casts the data() pointer to uint8_t and hashes over the elements
  directly. Otherwise, it falls back to the default HashableRange behavior.
*/
template <typename Type, typename Enable = void>
struct IsHashableDataRange : std::false_type {};

namespace detail {

template <typename HashableRange>
using DataType = decltype(std::declval<HashableRange>().data());

template <typename, typename Enable = void>
struct IsContiguousHashableDataRange : std::false_type {};

// If type.data() returns a pointer to a contiguously hashable array
template <typename HashableDataRange>
struct IsContiguousHashableDataRange<
    HashableDataRange, typename std::enable_if<IsHashableDataRange<HashableDataRange>::value>::type>
    : std::integral_constant<bool, std::is_pointer<DataType<HashableDataRange>>::value &&
                                       IsContiguousHashable<typename std::remove_pointer<
                                           DataType<HashableDataRange>>::type>::value> {};

}  // namespace detail

// If .data() function returns elements that cannot be hashed over,
// fallback to a hashable range
template <typename HashableDataRange>
struct IsHashableRange<HashableDataRange,
                       typename std::enable_if<IsHashableDataRange<HashableDataRange>::value>::type>
    : std::integral_constant<bool,
                             !detail::IsContiguousHashableDataRange<HashableDataRange>::value> {};


// Hashes over .data()
template <typename HashAlgorithm, typename HashableDataRange>
inline
    typename std::enable_if<detail::IsContiguousHashableDataRange<HashableDataRange>::value>::type
    HashAppend(HashAlgorithm& hash, const HashableDataRange& value) {
  using IteratorType = detail::DataType<HashableDataRange>;
  static_assert(std::is_pointer<IteratorType>::value, "expected pointer");
  using DataType = typename std::remove_pointer<IteratorType>::type;

  hash.Update(reinterpret_cast<const byte*>(value.data()), value.size() * sizeof(DataType));
  hash(value.size());
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_HASH_DATA_RANGE_H_
