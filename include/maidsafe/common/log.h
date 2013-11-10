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

#ifndef MAIDSAFE_COMMON_LOG_H_
#define MAIDSAFE_COMMON_LOG_H_

#include <cstdarg>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "boost/current_function.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"

#include "maidsafe/common/active.h"

#ifndef NDEBUG
#define USE_LOGGING
#endif

namespace maidsafe {

namespace log {

typedef std::map<std::string, int> FilterMap;

enum class Colour {
  kDefaultColour,
  kRed,
  kGreen,
  kYellow,
  kCyan
};
enum class ColourMode {
  kNone,
  kPartialLine,
  kFullLine
};

#ifdef MAIDSAFE_WIN32
class NullStream {
 public:
  NullStream() {}
  template <typename T>
  NullStream& operator<<(T const&) {
    return *this;
  }
  operator bool() const { return false; }
};
#else
// compile away DLOG and LOG statements
class NullStream {
 public:
  NullStream() {}
  template <typename T>
  NullStream& operator<<(T const&) {
    return *this;
  }
  explicit operator bool() const { return false; }
};
#endif

struct Envoid {
 public:
  Envoid() {}
  // This has to be an operator with a precedence lower than << but higher than ?:
  void operator&(NullStream&) {}
};

const int kVerbose = -1, kInfo = 0, kSuccess = 1, kWarning = 2, kError = 3, kFatal = 4;

#ifdef USE_LOGGING
#define LOG(level)                                                                            \
  maidsafe::log::LogMessage(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION, maidsafe::log::level) \
      .messageStream()
#else
#define LOG(_) maidsafe::log::Envoid() & maidsafe::log::NullStream()
#endif
#define TLOG(colour) maidsafe::log::GtestLogMessage(maidsafe::log::Colour::colour).messageStream()

class LogMessage {
 public:
  LogMessage(std::string file, int line, std::string function, int level);
  ~LogMessage();
  std::ostringstream& messageStream() { return stream_; }

 private:
  std::string file_;
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
  static Logging& Instance();
  // Returns unused options
  std::vector<std::string> Initialise(int argc, char** argv);
  void Send(std::function<void()> message_functor);
  void WriteToCombinedLogfile(const std::string& message);
  void WriteToProjectLogfile(const std::string& project, const std::string& message);
  FilterMap Filter() const { return filter_; }
  bool Async() const { return !no_async_; }
  bool LogToConsole() const { return !no_log_to_console_; }
  ColourMode Colour() const { return colour_mode_; }
  void Flush();

 private:
  struct LogFile {
    LogFile() : stream(), mutex() {}
    std::ofstream stream;
    std::mutex mutex;
  };
  Logging();
  bool IsHelpOption(const boost::program_options::options_description& log_config) const;
  void HandleFilterOptions();
  boost::filesystem::path GetLogfileName(const std::string& project) const;
  void SetStreams();

  boost::program_options::variables_map log_variables_;
  FilterMap filter_;
  bool no_async_, no_log_to_console_;
  std::time_t start_time_;
  boost::filesystem::path log_folder_;
  ColourMode colour_mode_;
  LogFile combined_logfile_stream_;
  std::map<std::string, std::unique_ptr<LogFile>> project_logfile_streams_;
  Active background_;
};

}  // namespace log

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_LOG_H_
