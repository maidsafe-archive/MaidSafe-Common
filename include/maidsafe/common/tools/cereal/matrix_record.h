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

#ifndef MAIDSAFE_COMMON_TOOLS_CEREAL_MATRIX_RECORD_H_
#define MAIDSAFE_COMMON_TOOLS_CEREAL_MATRIX_RECORD_H_

#include <string>
#include <vector>
#include <cstdint>

namespace maidsafe {

namespace common {

namespace tools {

namespace cereal {

struct MatrixRecord {
  template<typename Archive>
  void serialize(Archive& ref_archive) {
    ref_archive(owner_id_, matrix_ids_);
  }

  struct Element {
    Element() = default;
    Element(const std::string& ref_id, const std::int32_t type) :
      id_(ref_id),
      type_(type)
    { }

    template<typename Archive>
    void serialize(Archive& ref_archive) {
      ref_archive(id_, type_);
    }

    std::string id_;
    std::int32_t type_ = 0;
  };

  std::string owner_id_;
  std::vector<Element> matrix_ids_;
};

}  // namespace cereal

}  // namespace tools

}  // namespace common

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TOOLS_CEREAL_MATRIX_RECORD_H_
