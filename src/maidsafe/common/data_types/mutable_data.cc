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

#include "maidsafe/common/data_types/mutable_data.h"

#include <utility>

#include "maidsafe/common/types.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"

#include "maidsafe/common/serialisation.h"

namespace maidsafe {

MutableData::MutableData(const MutableData& other)
    : name_(other.name_), data_(other.data_) {}

MutableData::MutableData(MutableData&& other)
    : name_(std::move(other.name_)),
      data_(std::move(other.data_)) { }

MutableData& MutableData::operator=(MutableData other) {
  swap(*this, other);
  return *this;
}

MutableData::MutableData(Name name, NonEmptyString data)
    : name_(std::move(name)), data_(std::move(data)) {}

MutableData::MutableData(Name name, const serialised_type& serialised_mutable_data)
    : name_(std::move(name)), data_() {
  try { ConvertFromString(serialised_mutable_data->string(), *this); }
  catch(...) { BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error)); }
}

MutableData::serialised_type MutableData::Serialise() const {
  return serialised_type(NonEmptyString(ConvertToString(*this)));
}

MutableData::Name MutableData::name() const { return name_; }

NonEmptyString MutableData::data() const { return data_; }

void swap(MutableData& lhs, MutableData& rhs) {
  using std::swap;
  swap(lhs.name_, rhs.name_);
  swap(lhs.data_, rhs.data_);
}

}  // namespace maidsafe
