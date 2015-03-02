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

ImmutableData::ImmutableData(NonEmptyString data)
    : name_(crypto::Hash<crypto::SHA512>(data)), data_(std::move(data)) {}

ImmutableData::ImmutableData(ImmutableData&& other)
    : name_(std::move(other.name_)), data_(std::move(other.data_)) {}

ImmutableData& ImmutableData::operator=(ImmutableData&& other) {
  name_ = std::move(other.name_);
  data_ = std::move(other.data_);
  return *this;
}

const Identity& ImmutableData::Id() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return name_.value;
}

std::uint32_t ImmutableData::TagValue() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return static_cast<std::uint32_t>(Tag::kValue);
}

boost::optional<std::unique_ptr<Data>> ImmutableData::Merge(
    const std::vector<std::unique_ptr<Data>>& /*data_collection*/) const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return boost::none;
}

bool ImmutableData::IsInitialised() const { return name_->IsInitialised(); }

const ImmutableData::Name& ImmutableData::name() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return name_;
}

const NonEmptyString& ImmutableData::data() const {
  if (!IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return data_;
}

}  // namespace maidsafe
