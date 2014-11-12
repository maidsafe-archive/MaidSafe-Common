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

#include <atomic>
#include <cstdarg>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "boost/asio/ip/tcp.hpp"
#include "boost/current_function.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/optional.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"

#include "maidsafe/common/active.h"

#ifndef USE_LOGGING
# ifdef NDEBUG
#  define USE_LOGGING 0
# else
#  define USE_LOGGING 1
# endif
#endif

namespace maidsafe {

namespace test { class VisualiserLogTest; }

namespace log {

namespace detail {

  template<typename Left, typename Right>
  class OstreamBinder {
   public:

    OstreamBinder(const Left& left, const Right& right)
      : left_(left),
        right_(right) {
    }

    void Serialise(std::ostream& out) const {
      out << left_ << right_;
    }

   private:

    const Left& left_;
    const Right& right_;
  };

  template<>
  class OstreamBinder<void, void> {
   public:

    void Serialise(std::ostream&) const {}
  };

  template<typename BoundLeft, typename BoundRight, typename Right>
  OstreamBinder<OstreamBinder<BoundLeft, BoundRight>, Right> operator<<(
      const OstreamBinder<BoundLeft, BoundRight>& left, const Right& right) {
    return {left, right};
  }

  template<typename BoundLeft, typename BoundRight>
  std::ostream& operator<<(std::ostream& out, const OstreamBinder<BoundLeft, BoundRight>& binder) {
    binder.Serialise(out);
    return out;
  }

  struct NullStream {
    template<typename Left, typename Right>
    void operator=(const OstreamBinder<Left, Right>&) const {}
  };

  class LogMessage {
   private:
    struct FileInfo {
      const std::string project_;
      const std::string contract_file_;
    };

   public:
    LogMessage(const char* const file, const int level)
      : file_(file),
        level_(level) {
    }

    template<typename BoundLeft, typename BoundRight>
    void operator=(const OstreamBinder<BoundLeft, BoundRight>& binder) const {
      const boost::optional<FileInfo> should_log(ShouldLog());
      if (should_log) {
        std::ostringstream out;
        out << " " << should_log->contract_file_ << binder << "\n";
        Log(should_log->project_, out.str());
      }
    }

  private:
    boost::optional<FileInfo> ShouldLog() const;
    void Log(const std::string& project, std::string message) const;

  private:
    const char* const file_;
    const int level_;
  };
}

// Convert to map<string, int, std::less<>> when C++14 mode is enabled
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


const int kVerbose = -1, kInfo = 0, kSuccess = 1, kWarning = 2, kError = 3, kAlways = 4;

#if USE_LOGGING
#define LOG(level)                                                                  \
  maidsafe::log::detail::LogMessage(__FILE__, maidsafe::log::level) =               \
      maidsafe::log::detail::OstreamBinder<void, void>() << ":" << __LINE__ << "] "
#else
#define LOG(_) maidsafe::log::detail::NullStream() = maidsafe::log::detail::OstreamBinder<void, void>()
#endif
#define TLOG(colour) maidsafe::log::TestLogMessage(maidsafe::log::Colour::colour).MessageStream()

class TestLogMessage {
 public:
  explicit TestLogMessage(Colour colour);
  ~TestLogMessage();
  std::ostringstream& MessageStream() { return stream_; }

 private:
  const Colour kColour_;
  std::ostringstream stream_;
};

class Logging {
 public:
  static Logging& Instance();
  // Returns unused options
  template <typename Char>
  std::vector<std::vector<Char>> Initialise(int argc, Char** argv);
  void InitialiseVlog(const std::string& prefix, const std::string& session_id,
                      const std::string& server_name, uint16_t server_port,
                      const std::string& server_dir);
  void Send(std::function<void()> message_functor);
  void WriteToCombinedLogfile(const std::string& message);
  void WriteToVisualiserLogfile(const std::string& message);
  void WriteToVisualiserServer(const std::string& message);
  void WriteToProjectLogfile(const std::string& project, const std::string& message);
  FilterMap Filter() const { return filter_; }
  bool Async() const { return !no_async_ && background_; }
  bool LogToConsole() const { return !no_log_to_console_; }
  ColourMode Colour() const { return colour_mode_; }
  std::string VlogPrefix() const;
  std::string VlogSessionId() const;
  void Flush();

  friend class test::VisualiserLogTest;

 private:
  struct LogFile {
    LogFile() : stream(), mutex() {}
    std::ofstream stream;
    std::mutex mutex;
  };
  struct Visualiser {
    Visualiser() : prefix("Vault ID uninitialised"), session_id(), logfile(), server_stream(),
                   server_name(), server_dir(), server_port(0), initialised(false),
                   initialised_once_flag() {}
    std::string prefix, session_id;
    LogFile logfile;
    boost::asio::ip::tcp::iostream server_stream;
    std::string server_name, server_dir;
    uint16_t server_port;
    std::atomic<bool> initialised;
    std::once_flag initialised_once_flag;
  };
  Logging();
  bool IsHelpOption(const boost::program_options::options_description& log_config) const;
  void HandleFilterOptions();
  boost::filesystem::path GetLogfileName(const std::string& project) const;
  void SetStreams();
  void WriteToLogfile(const std::string& message, LogFile& log_file);

  boost::program_options::variables_map log_variables_;
  FilterMap filter_;
  bool no_async_, no_log_to_console_;
  std::time_t start_time_;
  boost::filesystem::path log_folder_;
  ColourMode colour_mode_;
  LogFile combined_logfile_stream_;
  std::map<std::string, std::unique_ptr<LogFile>> project_logfile_streams_;
  Visualiser visualiser_;
  std::unique_ptr<Active> background_;
};

namespace detail {

std::string GetLocalTime();
std::string GetUTCTime();

}  // namespace detail

}  // namespace log

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_LOG_H_
