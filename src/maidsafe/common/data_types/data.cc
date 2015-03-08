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

#include "maidsafe/common/data_types/data.h"

#include <limits>
#include <utility>

namespace maidsafe {

Data::NameAndTypeId::NameAndTypeId(Identity name_in, DataTypeId type_id_in)
    : name(std::move(name_in)), type_id(type_id_in) {}

Data::NameAndTypeId::NameAndTypeId()
    : name(), type_id(std::numeric_limits<DataTypeId::value_type>::max()) {}

Data::NameAndTypeId::NameAndTypeId(const NameAndTypeId&) = default;

Data::NameAndTypeId::NameAndTypeId(NameAndTypeId&& other)
    : name(std::move(other.name)), type_id(std::move(other.type_id)) MAIDSAFE_NOEXCEPT {}

Data::NameAndTypeId& Data::NameAndTypeId::operator=(const Data::NameAndTypeId&) = default;

Data::NameAndTypeId& Data::NameAndTypeId::operator=(Data::NameAndTypeId&& other) MAIDSAFE_NOEXCEPT {
  name = std::move(other.name);
  type_id = std::move(other.type_id);
  return *this;
}



Data::Data(Identity name) : name_(std::move(name)) {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
}

Data::Data() = default;

Data::Data(const Data&) = default;

Data::Data(Data&& other) : name_(std::move(other.name_)) {}

Data& Data::operator=(const Data&) = default;

Data& Data::operator=(Data&& other) {
  name_ = std::move(other.name_);
  return *this;
}

Data::~Data() = default;

bool Data::IsInitialised() const { return name_.IsInitialised(); }

const Identity& Data::Name() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return name_;
}

DataTypeId Data::TypeId() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return DataTypeId(ThisTypeId());
};

Data::NameAndTypeId Data::NameAndType() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return NameAndTypeId(name_, TypeId());
}

bool operator==(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return std::tie(lhs.name, lhs.type_id) == std::tie(rhs.name, rhs.type_id);
}

bool operator!=(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return !operator==(lhs, rhs);
}

bool operator<(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return std::tie(lhs.name, lhs.type_id) < std::tie(rhs.name, rhs.type_id);
}

bool operator>(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return operator<(rhs, lhs);
}

bool operator<=(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return !operator>(lhs, rhs);
}

bool operator>=(const Data::NameAndTypeId& lhs, const Data::NameAndTypeId& rhs) {
  return !operator<(lhs, rhs);
}

}  // namespace maidsafe
