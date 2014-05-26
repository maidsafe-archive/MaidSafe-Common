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

/*
  Documentation 
  =============  

  This is a very simple menu cli tool. 

  Example use 
  ===========
  auto inc = [&]() { ++test_value; }; // create some void functions you wish to run 
  
  MenuLevel main(MenuLevel("Main")); // create as many levels, use a descriptive name
  MenuItem one("one", inc); // Create items, these will have a function the sig void f();
  MenuItem two("two", inc);
  MenuItem three("three", dec);
  MenuLevel sub(MenuLevel("sub"));
  MenuLevel subsub(MenuLevel("subsub"));

  Menu menu; // give it a name
  menu.add_level(main); // now add the levels and items in the order you wish them to appear
  menu.add_item(one); // this item will be added in the main menu
  menu.add_item(two); // this item will be added in the main menu
  menu.add_item(three); // this item will be added in the main menu

  menu.add_level(sub, main); // add parent level then this level
  menu.add_item(one); // this item will be added in the sub menu
 
  menu.add_level(sub, subsub);
  menu.add_item(three); // this item will be added in the subsub menu
  menu.add_item(one); // this item will be added in the subsub menu
  menu.add_item(three);  // this item will be added in the subsub menu
  menu.start_menu(); // This starts the main loop which runs until the user presses 0.  

  This will create a menu like

  ###################
  Main
  ~~~~~~~~~~~~~~~~~~~
  1: one
  2: two 
  3: three
  4: Sub (this is a sub menu, i.e a level not an item)
  ~~~~~~~~~~~~~~~~~~~~
  Select item or 0 to exit 

  If you selected 4 here then you would have  
  ###################
  Sub
  ~~~~~~~~~~~~~~~~~~~
  1:  one
  2:  SubSub 
  99: Back to Sub
  ~~~~~~~~~~~~~~~~~~~~
  Select item or 0 to exit 

  That's basically it. You can include CLI objects as menu items if you wish. 
  Please see the cli_test.cc for information on testing cli tools automatically.
  This requires redirecting cin and cout etc. 
 */




#ifndef MAIDSAFE_COMMON_MENU_H_
#define MAIDSAFE_COMMON_MENU_H_

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

#include "maidsafe/common/menu_level.h"
#include "maidsafe/common/menu_item.h"
#include "maidsafe/common/cli.h"

namespace maidsafe {

class Menu {
 public:
  Menu() = default;
  ~Menu() = default;

  void add_level(MenuLevel level, MenuLevel parent);
  void add_item(MenuItem item);
  void add_seperator();
  void start_menu();

 private:
  using Option = std::pair<int, std::vector<std::pair<MenuLevel, MenuItem>>::iterator>;
  void Header();
  void ExecuteOption(Option option);
  MenuLevel current_level_;
  MenuLevel parent_level_;
  std::vector<std::pair<MenuLevel, MenuItem>> menus_;
  CLI cli_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MENU_H_
