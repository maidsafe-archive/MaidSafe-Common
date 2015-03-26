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

#include "cereal/types/base_class.hpp"
#include "cereal/types/polymorphic.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/identity.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/data_types/data.h"
// We must include all archives which this polymorphic type will be used with *before* the
// CEREAL_REGISTER_TYPE call below.
#include "maidsafe/common/serialisation/binary_archive.h"

namespace maidsafe {

class MutableData : public Data {
 public:
  static NameAndTypeId MakeNameAndTypeId(Identity name);

  MutableData(Identity name, NonEmptyString value);

  MutableData();
  MutableData(const MutableData&);
  MutableData(MutableData&& other) MAIDSAFE_NOEXCEPT;
  MutableData& operator=(const MutableData&);
  MutableData& operator=(MutableData&& other) MAIDSAFE_NOEXCEPT;
  virtual ~MutableData() final;

  const NonEmptyString& Value() const;

  template <typename Archive>
  Archive& save(Archive& archive) const {
    return archive(cereal::base_class<Data>(this), value_);
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    try {
      archive(cereal::base_class<Data>(this), value_);
      if (!value_.IsInitialised())
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    } catch (const std::exception& e) {
      LOG(kWarning) << "Error parsing MutableData: " << boost::diagnostic_information(e);
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    }
    return archive;
  }

 private:
  virtual std::uint32_t ThisTypeId() const final;

  NonEmptyString value_;
};

template <>
struct is_short_term_cacheable<MutableData> : public std::true_type {};

}  // namespace maidsafe

CEREAL_REGISTER_TYPE(maidsafe::MutableData)

#endif  // MAIDSAFE_COMMON_DATA_TYPES_MUTABLE_DATA_H_
