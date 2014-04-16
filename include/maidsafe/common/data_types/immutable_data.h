/*  Copyright 2013 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_
#define MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_

#include <algorithm>

#include "maidsafe/common/types.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/data_types/data_type_values.h"

namespace maidsafe {

class ImmutableData {
 public:
  typedef detail::Name<ImmutableData> Name;
  typedef detail::Tag<DataTagValue::kImmutableDataValue> Tag;
  typedef TaggedValue<NonEmptyString, ImmutableData> serialised_type;

  ImmutableData(const ImmutableData& other);
  ImmutableData(ImmutableData&& other);
  ImmutableData& operator=(ImmutableData other);

  explicit ImmutableData(const NonEmptyString& content);
  ImmutableData(Name name, serialised_type serialised_immutable_data);
  serialised_type Serialise() const;

  Name name() const { return name_; }
  NonEmptyString data() const { return data_; }

  friend void swap(ImmutableData& lhs, ImmutableData& rhs);

 private:
  void Validate() const;
  Name name_;
  NonEmptyString data_;
};

template <>
struct is_long_term_cacheable<ImmutableData> : public std::true_type {};

template <>
struct is_unique_on_network<ImmutableData> : public std::false_type {};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_
