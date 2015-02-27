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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_
#define MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "boost/optional/optional.hpp"
#include "cereal/types/base_class.hpp"
#include "cereal/types/polymorphic.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/data_types/data.h"
#include "maidsafe/common/data_types/data_type_values.h"
// We must include all archives which this polymorphic type will be used with *before* the
// CEREAL_REGISTER_TYPE call below.
#include "maidsafe/common/serialisation/binary_archive.h"

namespace maidsafe {

class ImmutableData : public Data {
 public:
  using Name = detail::Name<ImmutableData>;
  using Tag = detail::Tag<DataTagValue::kImmutableDataValue>;

  explicit ImmutableData(NonEmptyString data);

  ImmutableData() = default;
  ImmutableData(const ImmutableData&) = default;
  ImmutableData(ImmutableData&& other);
  ImmutableData& operator=(const ImmutableData&) = default;
  ImmutableData& operator=(ImmutableData&& other);
  virtual ~ImmutableData() final = default;

  virtual std::uint32_t TagValue() const final;
  virtual bool Authenticate() const final;
  virtual boost::optional<std::unique_ptr<Data>> Merge(
      const std::vector<std::unique_ptr<Data>>& data_collection) const final;

  template <typename Archive>
  Archive& save(Archive& archive) const {
    if (!Authenticate())
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    return archive(cereal::base_class<Data>(this), data_);
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    archive(cereal::base_class<Data>(this), data_);
    name_ = Name(crypto::Hash<crypto::SHA512>(data_));
    return archive;
  }

  const Name& name() const { return name_; }
  const NonEmptyString& data() const { return data_; }

 private:
  virtual const Identity& Id() const final;

  Name name_;
  NonEmptyString data_;
};

template <>
struct is_long_term_cacheable<ImmutableData> : public std::true_type {};

template <>
struct is_unique_on_network<ImmutableData> : public std::false_type {};

}  // namespace maidsafe

CEREAL_REGISTER_TYPE(maidsafe::ImmutableData);

#endif  // MAIDSAFE_COMMON_DATA_TYPES_IMMUTABLE_DATA_H_
