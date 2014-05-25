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
  if (menus_.empty()) current_level_ = level;
  MenuItem item(level.name, level);
  menus_.push_back(std::make_pair(current_level_, item));
  current_level_ = level;
}

void Menu::add_item(MenuItem item) { menus_.push_back(std::make_pair(current_level_, item)); }

void Menu::start_menu() {
  int result(999);
  current_level_ = std::begin(menus_)->first;
  std::vector<Option> current_options;

  while (result != 0) {
    if (result == 99) current_level_ = previous_level_;
    auto found = std::find_if(std::begin(current_options), std::end(current_options),
                              [&](Option& member) { return (member.first == result); });

    if (found != std::end(current_options)) ExecuteOption(*found);
    current_options.clear();
    int counter(0);
    Header();
    for (auto itr = std::begin(menus_); itr < std::end(menus_); ++itr) {
      if (itr->first == current_level_ && itr != std::begin(menus_)) {
        std::cout << "\n" << ++counter << ": \t\t" << itr->second.name;
        current_options.push_back(std::make_pair(counter, itr));
      }
      if (itr->second.target_level == current_level_) previous_level_ = itr->first;
    }
    if (previous_level_ != current_level_)
      std::cout << "\n" << 99 << ": \t\tBack to: " << previous_level_.name;

    std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    result = cli_.Get<int>("\nPlease Enter Option (0 to quit)");
  }
}

void Menu::ExecuteOption(Option option) {
  if (option.second->second.run != nullptr)
    option.second->second.run();
  else if (!option.second->second.name.empty())
    current_level_ = option.second->second.target_level;
}

void Menu::Header() {
  for (int i = 0; i < 200; ++i) std::cout << "\n";  // very ugly hack
  std::cout << "\n###################################################\n";
  std::cout << "\t" << current_level_.name << "\n";
  std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
}

}  // namespace maidsafe
