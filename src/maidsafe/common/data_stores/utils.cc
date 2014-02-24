/*  Copyright 2012 MaidSafe.net limited

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

#include <string>

#include "maidsafe/common/data_stores/utils.h"

#include "boost/variant/apply_visitor.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace data_stores {

namespace detail {

fs::path GetFileName(const DataNameVariant& data_name_variant) {
  auto result(boost::apply_visitor(GetTagValueAndIdentityVisitor(), data_name_variant));
  return (HexEncode(result.second) + '_' + std::to_string(static_cast<uint32_t>(result.first)));
}

DataNameVariant GetDataNameVariant(const fs::path& file_name) {
  std::string file_name_str(file_name.string());
  size_t index(file_name_str.rfind('_'));
  auto id(static_cast<DataTagValue>(std::stoul(file_name_str.substr(index + 1))));
  Identity key_id(HexDecode(file_name_str.substr(0, index)));
  return GetDataNameVariant(id, key_id);
}

}  // namespace detail

}  // namespace data_stores

}  // namespace maidsafe
