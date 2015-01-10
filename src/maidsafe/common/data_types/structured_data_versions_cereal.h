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

#include "boost/optional/optional.hpp"
#include "maidsafe/common/data_types/structured_data_versions.h"

namespace maidsafe {

namespace detail {

struct StructuredDataVersionsBranchCereal {
  StructuredDataVersionsBranchCereal() : absent_parent{}, names{} {}

  template <typename Archive>
  Archive& serialize(Archive& archive) {
    return archive(absent_parent, names);
  }

  boost::optional<StructuredDataVersions::VersionName> absent_parent;
  std::vector<StructuredDataVersions::VersionName> names;
};

struct StructuredDataVersionsCereal {
  StructuredDataVersionsCereal() : max_versions{0}, max_branches{0}, branches{} {}

  template <typename Archive>
  Archive& serialize(Archive& archive) {
    return archive(max_versions, max_branches, branches);
  }

  using Branch = StructuredDataVersionsBranchCereal;

  std::uint32_t max_versions;
  std::uint32_t max_branches;
  std::vector<Branch> branches;
};

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_CEREAL_H_
