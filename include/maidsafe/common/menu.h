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

/*
  Documentation
  =============

  This is a very simple menu cli tool.

  Example use
  ===========
  auto inc = [&]() { ++test_value; }; // create some void functions you wish to run

  std::string main(std::string("Main")); // create as many levels, use a descriptive name
  MenuItem one("one", inc); // Create items, these will have a function the sig void f();
  MenuItem two("two", inc);
  MenuItem three("three", dec);
  std::string sub(std::string("sub"));
  std::string subsub(std::string("subsub"));

  Menu menu; // give it a name
  menu.AddLevel(main); // now add the levels and items in the order you wish them to appear
  menu.AddItem(one); // this item will be added in the main menu
  menu.AddItem(two); // this item will be added in the main menu
  menu.AddItem(three); // this item will be added in the main menu

  menu.AddLevel(sub, main); // add parent level then this level
  menu.AddItem(one); // this item will be added in the sub menu

  menu.AddLevel(sub, subsub);
  menu.AddItem(three); // this item will be added in the subsub menu
  menu.AddItem(one); // this item will be added in the subsub menu
  menu.AddItem(three);  // this item will be added in the subsub menu
  menu.Start(); // This starts the main loop which runs until the user presses 0.

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

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/common/cli.h"
#include "maidsafe/common/menu_item.h"

namespace maidsafe {

class Menu {
 public:
  explicit Menu(std::string main_menu_name);
  Menu(std::string main_menu_name, std::string prompt);
  Menu(const Menu&) = delete;
  Menu(Menu&&) = delete;
  Menu& operator=(Menu) = delete;
  ~Menu() = default;

  MenuItem* AddItem(std::string name, MenuItem::Functor operation = nullptr);
  int Run();

 private:
  void ShowOptions() const;
  void ExecuteOption();

  MenuItem top_level_item_;
  const MenuItem* current_item_;
  CLI cli_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MENU_H_
