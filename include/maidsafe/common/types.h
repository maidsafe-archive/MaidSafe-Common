/*  Copyright 2012 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_TYPES_H_
#define MAIDSAFE_COMMON_TYPES_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/tagged_value.h"

namespace maidsafe {

using NonEmptyString = detail::BoundedString<1>;
using Identity = detail::BoundedString<64, 64>;
using byte = unsigned char;

using MemoryUsage = TaggedValue<uint64_t, struct memory_usage_tag>;
using DiskUsage = TaggedValue<uint64_t, struct disk_usage_tag>;

template <typename T>
struct is_long_term_cacheable : public std::false_type {};

template <typename T>
struct is_short_term_cacheable : public std::false_type {};

template <typename T>
struct is_cacheable : public std::integral_constant<
    bool, is_long_term_cacheable<T>::value || is_short_term_cacheable<T>::value> {};  // NOLINT

template <typename T>
struct is_payable : public std::true_type {};

template <typename T>
struct is_unique_on_network : public std::true_type {};

namespace tcp {

class Connection;
class Listener;

typedef std::shared_ptr<Connection> ConnectionPtr;
typedef std::shared_ptr<Listener> ListenerPtr;
typedef std::function<void(std::string)> MessageReceivedFunctor;
typedef std::function<void()> ConnectionClosedFunctor;
typedef std::function<void(ConnectionPtr)> NewConnectionFunctor;
typedef uint16_t Port;

}  // namespace tcp

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TYPES_H_
