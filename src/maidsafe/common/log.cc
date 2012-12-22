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

#ifdef MAIDSAFE_WIN32
#  include <Windows.h>
#else
#  include <unistd.h>
#endif
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <thread>

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/make_shared.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/value_semantic.hpp"

#include "maidsafe/common/utils.h"


namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace maidsafe {

namespace log {

namespace {

#ifdef __MSVC__
// This function is needed to avoid use of po::bool_switch causing MSVC warning C4505:
// 'boost::program_options::typed_value<bool>::name' : unreferenced local function has been removed.
void UseUnreferenced() {
  auto dummy = po::typed_value<bool>(nullptr);
  static_cast<void>(dummy);
}
#endif

std::once_flag logging_initialised;
std::mutex g_console_mutex;
const std::array<std::string, 11> kProjects = { { "common",
                                                  "drive",
                                                  "encrypt",
                                                  "lifestuff",
                                                  "lifestuff-gui",
                                                  "nfs",
                                                  "passport",
                                                  "private",
                                                  "routing",
                                                  "rudp",
                                                  "vault"
                                              } };

#ifdef MAIDSAFE_WIN32

WORD GetColourAttribute(Colour colour) {
  switch (colour) {
    case Colour::kRed:
      return FOREGROUND_RED | FOREGROUND_INTENSITY;
    case Colour::kGreen:
      return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case Colour::kYellow:
      return FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    case Colour::kCyan:
      return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    default:
      return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
  }
}

void ColouredPrint(Colour colour, const std::string &text) {
  CONSOLE_SCREEN_BUFFER_INFO console_info_before;
  const HANDLE kConsoleHandle(GetStdHandle(STD_OUTPUT_HANDLE));
  std::lock_guard<std::mutex> lock(g_console_mutex);
  if (kConsoleHandle != INVALID_HANDLE_VALUE) {
    int got_console_info = GetConsoleScreenBufferInfo(kConsoleHandle, &console_info_before);
    fflush(stdout);
    SetConsoleTextAttribute(kConsoleHandle, GetColourAttribute(colour));
    printf("%s", text.c_str());
    fflush(stdout);
    if (got_console_info != 0)
      SetConsoleTextAttribute(kConsoleHandle, console_info_before.wAttributes);
  } else {
    printf("%s", text.c_str());
  }
  fflush(stdout);
}

#else

const char* GetAnsiColourCode(Colour colour) {
  switch (colour) {
    case Colour::kRed:
      return "1";
    case Colour::kGreen:
      return "2";
    case Colour::kYellow:
      return "3";
    case Colour::kCyan:
      return "6";
    default:
      return nullptr;
  }
}

void ColouredPrint(Colour colour, const std::string &text) {
  // On non-Windows platforms, we rely on the TERM variable.
  std::lock_guard<std::mutex> lock(g_console_mutex);
  auto env_ptr = getenv("TERM");
  const std::string kTerm(env_ptr ? env_ptr : "");
  const bool kTermSupportsColour(kTerm == "xterm" ||
                                 kTerm == "xterm-color" ||
                                 kTerm == "xterm-256color" ||
                                 kTerm == "screen" ||
                                 kTerm == "linux" ||
                                 kTerm == "cygwin");
  static const bool in_colour_mode = ((isatty(fileno(stdout)) != 0) && kTermSupportsColour);
  const bool kUseColour = in_colour_mode && (colour != Colour::kDefaultColour);
  if (kUseColour)
    printf("\033[0;3%sm", GetAnsiColourCode(colour));
  printf("%s", text.c_str());
  if (kUseColour)
    printf("\033[m");
  fflush(stdout);
}

#endif

std::string GetProjectAndContractFile(std::string& file) {
  boost::replace_all(file, "\\", "/");
  std::string project;
  size_t position(file.rfind("maidsafe"));
  if (position != std::string::npos) {
    file = file.substr(position + 9);
    position = file.find('/');
    project = file.substr(0, position);
  }
  return project;
}

bool ShouldLog(std::string project, int level) {
  FilterMap filter(Logging::Instance().Filter());
  if (project.empty())
    project = "common";
  auto filter_itr = filter.find(project);
  return (filter_itr != filter.end()) && ((*filter_itr).second <= level);
}

void GetColourAndLevel(char& log_level, Colour& colour, int level) {
  switch (level) {
    case kVerbose:
      log_level = 'V';
      colour = Colour::kCyan;
      break;
    case kInfo:
      log_level = 'I';
      colour = Colour::kDefaultColour;
      break;
    case kSuccess:
      log_level = 'S';
      colour = Colour::kGreen;
      break;
    case kWarning:
      log_level = 'W';
      colour = Colour::kYellow;
      break;
    case kError:
      log_level = 'E';
      colour = Colour::kRed;
      break;
    case kFatal:
      log_level = 'F';
      colour = Colour::kRed;
      break;
    default:
      log_level = ' ';
  }
}

std::string GetColouredLogEntry(char log_level) {
  std::ostringstream oss;
  oss << log_level << " " << std::this_thread::get_id();
#ifdef MAIDSAFE_WIN32
  oss << '\t';
#else
  oss << ' ';
#endif
  oss << boost::posix_time::microsec_clock().universal_time().time_of_day();
  return oss.str();
}

std::string GetPlainLogEntry(const std::string &file, int line, const std::ostringstream& stream) {
  std::ostringstream oss;
  oss << " " << file << ":" << line << "] ";
//  oss << " Function: " << function_ << "] ";
  oss << stream.str() << '\n';
  return oss.str();
}

void SendToConsole(ColourMode colour_mode,
                   Colour colour,
                   const std::string& coloured_log_entry,
                   const std::string& plain_log_entry) {
  if (!Logging::Instance().LogToConsole())
    return;

  if (colour_mode == ColourMode::kFullLine) {
    ColouredPrint(colour, coloured_log_entry + plain_log_entry);
  } else if (colour_mode == ColourMode::kPartialLine) {
    ColouredPrint(colour, coloured_log_entry);
    printf("%s", plain_log_entry.c_str());
  } else {
    printf("%s%s", coloured_log_entry.c_str(), plain_log_entry.c_str());
  }
  fflush(stdout);
}

po::options_description SetProgramOptions(std::string& config_file,
                                          bool& no_log_to_console,
                                          std::string& log_folder,
                                          bool& no_async,
                                          int& colour_mode) {
  fs::path inipath(fs::temp_directory_path() / "maidsafe_log.ini");
  fs::path logpath(fs::temp_directory_path() / "maidsafe_logs");
  po::options_description log_config("Logging Configuration");
  log_config.add_options()
      ("log_no_async", po::bool_switch(&no_async), "Disable asynchronous logging.")
      ("log_colour_mode", po::value<int>(&colour_mode)->default_value(1),
          "0 for no colour, 1 for partial, 2 for full.")
      ("log_config", po::value<std::string>(&config_file)->default_value(inipath.string().c_str()),
          "Path to the logging configuration file.")
      ("log_folder", po::value<std::string>(&log_folder)->default_value(logpath.string().c_str()),
          "Path to folder where log files will be written. If empty, no files will be written.")
      ("log_no_console", po::bool_switch(&no_log_to_console), "Disable logging to console.")
      ("help,h", "Show help message.");
  for (auto project : kProjects) {
    std::string description("Set log level for ");
    description += std::string(project) + " project.";
    std::string name("log_");
    name += project;
    log_config.add(boost::make_shared<po::option_description>(name.c_str(),
                                                              po::value<std::string>(),
                                                              description.c_str()));
  }
  log_config.add_options()("log_*", po::value<std::string>(),
      "Set log level for all projects. Overrides any individual project levels.");
  return log_config;
}

po::variables_map ParseProgramOptions(const po::options_description& log_config,
                                      const std::string& config_file,
                                      int argc,
                                      char **argv) {
  po::options_description cmdline_options;
  cmdline_options.add(log_config);
  po::variables_map log_variables;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).allow_unregistered().
                                                run(), log_variables);
  po::notify(log_variables);

  po::options_description config_file_options;
  config_file_options.add(log_config);
  std::ifstream config_file_stream(config_file.c_str());
  if (config_file_stream) {
    po::store(po::parse_config_file(config_file_stream, config_file_options), log_variables);
    po::notify(log_variables);
  }

  return log_variables;
}

void DoCasts(int col_mode,
             const std::string& log_folder,
             ColourMode& colour_mode,
             fs::path& log_folder_path) {
  if (col_mode != -1) {
    if (col_mode < 0 || col_mode > 2) {
      std::cout << "colour_mode must be 0, 1, or 2\n";
      ThrowError(CommonErrors::invalid_parameter);
    }
    colour_mode = static_cast<ColourMode>(col_mode);
  }
  log_folder_path = log_folder;
}

int GetLogLevel(std::string level) {
  boost::to_lower(level);
  if ((level == "v") || (level == "verbose") || (level == "kverbose") || (level == "-1"))
    return -1;
  if ((level == "i") || (level == "info") || (level == "kinfo") || (level == "0"))
    return 0;
  if ((level == "s") || (level == "success") || (level == "ksuccess") || (level == "1"))
    return 1;
  if ((level == "w") || (level == "warning") || (level == "kwarning") || (level == "2"))
    return 2;
  if ((level == "e") || (level == "error") || (level == "kerror") || (level == "3"))
    return 3;
  if ((level == "f") || (level == "fatal") || (level == "kfatal") || (level == "4"))
    return 4;
  return std::numeric_limits<int>::min();
}

bool SetupLogFolder(const fs::path& log_folder) {
  boost::system::error_code ec;
  if (fs::exists(log_folder, ec)) {
    if (!fs::is_directory(log_folder, ec)) {
      std::cout << "Requested logging folder " << log_folder << " is not a directory.\n";
      return false;
    }
  } else {
    if (!fs::create_directories(log_folder, ec) || ec) {
      std::cout << "Failed to create logging folder: " << ec.message() << '\n';
      return false;
    }
  }
  return true;
}

}  // unnamed namespace

