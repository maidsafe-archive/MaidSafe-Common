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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_DATA_TYPE_VALUES_H_
#define MAIDSAFE_COMMON_DATA_TYPES_DATA_TYPE_VALUES_H_

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>

#include "maidsafe/common/config.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/data_types/data_type_macros.h"

namespace maidsafe {

#define MAIDSAFE_DATA_TYPES                                                                     \
  (Anmaid, passport::PublicAnmaid)(Maid, passport::PublicMaid)(Anpmid, passport::PublicAnpmid)( \
      Pmid, passport::PublicPmid)(Anmpid, passport::PublicAnmpid)(Mpid, passport::PublicMpid)(  \
      ImmutableData, ImmutableData)(MutableData, MutableData)

// Defines:
//     enum class DataTagValue : uint32_t { kAnmaid, kMaid, ... };
// Also defines a std::ostream operator<< for the DataTagValue.
// Also defines MAIDSAFE_DATA_TYPES_SIZE which is the number of different data types defined.
DEFINE_MAIDSAFE_DATA_TYPES_ENUM_VALUES(DataTagValue, uint32_t)

namespace detail {

template <typename Parent>
struct Name {
  Name() : value() {}
  explicit Name(Identity value_in) : value(std::move(value_in)) {}
  Name(const Name& other) : value(other.value) {}
  Name(Name&& other) : value(std::move(other.value)) {}
  Name& operator=(Name other);

  operator Identity() const { return value; }
  Identity const* operator->() const { return &value; }
  Identity* operator->() { return &value; }

  template <typename Archive>
  Archive& serialize(Archive& archive) {
    return archive(value);
  }

  Identity value;
  typedef Parent data_type;
};

template <typename Parent>
void swap(Name<Parent>& lhs, Name<Parent>& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.value, rhs.value);
}

template <typename Parent>
Name<Parent>& Name<Parent>::operator=(Name<Parent> other) {
  swap(*this, other);
  return *this;
}

template <typename Parent>
inline bool operator==(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return lhs.value == rhs.value;
}

template <typename Parent>
inline bool operator!=(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return !operator==(lhs, rhs);
}

template <typename Parent>
inline bool operator<(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return lhs.value < rhs.value;
}

template <typename Parent>
inline bool operator>(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return operator<(rhs, lhs);
}

template <typename Parent>
inline bool operator<=(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return !operator>(lhs, rhs);
}

template <typename Parent>
inline bool operator>=(const Name<Parent>& lhs, const Name<Parent>& rhs) {
  return !operator<(lhs, rhs);
}

template <DataTagValue Value>
struct Tag {
  static const DataTagValue kValue = Value;
};

template <DataTagValue Value>
const DataTagValue Tag<Value>::kValue;

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_DATA_TYPE_VALUES_H_
