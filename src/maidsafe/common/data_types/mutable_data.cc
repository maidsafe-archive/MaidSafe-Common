/*  Copyright 2015 MaidSafe.net limited

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
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

MutableData::MutableData(Identity name, NonEmptyString value)
    : Data(std::move(name)), value_(std::move(value)) {
  if (!name_.IsInitialised()) {
    LOG(kWarning) << "Name is uninitialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  if (!value_.IsInitialised()) {
    LOG(kWarning) << "Data is uninitialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
}

MutableData::MutableData() = default;

MutableData::MutableData(const MutableData&) = default;

MutableData::MutableData(MutableData&& other)
    : Data(std::move(other)), value_(std::move(other.value_)) {}

MutableData& MutableData::operator=(const MutableData&) = default;

MutableData& MutableData::operator=(MutableData&& other) {
  Data::operator=(std::move(other));
  value_ = std::move(other.value_);
  return *this;
}

MutableData::~MutableData() = default;

const NonEmptyString& MutableData::Value() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return value_;
}

}  // namespace maidsafe
