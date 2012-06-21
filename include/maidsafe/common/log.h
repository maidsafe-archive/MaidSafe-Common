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

#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <map>
#include <memory>

#include "boost/current_function.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/active.h"


namespace maidsafe {

namespace log {

typedef std::map<std::string, int> FilterMap;

enum class Colour { kDefaultColour, kRed, kGreen, kYellow };

#ifdef MAIDSAFE_WIN32
class NullStream {
 public:
  NullStream() {}
  template<typename T> NullStream& operator<<(T const&) { return *this; }
  operator bool() const { return false; }
};
#else
// compile away DLOG and LOG statements
class NullStream {
 public:
  NullStream() {}
  template<typename T> NullStream& operator<<(T const&) { return *this; }
  explicit operator bool() const { return false; }
};
#endif

struct Envoid {
 public:
  Envoid() {}
  // This has to be an operator with a precedence lower than << but higher than ?:
  void operator&(NullStream&) {}
};


const int kVerbose = -1, kInfo = 0, kWarning = 1, kError = 2, kFatal = 3;

#ifdef NDEBUG
#  define LOG(_) maidsafe::log::Envoid() & maidsafe::log::NullStream()
#else
#  define LOG(level) maidsafe::log::LogMessage(__FILE__, \
                                               __LINE__, \
                                               BOOST_CURRENT_FUNCTION, \
                                               maidsafe::log::level).messageStream()
#endif
#define TLOG(colour) maidsafe::log::GtestLogMessage(maidsafe::log::Colour::colour).messageStream()

typedef const std::string& LogEntry;

class LogMessage {
 public:
  LogMessage(const std::string &file, int line, const std::string &function, int level);
  ~LogMessage();
  std::ostringstream& messageStream() { return stream_; }
 private:
  const boost::filesystem::path kFile_;
  const int kLine_;
  const std::string kFunction_;
  const int kLevel_;
  std::ostringstream stream_;
};

class GtestLogMessage {
 public:
  explicit GtestLogMessage(Colour colour);
  ~GtestLogMessage();
  std::ostringstream& messageStream() { return stream_; }
 private:
  const Colour kColour_;
  std::ostringstream stream_;
};

class Logging {
 public:
  static Logging& instance() {
    static Logging logging;
    return logging;
  }
  typedef std::function<void()> functor;
  void Send(functor function);
  void SetFilter(FilterMap filter) { filter_ = filter; }
  void AddFilter(std::string project, int level) { filter_[project] = level; }
  FilterMap Filter() const { return filter_; }
  void SetAsync(bool async) { async_ = async; }
  bool Async() const { return async_; }
  void SetColour(bool colour) { colour_ = colour; }
  bool Colour() const { return colour_; }
 private:
  Logging();
  maidsafe::Active background_;
  int log_level_;
  FilterMap filter_;
  bool async_, colour_;
};

}  // namespace log

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_LOG_H_
