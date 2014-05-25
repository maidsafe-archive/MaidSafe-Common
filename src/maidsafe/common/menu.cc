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

namespace maidsafe {

void Menu::add_level(MenuLevel level, MenuLevel parent) {
  levels_.push_back(std::make_pair(level,parent));
}

void Menu::add_item(MenuItem item) {
  menus_.push_back(item);
}

void Menu::start_menu() {
  std::string result("");
  level_itr_ = std::begin(levels_);
  std::cout << level_itr_->second.description << "\n";
  std::cout << "\n######################################\n";
  while(result != "Q" &&  result != "q") {
    for (auto i: levels_) {
       if (result == i.first.name) {
          std::cout << " You selected " << i.first.description;
       }
    }
    result = cli_.Get<std::string>("\nPlease Enter Option (Q to quit)");
  }
}

}  // namespace maidsafe
