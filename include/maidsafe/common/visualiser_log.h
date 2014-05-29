/*  Copyright 2010 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_VISUALISER_LOG_H_
#define MAIDSAFE_COMMON_VISUALISER_LOG_H_

#include <sstream>
#include <string>
#include <type_traits>

#include "maidsafe/common/log.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"

#define VLOG(persona_enum, action_enum, identity) \
    maidsafe::log::VisualiserLogMessage(persona_enum, action_enum, identity).messageStream()

namespace maidsafe {

namespace log {

class VisualiserLogMessage {
 public:
  template <typename PersonaEnum, typename ActionEnum>
  VisualiserLogMessage(PersonaEnum persona_enum, ActionEnum action_enum, Identity target)
      : stream_(), kSeparator_(',') {
    stream_ << detail::GetUTCTime() << kSeparator_
      << Logging::Instance().VlogPrefix() << kSeparator_
      << static_cast<typename std::underlying_type<PersonaEnum>::type>(persona_enum)
      << kSeparator_ << persona_enum << kSeparator_
      << static_cast<typename std::underlying_type<ActionEnum>::type>(action_enum)
      << kSeparator_ << action_enum << kSeparator_;
    if (target.IsInitialised())
      stream_ << DebugId(target);
    else
      stream_ << "Unknown target";
    stream_ << kSeparator_;
  }
  ~VisualiserLogMessage() {
    std::string log_entry{ stream_.str() + '\n' };
    if (Logging::Instance().LogToConsole()) {
      printf("%s", log_entry.c_str());
      fflush(stdout);
    }
    auto print_functor([log_entry] { Logging::Instance().WriteToVisualiserLogfile(log_entry); });
    Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
  }
  std::ostringstream& messageStream() { return stream_; }

 private:
  std::ostringstream stream_;
  const char kSeparator_;
};

}  // namespace log

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_VISUALISER_LOG_H_
