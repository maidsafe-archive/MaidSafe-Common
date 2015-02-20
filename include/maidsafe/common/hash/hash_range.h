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

#ifndef MAIDSAFE_COMMON_HASH_HASH_RANGE_H_
#define MAIDSAFE_COMMON_HASH_HASH_RANGE_H_

#include <iterator>
#include <type_traits>

#include "maidsafe/common/hash/hash_contiguous.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

/*
  Types that are a hashable range must opt-in to the default hashing
  implementation. Overload the trait
  template<> IsHashableRange<YourType> : std::true_type {};. The type must
  have a std::begin and std::end available for it.

  If a type is marked as a hashable range, it will use std::begin and std::end
  to iterate + hash over the elements, and then will hash the size of the
  iteration so that empty ranges still change the hash value. The following
  optimizations are performed:
    (1) If std::begin returns a pointer to a type that IsContiguousHashable,
        the pointer is cast to uint8_t, and all the elements are hashed in one
        call to the hash function.
    (2) If not (1), but std::begin returns a random access iterator, a count is
        not kept while iterating, and is computed at the end.
    (3) If not (1) or (2), then a count is kept while iterating. This way
        std::list is still a O(n) operation to hash.
*/
template <typename Type, typename Enable = void>
struct IsHashableRange : std::is_array<Type> {};

namespace detail {

template <typename HashableRange>
using IteratorType = decltype(std::begin(std::declval<HashableRange>()));

// Remove qualifiers when looking up the user defined trait "IsHashableRange"
template <typename HashableRange>
using NormalizeRange =
    typename std::remove_cv<typename std::remove_reference<HashableRange>::type>::type;


template <typename, typename Enable = void>
struct IsRandomHashableRange : std::false_type {};

// If type declared as iterable and has a random access iterator
template <typename HashableRange>
struct IsRandomHashableRange<
    HashableRange,
    typename std::enable_if<IsHashableRange<NormalizeRange<HashableRange>>::value>::type>
    : std::is_same<std::random_access_iterator_tag,
                   typename std::iterator_traits<IteratorType<HashableRange>>::iterator_category> {
};


template <typename, typename Enable = void>
struct IsContiguousHashableRange : std::false_type {};

// If type declared as iterable has a pointer iterator and
// contiguous hashable elements
template <typename HashableRange>
struct IsContiguousHashableRange<
    HashableRange, typename std::enable_if<IsRandomHashableRange<HashableRange>::value>::type>
    : std::integral_constant<bool, std::is_pointer<IteratorType<HashableRange>>::value &&
                                       IsContiguousHashable<typename std::remove_pointer<
                                           IteratorType<HashableRange>>::type>::value> {};


// Enable Scenario 3
template <typename HashableRange>
using EnableBasicRange =
    typename std::enable_if<IsHashableRange<NormalizeRange<HashableRange>>::value &&
                            !IsRandomHashableRange<HashableRange>::value>::type;

// Enable Scenario 2
template <typename HashableRange>
using EnableRandomRange =
    typename std::enable_if<IsRandomHashableRange<HashableRange>::value &&
                            !IsContiguousHashableRange<HashableRange>::value>::type;

// Enable Scenario 1
template <typename HashableRange>
using EnableContinousRange =
    typename std::enable_if<IsContiguousHashableRange<HashableRange>::value>::type;

}  // namespace detail


// HashableRange with non-random access iterators
template <typename HashAlgorithm, typename HashableRange>
inline detail::EnableBasicRange<HashableRange> HashAppend(HashAlgorithm& hash,
                                                          HashableRange&& value) {
  std::size_t count = 0;
  for (const auto& one_value : value) {
    hash(one_value);
    ++count;
  }

  // in case iterable is empty, make some noise
  hash(count);
}

// HashableRange with random access iterators, but not continously hashable data
template <typename HashAlgorithm, typename HashableRange>
inline detail::EnableRandomRange<HashableRange> HashAppend(HashAlgorithm& hash,
                                                           HashableRange&& value) {
  for (const auto& one_value : value) {
    hash(one_value);
  }

  // in case iterable is empty, make some noise
  hash(std::distance(std::begin(value), std::end(value)));
}

// HashableRange with data that can be continuously hashed over
template <typename HashAlgorithm, typename HashableRange>
inline detail::EnableContinousRange<HashableRange> HashAppend(HashAlgorithm& hash,
                                                              HashableRange&& value) {
  using IteratorType = detail::IteratorType<HashableRange>;
  static_assert(std::is_pointer<IteratorType>::value, "bug in traits");
  using ContainedType = typename std::remove_pointer<IteratorType>::type;

  const auto size = std::distance(std::begin(value), std::end(value));
  hash.Update(reinterpret_cast<const byte*>(std::begin(value)), size * sizeof(ContainedType));

  // in case iterable is empty, make some noise
  hash(size);
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_HASH_RANGE_H_
