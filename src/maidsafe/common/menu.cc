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

#include "maidsafe/common/menu.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include "maidsafe/common/error.h"

namespace maidsafe {

void Menu::add_level(MenuLevel level, MenuLevel parent) {
  if (std::any_of(std::begin(levels_), std::end(levels_),[&] (std::pair<MenuLevel, MenuLevel>& up_one)
                  {
                  return up_one.first == parent;
                  }) && level != parent )
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  levels_.push_back(std::make_pair(level,parent));
}

void Menu::add_item(MenuItem item) {
  menus_.push_back(item);
}

void Menu::start_menu() {
  int result(999);
  int level(0);
  while(result != 0) {
  std::string current = (std::begin(levels_) + level)->first.name; 
  std::cout << current << "\n";
  std::cout << "\n######################################\n";
  for (auto i: levels_) {

    if (result != 0) {
      std::cout << " You selected " << i.first.name;
    }
  }
  result = cli_.Get<int>("\nPlease Enter Option (0 to quit)");
  }
}

}  // namespace maidsafe
