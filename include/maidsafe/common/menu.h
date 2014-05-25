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

#ifndef MAIDSAFE_COMMON_MENU_H_
#define MAIDSAFE_COMMON_MENU_H_

#if defined MAIDSAFE_WIN32
#include <windows.h>
#else
#include <unistd.h>
#if defined MAIDSAFE_LINUX
#include <termio.h>
#elif defined MAIDSAFE_APPLE
#include <termios.h>
#endif
#endif

#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <utility>

#include "maidsafe/common/detail/menu_level.h"
#include "maidsafe/common/detail/menu_item.h"
#include "maidsafe/common/cli.h"

namespace maidsafe {

class Menu {
 public:
  Menu() = default;
  ~Menu() = default;

  void add_level(MenuLevel level);
  void add_item(MenuItem item);
  void start_menu();

 private:
  using Option = std::pair<int, std::vector<std::pair<MenuLevel, MenuItem>>::iterator>;
  void Header();
  void ExecuteOption(Option option);
  MenuLevel current_level_;
  MenuLevel previous_level_;
  std::vector<std::pair<MenuLevel, MenuItem>> menus_;
  CLI cli_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MENU_H_
