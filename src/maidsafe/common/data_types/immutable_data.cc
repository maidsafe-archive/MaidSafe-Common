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

#include "maidsafe/common/data_types/immutable_data.h"

#include <utility>

#include "maidsafe/common/error.h"

namespace maidsafe {
namespace {
const std::uint32_t kTypeId = 0;
}

Data::NameAndTypeId ImmutableData::MakeNameAndTypeId(Identity name) {
  return NameAndTypeId{std::move(name), DataTypeId{kTypeId}};
}

ImmutableData::ImmutableData(NonEmptyString value)
    : Data(crypto::Hash<crypto::SHA512>(value)), value_(std::move(value)) {}

ImmutableData::ImmutableData() = default;

ImmutableData::ImmutableData(const ImmutableData&) = default;

ImmutableData::ImmutableData(ImmutableData&& other) MAIDSAFE_NOEXCEPT
    : Data(std::move(other)), value_(std::move(other.value_)) {}

ImmutableData& ImmutableData::operator=(const ImmutableData&) = default;

ImmutableData& ImmutableData::operator=(ImmutableData&& other) MAIDSAFE_NOEXCEPT{
  Data::operator=(std::move(other));
  value_ = std::move(other.value_);
  return *this;
}

ImmutableData::~ImmutableData() = default;

const NonEmptyString& ImmutableData::Value() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return value_;
}

std::uint32_t ImmutableData::ThisTypeId() const { return kTypeId; }

}  // namespace maidsafe
