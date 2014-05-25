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

void Menu::add_level(MenuLevel level) {
  current_level_ = level;
}

void Menu::add_item(MenuItem item) {
  menus_.push_back(std::make_pair(current_level_, item));
}

void Menu::start_menu() {
  int result(999);
  current_level_ = std::begin(menus_)->first;
  while(result != 0) {
  for(int i = 0 ; i < 200; ++i)
    std::cout << "\n";  // very ugly hack
  std::cout << "\n###################################################\n";
  std::cout << "\t" << current_level_.name << "\n";
  std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
// code here
  if(result != 0 && (std::begin(menus_) + result)->second.run != nullptr)
   (std::begin(menus_) + result - 1)->second.run();
  else if (result != 0 && (std::begin(menus_) + result)->second.name != MenuLevel())
    current_level_ =  (std::begin(menus_) + result)->second.name; 

  std::pair<int, MenuItem> option(1, MenuItem());
  for (auto i: menus_) {
    if (i.first.name == current_level_) {
      std::cout << "\n" << option.first << ": \t\t" << i.second.name;
      option = std::make_pair(option.first, i.second);
      ++option.first;
    }
  }
  std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
  result = cli_.Get<int>("\nPlease Enter Option (0 to quit)");
  }
}

}  // namespace maidsafe
