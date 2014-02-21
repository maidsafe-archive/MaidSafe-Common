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

#include "maidsafe/common/data_types/immutable_data.h"

#include <utility>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"

namespace maidsafe {

ImmutableData::ImmutableData(const ImmutableData& other) : name_(other.name_), data_(other.data_) {}

ImmutableData::ImmutableData(ImmutableData&& other)
    : name_(std::move(other.name_)), data_(std::move(other.data_)) {}

ImmutableData& ImmutableData::operator=(ImmutableData other) {
  swap(*this, other);
  return *this;
}

ImmutableData::ImmutableData(const NonEmptyString& content)
    : name_(crypto::Hash<crypto::SHA512>(content)), data_(content) {}

ImmutableData::ImmutableData(Name name, serialised_type serialised_immutable_data)
    : name_(std::move(name)), data_(std::move(serialised_immutable_data.data)) {
  Validate();
}

void ImmutableData::Validate() const {
  if (name_.value != crypto::Hash<crypto::SHA512>(data_))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::hashing_error));
}

ImmutableData::serialised_type ImmutableData::Serialise() const { return serialised_type(data_); }

void swap(ImmutableData& lhs, ImmutableData& rhs) {
  using std::swap;
  swap(lhs.name_, rhs.name_);
  swap(lhs.data_, rhs.data_);
}

}  // namespace maidsafe
