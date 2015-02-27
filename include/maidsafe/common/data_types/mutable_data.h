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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_MUTABLE_DATA_H_
#define MAIDSAFE_COMMON_DATA_TYPES_MUTABLE_DATA_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "boost/optional/optional.hpp"
#include "cereal/types/base_class.hpp"
#include "cereal/types/polymorphic.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/data_types/data.h"
#include "maidsafe/common/data_types/data_type_values.h"
// We must include all archives which this polymorphic type will be used with *before* the
// CEREAL_REGISTER_TYPE call below.
#include "maidsafe/common/serialisation/binary_archive.h"

namespace maidsafe {

class MutableData : public Data {
 public:
  using Name = detail::Name<MutableData>;
  using Tag = detail::Tag<DataTagValue::kMutableDataValue>;

  MutableData(Name name, NonEmptyString data);

  MutableData() = default;
  MutableData(const MutableData&) = default;
  MutableData(MutableData&& other);
  MutableData& operator=(const MutableData&) = default;
  MutableData& operator=(MutableData&& other);
  virtual ~MutableData() final = default;

  virtual std::uint32_t TagValue() const final;
  virtual bool Authenticate() const final;
  virtual boost::optional<std::unique_ptr<Data>> Merge(
      const std::vector<std::unique_ptr<Data>>& data_collection) const final;

  template <typename Archive>
  Archive& save(Archive& archive) const {
    if (!Authenticate())
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    return archive(cereal::base_class<Data>(this), name_, data_);
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    return archive(cereal::base_class<Data>(this), name_, data_);
  }

  const Name& name() const { return name_; }
  const NonEmptyString& data() const { return data_; }

 private:
  virtual const Identity& Id() const final;

  Name name_;
  NonEmptyString data_;
};

template <>
struct is_short_term_cacheable<MutableData> : public std::true_type {};

}  // namespace maidsafe

CEREAL_REGISTER_TYPE(maidsafe::MutableData);

#endif  // MAIDSAFE_COMMON_DATA_TYPES_MUTABLE_DATA_H_
