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

#ifndef MAIDSAFE_COMMON_MENU_LEVEL_H_
#define MAIDSAFE_COMMON_MENU_LEVEL_H_

#include <cstdint>
#include <string>
#include <utility>

namespace maidsafe {

struct MenuLevel {
  MenuLevel(std::string name) : name(name) {}
  ~MenuLevel() = default;

  MenuLevel(const MenuLevel& other) = default;
  MenuLevel(MenuLevel&& other)
      : name(std::move(other.name)) {}

  void swap(MenuLevel& lhs, MenuLevel& rhs) noexcept {
    using std::swap;
    swap(lhs.name, rhs.name);
  }

  MenuLevel& operator=(MenuLevel other) {
    swap (*this, other);
    return *this;
  }

  friend
  bool operator==(const MenuLevel& lhs, const MenuLevel& rhs) {
    return lhs.name == rhs.name;
  }

  friend
  bool operator!=(const MenuLevel& lhs, const MenuLevel& rhs) {
    return !operator==(lhs, rhs);
  }

  std::string name;

};



}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MENU_LEVEL_H_
