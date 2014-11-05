/*  Copyright 2014 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_CEREAL_H_
#define MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_CEREAL_H_

#include <cstdint>
#include <vector>

#include "boost/optional.hpp"
#include "maidsafe/common/data_types/version_cereal.h"

namespace maidsafe {

namespace detail {

struct StructuredDataVersionsCereal_Branch {
  StructuredDataVersionsCereal_Branch()
    : absent_parent_ {},
      name_ {}
  { }

  template<typename Archive>
  Archive& serialize(Archive& ref_archive) {
    return ref_archive(absent_parent_, name_);
  }

  boost::optional<VersionCereal> absent_parent_;
  std::vector<VersionCereal> name_;
};

struct StructuredDataVersionsCereal {
  StructuredDataVersionsCereal()
    : max_versions_ {},
      max_branches_ {},
      branch_ {}
  { }

  template<typename Archive>
  Archive& serialize(Archive& ref_archive) {
    return ref_archive(max_versions_, max_branches_, branch_);
  }

  using Branch = StructuredDataVersionsCereal_Branch;

  std::uint32_t max_versions_;
  std::uint32_t max_branches_;
  std::vector<Branch> branch_;
};

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_CEREAL_H_
