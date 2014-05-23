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

#include "maidsafe/common/cli.h"

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
#include <utility>
#include "boost/tokenizer.hpp"
#include "maidsafe/common/crypto.h"

namespace maidsafe {

MenuLevel::MenuLevel(std::string name, std::string description)
      : name(name), description(description) {}

MenuLevel::MenuLevel()
      : name(), description() {}

MenuItem::MenuItem(std::string name, MenuLevel level, Func func)
    : name(name), level(level), target_level(), run(func) {}

MenuItem::MenuItem(std::string name, MenuLevel level, MenuLevel target_level)
    : name(name), level(level), target_level(target_level), run() {}

Menu::Menu()
    : menus_(), levels_(), level_itr_(std::begin(levels_)), cli_() {}

void Menu::add_level(MenuLevel level, MenuLevel parent) {
  levels_.push_back(std::make_pair(level,parent));
}

void Menu::add_item(MenuItem item) {
  menus_.push_back(item);
}

void Menu::start_menu() {
  do{
      std::cout << level_itr_->first.name << "\n";
      std::cout << "######################################\n";
    // for (auto i: levels_) {
      // if (level_itr_->first == i.first) {
      // }
    // }
  } while(cli_.Get<std::string>("\nPlease Enter Option (Q to quit)") != "Q");
}


CLI::CLI(std::string prompt) : kPrompt_(prompt) {}

#if defined MAIDSAFE_WIN32
#pragma warning(push)
#pragma warning(disable : 4701)
#endif
template <class T>
T CLI::Get(std::string display_message, bool echo_input) {
  Echo(echo_input);
  std::cout << display_message << "\n";
  std::cout << kPrompt_ << std::flush;
  T command;
  std::string input;
  while (std::getline(std::cin, input, '\n')) {
    std::cout << kPrompt_ << std::flush;
    if (std::stringstream(input) >> command) {
      Echo(true);
      return command;
    } else {
      Echo(true);
      std::cout << "invalid option\n";
      std::cout << kPrompt_ << std::flush;
    }
  }
  return command;
}
#if defined MAIDSAFE_WIN32
#pragma warning(pop)
#endif


void CLI::Echo(bool enable) {
#ifdef WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);

  if (!enable)
    mode &= ~ENABLE_ECHO_INPUT;
  else
    mode |= ENABLE_ECHO_INPUT;

  SetConsoleMode(hStdin, mode);
#else
  struct termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (!enable)
    tty.c_lflag &= ~ECHO;
  else
    tty.c_lflag |= ECHO;

  (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

std::string CLI::GetPasswd(bool repeat) {
  std::string passwd("r"), passwd2("s");
  do {
    passwd = Get<std::string>("please Enter passwd \n", false);
    if (repeat)
      passwd2 = Get<std::string>("please Re-Enter same passwd \n", false);
  } while ((passwd != passwd2) && (repeat));
  return maidsafe::crypto::Hash<maidsafe::crypto::SHA512>(passwd).string();
}

std::vector<std::string> CLI::TokeniseLine(std::string line) {
  std::vector<std::string> args;
  line = std::string("--") + line;
  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
  for (const auto& t : tokens)  // NOLINT (Fraser)
    args.push_back(t);
  return args;
}

void CLI::Exit() { exit(0); }

}  // namespace maidsafe
