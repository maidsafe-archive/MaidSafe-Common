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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_DATA_H_
#define MAIDSAFE_COMMON_DATA_TYPES_DATA_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "boost/optional/optional.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

class Data {
 public:
  Data() = default;
  Data(const Data&) = default;
  Data(Data&&) {}
  Data& operator=(const Data&) = default;
  Data& operator=(Data&&) {}
  virtual ~Data() = default;

  virtual const Identity& Id() const = 0;
  virtual std::uint32_t TagValue() const = 0;
  virtual bool Authenticate() const = 0;
  virtual boost::optional<std::unique_ptr<Data>> Merge(
      const std::vector<std::unique_ptr<Data>>& data_collection) const = 0;

  template <typename Archive>
  Archive& save(Archive& archive) const {
    return archive;
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    return archive;
  }
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_DATA_H_