LogMessage::LogMessage(const std::string &file, int line, const std::string &function, int level)
    : file_(file),
      kLine_(line),
      kFunction_(function),
      kLevel_(level),
      stream_() {}

LogMessage::~LogMessage() {
  std::string project(GetProjectAndContractFile(file_));
  if (!ShouldLog(project, kLevel_))
    return;

  char log_level(' ');
  Colour colour(Colour::kDefaultColour);
  GetColourAndLevel(log_level, colour, kLevel_);

  std::string coloured_log_entry(GetColouredLogEntry(log_level));
  std::string plain_log_entry(GetPlainLogEntry(file_, kLine_, stream_));
  ColourMode colour_mode(Logging::Instance().Colour());
  auto print_functor([colour, coloured_log_entry, plain_log_entry, colour_mode, project] {
    SendToConsole(colour_mode, colour, coloured_log_entry, plain_log_entry);
    Logging::Instance().WriteToCombinedLogfile(coloured_log_entry + plain_log_entry);
    Logging::Instance().WriteToProjectLogfile(project, coloured_log_entry + plain_log_entry);
  });

  Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
}

GtestLogMessage::GtestLogMessage(Colour colour) : kColour_(colour), stream_() {}

GtestLogMessage::~GtestLogMessage() {
  Colour colour(kColour_);
  std::string log_entry(stream_.str());
  FilterMap filter(Logging::Instance().Filter());
  auto print_functor([colour, log_entry, filter] {
      if (Logging::Instance().LogToConsole())
        ColouredPrint(colour, log_entry);
      for (auto& entry : filter)
        Logging::Instance().WriteToProjectLogfile(entry.first, log_entry);
      if (filter.size() != 1)
        Logging::Instance().WriteToCombinedLogfile(log_entry);
  });
  Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
}

