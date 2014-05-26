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

CLI::CLI(std::string prompt) : kPrompt_(prompt) {}

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
    if (repeat) passwd2 = Get<std::string>("please Re-Enter same passwd \n", false);
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
