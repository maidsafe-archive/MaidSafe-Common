/* Copyright (c) 2010 maidsafe.net limited
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


#ifndef MAIDSAFE_COMMON_LOG_H_
#define MAIDSAFE_COMMON_LOG_H_

#include "boost/current_function.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <chrono>

class LogWorker;
// compile away DLOG and LOG statements
class NullStream {
    public:
    NullStream() { }
    template<typename T> NullStream& operator<<(T const&) { return *this; }
};


const int INFO = 1, WARNING = 2, ERROR = 3, FATAL = 4;

#define G2_LOG_INFO  g2::internal::LogMessage(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION,"INFO")
#define G2_LOG_WARNING  g2::internal::LogMessage(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION,"WARNING")
#define G2_LOG_ERROR  g2::internal::LogMessage(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION,"ERROR")
#define G2_LOG_FATAL  g2::internal::LogMessage(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION,"FATAL")

#ifdef DEBUG
#define LOG(level) G2_LOG_##level.messageStream()
#define DLOG(level) G2_LOG_##level.messageStream()
#else
#define LOG(level) NullStream()
#define DLOG(level) NullStream()
#endif

namespace maidsafe {
  
namespace log {

void initializeLogging(LogWorker *logger);

namespace internal {
typedef const std::string& LogEntry;

class LogMessage {
 public:
  LogMessage(const std::string &file, const int line, const std::string& function_, const std::string &level);
  virtual ~LogMessage(); // at destruction will flush the message
  std::ostringstream& messageStream(){return stream_;}
  void messageSave(const char *printf_like_message, ...);
 protected:
  const std::string file_;
  const int line_;
  const std::string function_;
  const std::string level_;
  std::ostringstream stream_;
  std::string log_entry_;
};

}  // internal
}  // log
}  // maidsafe

#endif  // MAIDSAFE_COMMON_LOG_H_