Logging::Logging()
    : log_variables_(),
      filter_(),
      no_async_(false),
      no_log_to_console_(false),
      start_time_(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())),
      log_folder_(),
      colour_mode_(ColourMode::kPartialLine),
      combined_logfile_stream_(),
      project_logfile_streams_(),
      background_() {}

Logging& Logging::Instance() {
  static Logging logging;
  return logging;
}

void Logging::Initialise(int argc, char **argv) {
  std::call_once(logging_initialised, [this, argc, argv]() {
    try {
      std::string config_file, log_folder;
      int colour_mode(-1);
      po::options_description log_config(SetProgramOptions(config_file, no_log_to_console_,
                                                           log_folder, no_async_, colour_mode));
      log_variables_ = ParseProgramOptions(log_config, config_file, argc, argv);
      if (IsHelpOption(log_config))
        return;
      DoCasts(colour_mode, log_folder, colour_mode_, log_folder_);
      HandleFilterOptions();
      SetStreams();
    }
    catch(const std::exception& e) {
      std::cout << "Exception initialising logging: " << e.what() << "\n\n";
    }
  });
}

bool Logging::IsHelpOption(const po::options_description& log_config) const {
#ifdef USE_LOGGING
  if (log_variables_.count("help")) {
    std::cout << log_config << "Logging levels are as follows:\n"
              << "Verbose(V), Info(I), Success(S), Warning(W), Error(E), Fatal(F)\n\n\n";
    return true;
  }
  return false;
#else
  static_cast<void>(log_config);
  return false;
#endif
}

