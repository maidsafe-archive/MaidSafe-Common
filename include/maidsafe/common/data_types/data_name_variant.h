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

#ifndef MAIDSAFE_DATA_TYPES_DATA_NAME_VARIANT_H_
#define MAIDSAFE_DATA_TYPES_DATA_NAME_VARIANT_H_

#include <utility>

#include "boost/variant/static_visitor.hpp"
#include "boost/variant/variant.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/data_types/data_type_macros.h"
#include "maidsafe/common/data_types/data_type_values.h"
#include "maidsafe/common/data_types/immutable_data.h"
#include "maidsafe/common/data_types/mutable_data.h"

#include "maidsafe/passport/types.h"

namespace maidsafe {

// Defines a typedef of boost::variant<MAIDSAFE_DATA_TYPES> named 'DataNameVariant'.  Also defines a
// function with signature:
//     inline DataNameVariant GetDataNameVariant(DataTagValue type, const Identity& name);
// This throws if 'type' is not a valid DataTagValue.
DEFINE_DATA_NAME_VARIANT

struct GetTagValueVisitor : public boost::static_visitor<DataTagValue> {
  template <typename NameType>
  result_type operator()(const NameType&) const {
    return NameType::data_type::Tag::kValue;
  }
};

struct GetIdentityVisitor : public boost::static_visitor<Identity> {
  template <typename NameType>
  result_type operator()(const NameType& name) const {
    return name.value;
  }
};

struct GetTagValueAndIdentityVisitor
    : public boost::static_visitor<std::pair<DataTagValue, Identity>> {
  template <typename NameType>
  result_type operator()(const NameType& name) const {
    return std::make_pair(NameType::data_type::Tag::kValue, name.value);
  }
};

}  // namespace maidsafe

#endif  // MAIDSAFE_DATA_TYPES_DATA_NAME_VARIANT_H_
