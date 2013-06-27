/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_TYPES_H_
#define MAIDSAFE_COMMON_TYPES_H_

#include <cstdint>
#include <type_traits>
#include <utility>

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/tagged_value.h"


namespace maidsafe {

typedef detail::BoundedString<1> NonEmptyString;
typedef detail::BoundedString<64, 64> Identity;

typedef TaggedValue<uint64_t, struct MemoryUsageTag> MemoryUsage;
typedef TaggedValue<uint64_t, struct DiskUsageTag> DiskUsage;

template<typename T>
struct is_long_term_cacheable : public std::false_type {};

template<typename T>
struct is_short_term_cacheable : public std::false_type {};

template<typename T>
struct is_cacheable : public std::integral_constant<
    bool,
    is_long_term_cacheable<T>::value || is_short_term_cacheable<T>::value> {};  // NOLINT

template<typename T>
struct is_payable : public std::true_type {};

template<typename T>
struct is_unique_on_network : public std::true_type {};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TYPES_H_