void Logging::HandleFilterOptions() {
  auto itr(log_variables_.find("log_*"));
  if (itr != log_variables_.end()) {
    filter_.clear();
    for (auto& project : kProjects)
      filter_[project] = GetLogLevel((*itr).second.as<std::string>());
  }
  for (auto& project : kProjects) {
    std::string option("log_" + project);
    itr = log_variables_.find(option);
    if (itr != log_variables_.end())
      filter_[project] = GetLogLevel((*itr).second.as<std::string>());
  }
}

fs::path Logging::GetLogfileName(const std::string& project) const {
  char mbstr[100];
  std::strftime(mbstr, 100, "%Y-%m-%d_%H-%M-%S_", std::localtime(&start_time_));  // NOLINT (Fraser)
  fs::path name(log_folder_ / mbstr);
  name += project + ".log";
  return name;
}

void Logging::SetStreams() {
  if (log_folder_.empty() || !SetupLogFolder(log_folder_))
    return;

  for (auto& entry : filter_) {
    std::unique_ptr<LogFile> log_file(new LogFile);
    log_file->stream.open(GetLogfileName(entry.first).c_str(), std::ios_base::trunc);
    project_logfile_streams_.insert(std::make_pair(entry.first, std::move(log_file)));
  }

  if (filter_.size() != 1)
    combined_logfile_stream_.stream.open(GetLogfileName("combined").c_str(), std::ios_base::trunc);
}

void Logging::Send(std::function<void()> message_functor) {
  background_.Send(message_functor);
}

void Logging::WriteToCombinedLogfile(const std::string& message) {
  std::lock_guard<std::mutex> lock(combined_logfile_stream_.mutex);
  if (combined_logfile_stream_.stream.good())
    combined_logfile_stream_.stream.write(message.c_str(), message.size());
}

void Logging::WriteToProjectLogfile(const std::string& project, const std::string& message) {
  auto itr(project_logfile_streams_.find(project));
  if (itr != project_logfile_streams_.end()) {
    std::lock_guard<std::mutex> lock((*itr).second->mutex);
    if ((*itr).second->stream.good())
      (*itr).second->stream.write(message.c_str(), message.size());
  }
}

}  // namespace log

}  // namespace maidsafe
