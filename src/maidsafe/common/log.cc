/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/log.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <cassert>
#include <mutex>
#include <thread>
#include <algorithm>
#include "boost/tokenizer.hpp"
#include "boost/filesystem.hpp"


namespace maidsafe {

namespace log {

LogMessage::LogMessage(const std::string &file,
                       const int line,
                       const std::string& function,
                       const int level)
  : file_(file),
    line_(line),
    function_(function),
    level_(level) {}

LogMessage::~LogMessage() {
  std::ostringstream oss;
  boost::char_separator<char> seperator(", ");
  boost::tokenizer< boost::char_separator<char>> tokens(Logging::instance().Filter(), seperator);

  for (auto &t : tokens) {
    if ((function_.find(t) == std::string::npos) || (Logging::instance().LogLevel() < level_))
      return;
  }

  std::string log_level;
  switch (level_) {
    case 0:
      log_level = "INFO";
      break;
    case 1:
      log_level = "WARNING";
      break;
    case 2:
      log_level = "ERROR";
      break;
    case 3:
      log_level = "FATAL";
      break;
    default:
      log_level = "Unknown";
  }

  oss << log_level << " [" << boost::filesystem3::path(file_).filename().string();
  oss << " :" << line_;
  oss << " Thread : " <<  std::this_thread::get_id();
  oss <<  " Function: " << function_ << "] ";
  const std::string str(stream_.str());
  if (!str.empty())
    oss << '"' << str << '"';
  std::string log_entry(oss.str());
  Logging::instance().Send([=] ()
    { printf("%s \n", log_entry.c_str()); }
  ); //message saved
}

Logging::Logging() :
  background_(maidsafe::Active::createActive()),
  log_levels_(FATAL),  // Default on (fatal
  filter_() {}

void Logging::Send(functor voidfunction) {
  background_->Send(voidfunction);
}


}  // namespace log
}  // namespace maidsafe
