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
#include "boost/filesystem.hpp"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/cli.h"
#include "maidsafe/common/menu.h"
#include "maidsafe/common/test.h"

namespace maidsafe {

void MainMenu() {
  auto inc = [] {};
  MenuLevel main(MenuLevel("Main"));
  MenuItem three(MenuLevel("three"), inc);
  MenuLevel qa("Qa (stress tests, synamic and static analysis");
  MenuLevel dev("Dev (core development help");
  MenuLevel builders("Builders (includes examples)");
  MenuLevel test("Test (check your setup and farming ability)");

  Menu menu;
  menu.add_level(main, main);
  menu.add_item(three);
  menu.add_level(qa, main);
  menu.add_level(dev, main);
  menu.add_level(builders, main);
  menu.add_level(test, main);

  menu.start_menu();
}

}  // namespace maidsafe

int main() {
  maidsafe::MainMenu();
  return 0;
}

