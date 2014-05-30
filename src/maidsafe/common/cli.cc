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

#include "maidsafe/common/cli.h"

#if defined MAIDSAFE_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <cstdlib>

#include "boost/tokenizer.hpp"
#include "maidsafe/common/crypto.h"

namespace maidsafe {

CLI::CLI(std::string prompt) : kPrompt_(std::move(prompt)) {}

void CLI::Echo(bool enable) const {
#ifdef MAIDSAFE_WIN32
  HANDLE hStdin{ GetStdHandle(STD_INPUT_HANDLE) };
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

  tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

void CLI::Clear() const {
#ifdef MAIDSAFE_WIN32
  COORD top_left = { 0, 0 };
  HANDLE console{ GetStdHandle(STD_OUTPUT_HANDLE) };
  CONSOLE_SCREEN_BUFFER_INFO screen;
  DWORD written;
  GetConsoleScreenBufferInfo(console, &screen);
  FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, top_left, &written);
  FillConsoleOutputAttribute(
      console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
      screen.dwSize.X * screen.dwSize.Y, top_left, &written);
  SetConsoleCursorPosition(console, top_left);
#else
  TLOG(kDefaultColour) << "\x1B[2J\x1B[H";  // CSI[2J to clear, CSI[H to move cursor to top-left
#endif
}

std::string CLI::GetPassword(bool repeat) const {
  std::string password1, password2;
  do {
    password1 = Get<std::string>("Please enter password\n", false);
    if (repeat)
      password2 = Get<std::string>("Please re-enter same password\n", false);
  } while ((password1 != password2) && repeat);
  return crypto::Hash<crypto::SHA512>(password1).string();
}

std::vector<std::string> CLI::TokeniseLine(std::string line) const {
  std::vector<std::string> args;
  line = std::string{ "--" } +line;
  boost::char_separator<char> sep{ " " };
  boost::tokenizer<boost::char_separator<char>> tokens{ line, sep };
  for (const auto& token : tokens)
    args.push_back(token);
  return args;
}

void CLI::Exit() const { exit(0); }

}  // namespace maidsafe
