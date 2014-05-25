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

#ifndef MAIDSAFE_COMMON_CLI_H_
#define MAIDSAFE_COMMON_CLI_H_

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
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <utility>

namespace maidsafe {

typedef std::function<void()> Func;

class CLI {
 public:
  CLI(std::string prompt = ">> ");
  template <typename T>
  T Get(std::string display_message, bool echo_input = true);
  void Echo(bool enable = true);
  std::vector<std::string> TokeniseLine(std::string line);
  void Exit();
  std::string GetPasswd(bool repeat = true);

 private:
  std::string kPrompt_;
};

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

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CLI_H_
