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

#ifndef MAIDSAFE_COMMON_CLI_H_
#define MAIDSAFE_COMMON_CLI_H_

#include <sstream>
#include <string>
#include <vector>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"

namespace maidsafe {

class CLI {
 public:
  explicit CLI(std::string prompt = ">> ");
  CLI(const CLI&) = delete;
  CLI(CLI&&) = delete;
  CLI& operator=(CLI) = delete;

  template <typename T>
  T Get(std::string display_message, bool echo_input = true) const;
  void Echo(bool enable = true) const;
  void Clear() const;
  std::string GetPassword(bool repeat = true) const;
  std::vector<std::string> TokeniseLine(std::string line) const;
  void Exit() const;

 private:
  const std::string kPrompt_;
};

template <class T>
T CLI::Get(std::string display_message, bool echo_input) const {
  on_scope_exit restore_console{[this] {
    Echo(true);
    TLOG(kDefaultColour) << kPrompt_;
  }};
  Echo(echo_input);
  TLOG(kDefaultColour) << std::move(display_message) << '\n' << kPrompt_;
  T command{};
  std::string input;
  if (!std::getline(std::cin, input))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));

  if (!(std::stringstream(input) >> command))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));

  return command;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CLI_H_
