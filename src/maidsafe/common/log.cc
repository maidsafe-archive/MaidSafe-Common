/*  Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/log.h"

#ifdef MAIDSAFE_WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <mutex>
#include <vector>

#ifdef __ANDROID__
#include "android/log.h"
#endif

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/find.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/make_shared.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/value_semantic.hpp"
#include "boost/range/adaptor/replaced.hpp"
#include "boost/range/algorithm/find_first_of.hpp"
#include "boost/utility/string_ref.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace maidsafe {

namespace log {

namespace {

#ifdef _MSC_VER
// This function is needed to avoid use of po::bool_switch causing MSVC warning C4505:
// 'boost::program_options::typed_value<bool>::name' : unreferenced local function has been removed.
void UseUnreferenced() {
  auto dummy = po::typed_value<bool>(nullptr);
  static_cast<void>(dummy);
}
#endif

std::once_flag logging_initialised;

// This fellow needs to work during static data deinit
maidsafe::detail::Spinlock& g_console_mutex() {
  static maidsafe::detail::Spinlock mutex;
  return mutex;
}

const std::array<std::string, 10> kProjects = {{"api", "common", "drive", "encrypt", "nfs",
                                                "passport", "routing", "rudp", "vault",
                                                "vault_manager"}};

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

void ColouredPrint(Colour colour, const std::string& text) {
  CONSOLE_SCREEN_BUFFER_INFO console_info_before;
  const HANDLE kConsoleHandle(GetStdHandle(STD_OUTPUT_HANDLE));
  std::lock_guard<maidsafe::detail::Spinlock> lock(g_console_mutex());
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

void ColouredPrint(Colour colour, const std::string& text) {
  // On non-Windows platforms, we rely on the TERM variable.
  std::lock_guard<maidsafe::detail::Spinlock> lock(g_console_mutex());
  auto env_ptr = std::getenv("TERM");
  const std::string kTerm(env_ptr ? env_ptr : "");
  const bool kTermSupportsColour(kTerm == "xterm" || kTerm == "xterm-color" ||
                                 kTerm == "xterm-256color" || kTerm == "screen" ||
                                 kTerm == "linux" || kTerm == "cygwin");
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

std::pair<boost::string_ref, boost::string_ref> GetProjectAndContractFile(
    const boost::string_ref entire_path) {
  namespace range = boost::range;

  const auto filename_project_separator = boost::find_last(entire_path, "maidsafe");

  if (filename_project_separator.empty()) {
    return {boost::string_ref(), entire_path};
  }

  boost::string_ref filename(filename_project_separator.end(),
                             entire_path.end() - filename_project_separator.end());

  if (filename.empty()) {
    return {boost::string_ref(), entire_path};
  }

  filename.remove_prefix(1);  // remove leading slash

  const auto dir_separator = {'/', '\\'};
  auto project = boost::make_iterator_range(
      std::reverse_iterator<const char*>(filename_project_separator.begin()), entire_path.rend());

  for (unsigned i = 0; i < 2; ++i) {
    project = range::find_first_of<boost::return_next_end>(project, dir_separator);
  }

  project = range::find_first_of<boost::return_begin_found>(project, dir_separator);

  return {boost::string_ref(project.end().base(), project.begin().base() - project.end().base()),
          filename};
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
    case kAlways:
      log_level = 'A';
      colour = Colour::kDefaultColour;
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
  oss << detail::GetUTCTime();
  return oss.str();
}

void SendToConsole(ColourMode colour_mode, Colour colour, int level,
                   const std::string& coloured_log_entry, const std::string& plain_log_entry) {
  if (!Logging::Instance().LogToConsole())
    return;

#ifdef __ANDROID__
  int android_level(0);
  switch (level) {
    case kVerbose:
      android_level = 2;
      break;
    case kInfo:
      android_level = 4;
      break;
    case kSuccess:
      android_level = 3;
      break;
    case kWarning:
      android_level = 5;
      break;
    case kError:
      android_level = 6;
      break;
    case kAlways:
      android_level = 1;
      break;
    default:
      android_level = 0;
  }
  __android_log_print(android_level, "MAIDSAFE_LOG", "%s",
                      (coloured_log_entry.substr(2) + plain_log_entry).c_str());
  static_cast<void>(colour_mode);
  static_cast<void>(colour);
#else
  if (colour_mode == ColourMode::kFullLine) {
    ColouredPrint(colour, coloured_log_entry + plain_log_entry);
  } else if (colour_mode == ColourMode::kPartialLine) {
    ColouredPrint(colour, coloured_log_entry);
    printf("%s", plain_log_entry.c_str());
  } else {
    printf("%s%s", coloured_log_entry.c_str(), plain_log_entry.c_str());
  }
  static_cast<void>(level);
#endif
  fflush(stdout);
}

po::options_description SetProgramOptions(std::string& config_file, bool& no_log_to_console,
                                          std::string& log_folder, bool& no_async,
                                          int& colour_mode) {
#ifdef __ANDROID__
  fs::path inipath;
  fs::path logpath;
#else
  fs::path inipath(fs::temp_directory_path() / "maidsafe_log.ini");
  fs::path logpath(fs::temp_directory_path() / "maidsafe_logs");
#endif
  po::options_description log_config("Logging Configuration");
  log_config.add_options()("log_no_async", po::bool_switch(&no_async),
                           "Disable asynchronous logging.")(
      "log_colour_mode", po::value<int>(&colour_mode)->default_value(1),
      "0 for no colour, 1 for partial, 2 for full.")(
      "log_config", po::value<std::string>(&config_file)->default_value(inipath.string().c_str()),
      "Path to the logging configuration file.")(
      "log_folder", po::value<std::string>(&log_folder)->default_value(logpath.string().c_str()),
      "Path to folder where log files will be written. If empty, no files will be written.")(
      "log_no_console", po::bool_switch(&no_log_to_console),
      "Disable logging to console.")("help,h", "Show help message.");
  for (auto project : kProjects) {
    std::string description("Set log level for ");
    description += std::string(project) + " project.";
    std::string name("log_");
    name += project;
    log_config.add(boost::make_shared<po::option_description>(
        name.c_str(), po::value<std::string>(), description.c_str()));
  }
  log_config.add_options()(
      "log_*", po::value<std::string>(),
      "Set log level for all projects. Overrides any individual project levels.");
  return log_config;
}

template <typename Char>
void ParseProgramOptions(const po::options_description& log_config, const std::string& config_file,
                         int argc, Char** argv, po::variables_map& log_variables,
                         std::vector<std::vector<Char>>& unused_options) {
  po::options_description cmdline_options;
  cmdline_options.add(log_config);
  po::basic_parsed_options<Char> parsed(po::basic_command_line_parser<Char>(argc, argv)
                                            .options(cmdline_options)
                                            .allow_unregistered()
                                            .run());

  po::store(parsed, log_variables);
  po::notify(log_variables);

  po::options_description config_file_options;
  config_file_options.add(log_config);
  std::ifstream config_file_stream(config_file.c_str());
  if (config_file_stream) {
    po::store(po::parse_config_file(config_file_stream, config_file_options), log_variables);
    po::notify(log_variables);
  }

  auto this_exe_path(ThisExecutablePath().string());
  unused_options.emplace_back(this_exe_path.c_str(),
                              this_exe_path.c_str() + this_exe_path.size() + 1u);
  auto unuseds(po::collect_unrecognized(parsed.options, po::include_positional));
  if (log_variables.count("help")) {
    const Char help[] = {'-', '-', 'h', 'e', 'l', 'p', '\0'};
    unuseds.push_back(help);
  }
  for (const auto& unused : unuseds)
    unused_options.emplace_back(unused.c_str(), unused.c_str() + unused.size() + 1u);
}

#if USE_LOGGING
void DoCasts(int col_mode, const std::string& log_folder, ColourMode& colour_mode,
             fs::path& log_folder_path) {
  if (col_mode != -1) {
    if (col_mode < 0 || col_mode > 2) {
      std::cout << "colour_mode must be 0, 1, or 2\n";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
    }
    colour_mode = static_cast<ColourMode>(col_mode);
  }
  log_folder_path = log_folder;
}
#endif

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
  if ((level == "a") || (level == "always") || (level == "kalways") || (level == "4"))
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

enum class TimeType { kLocal, kUTC };

template <TimeType time_type>
std::string Strftime(const std::time_t* now_t);

template <>
std::string Strftime<TimeType::kLocal>(const std::time_t* now_t) {
  std::lock_guard<maidsafe::detail::Spinlock> lock(g_console_mutex());
  char temp[10];
  if (!std::strftime(temp, sizeof(temp), "%H:%M:%S.", std::localtime(now_t)))  // NOLINT (Fraser)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));
  return std::string{temp};
}

template <>
std::string Strftime<TimeType::kUTC>(const std::time_t* now_t) {
  std::lock_guard<maidsafe::detail::Spinlock> lock(g_console_mutex());
  char temp[21];
  if (!std::strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S.",
                     std::gmtime(now_t)))  // NOLINT (Fraser)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));
  return std::string{temp};
}

template <TimeType time_type>
std::string GetTime() {
  auto now(std::chrono::system_clock::now());
  auto seconds_since_epoch(
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()));

  std::time_t now_t(std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::time_point(seconds_since_epoch)));

  return Strftime<time_type>(&now_t) +
         std::to_string((now.time_since_epoch() - seconds_since_epoch).count());
}

}  // unnamed namespace

namespace detail {

// ======================================= LogMessage ==============================================
boost::optional<LogMessage::FileInfo> LogMessage::ShouldLog() const {
  auto file_info(GetProjectAndContractFile(file_));

  const FilterMap filter(Logging::Instance().Filter());
  if (file_info.first.empty()) {
    file_info.first = "common";
  }

  // With C++14, the map can be searched without a temp string
  const std::string project(file_info.first.begin(), file_info.first.end());

  const auto filter_itr = filter.find(project);

  if (filter_itr != filter.end()) {
    if ((*filter_itr).second <= level_) {
      const auto fix_slashes(file_info.second | boost::adaptors::replaced('\\', '/'));
      return FileInfo{std::move(project), std::string(fix_slashes.begin(), fix_slashes.end())};
    }
  }
  return boost::none;
}

void LogMessage::Log(const std::string& project, std::string message) const {
  char log_level(' ');
  Colour colour(Colour::kDefaultColour);
  const int level(level_);
  GetColourAndLevel(log_level, colour, level);
  std::string coloured_log_entry(GetColouredLogEntry(log_level));
  ColourMode colour_mode(Logging::Instance().Colour());
#if defined(__GLIBCXX__)
  //  && __GLIBCXX__ < date (date in format of 20141218 as the date of fix of COW string)
  auto message_ptr(std::make_shared<std::string>(message.data(), message.size()));
  auto coloured_log_entry_ptr(
      std::make_shared<std::string>(coloured_log_entry.data(), coloured_log_entry.size()));
  auto print_functor([level, colour, coloured_log_entry_ptr, message_ptr, colour_mode, project] {
    SendToConsole(colour_mode, colour, level, *coloured_log_entry_ptr, *message_ptr);
    Logging::Instance().WriteToCombinedLogfile(*coloured_log_entry_ptr + *message_ptr);
    Logging::Instance().WriteToProjectLogfile(project, *coloured_log_entry_ptr + *message_ptr);
  });
#else
  auto print_functor([level, colour, coloured_log_entry, message, colour_mode, project] {
    SendToConsole(colour_mode, colour, level, coloured_log_entry, message);
    Logging::Instance().WriteToCombinedLogfile(coloured_log_entry + message);
    Logging::Instance().WriteToProjectLogfile(project, coloured_log_entry + message);
  });
#endif
  Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
}

}  // namespace detail


// ===================================== TestLogMessage ============================================
TestLogMessage::TestLogMessage(Colour colour) : kColour_(colour), stream_() {}

TestLogMessage::~TestLogMessage() {
  Colour colour(kColour_);
  FilterMap filter(Logging::Instance().Filter());
#if defined(__GLIBCXX__)
  //  && __GLIBCXX__ < date (date in format of 20141218 as the date of fix of COW string)
  auto log_entry(std::make_shared<std::string>(stream_.str()));
  auto print_functor([colour, log_entry, filter] {
    ColouredPrint(colour, *log_entry);
    if (filter.size() == 1)
      Logging::Instance().WriteToProjectLogfile(filter.begin()->first, *log_entry);
    else
      Logging::Instance().WriteToCombinedLogfile(*log_entry);
  });
#else
  std::string log_entry(stream_.str());
  auto print_functor([colour, log_entry, filter] {
    ColouredPrint(colour, log_entry);
    if (filter.size() == 1)
      Logging::Instance().WriteToProjectLogfile(filter.begin()->first, log_entry);
    else
      Logging::Instance().WriteToCombinedLogfile(log_entry);
  });
#endif
  Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
}



// ========================================= Logging ===============================================
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
      visualiser_(),
      background_() {
  // Force intialisation order to ensure g_console_mutex is available in Logging's destuctor.
  std::lock_guard<maidsafe::detail::Spinlock> lock(g_console_mutex());
  static_cast<void>(lock);
}

Logging& Logging::Instance() {
  static Logging logging;
  return logging;
}

template <>
std::vector<std::vector<char>> Logging::Initialise(int argc, char** argv) {
  SetThisExecutablePath(argv);
  std::vector<std::vector<char>> unused_options;
  std::call_once(logging_initialised, [this, argc, argv, &unused_options]() {
    try {
      std::string config_file, log_folder;
      int colour_mode(-1);
      po::options_description log_config(
          SetProgramOptions(config_file, no_log_to_console_, log_folder, no_async_, colour_mode));
      ParseProgramOptions(log_config, config_file, argc, argv, log_variables_, unused_options);
      if (IsHelpOption(log_config))
        return;
#if USE_LOGGING
      background_ = maidsafe::make_unique<Active>();
      DoCasts(colour_mode, log_folder, colour_mode_, log_folder_);
      HandleFilterOptions();
      SetStreams();
#endif
    } catch (const std::exception& e) {
      std::cout << "Exception initialising logging: " << boost::diagnostic_information(e) << "\n\n";
    }
  });
  return unused_options;
}

template <>
std::vector<std::vector<wchar_t>> Logging::Initialise(int argc, wchar_t** argv) {
  SetThisExecutablePath(argv);
  std::vector<std::vector<wchar_t>> unused_options;
  std::call_once(logging_initialised, [this, argc, argv, &unused_options]() {
    try {
      std::string config_file, log_folder;
      int colour_mode(-1);
      po::options_description log_config(
          SetProgramOptions(config_file, no_log_to_console_, log_folder, no_async_, colour_mode));
      ParseProgramOptions(log_config, config_file, argc, argv, log_variables_, unused_options);
      if (IsHelpOption(log_config))
        return;
#if USE_LOGGING
      background_ = maidsafe::make_unique<Active>();
      DoCasts(colour_mode, log_folder, colour_mode_, log_folder_);
      HandleFilterOptions();
      SetStreams();
#endif
    } catch (const std::exception& e) {
      std::cout << "Exception initialising logging: " << boost::diagnostic_information(e) << "\n\n";
    }
  });
  return unused_options;
}

void Logging::InitialiseVlog(const std::string& prefix, const std::string& session_id,
                             const std::string& server_name, uint16_t server_port,
                             const std::string& server_dir) {
  if (visualiser_.initialised)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::already_initialised));
  std::call_once(visualiser_.initialised_once_flag, [&] {
    visualiser_.initialised = true;
    visualiser_.prefix = prefix;
    visualiser_.session_id = session_id;
    if (visualiser_.session_id.empty())
      LOG(kWarning) << "VLOG messages disabled since Vlog Session ID is empty.";
    visualiser_.logfile.stream.open(GetLogfileName("visualiser").c_str(), std::ios_base::trunc);
    visualiser_.server_name = server_name;
    visualiser_.server_port = server_port;
    visualiser_.server_dir = server_dir;
    visualiser_.server_stream.connect(server_name,
                                      std::to_string(static_cast<unsigned>(server_port)));
    if (!visualiser_.server_stream) {
      LOG(kError) << "Failed to connect to VLOG server: "
                  << visualiser_.server_stream.error().message();
    }
  });
}

bool Logging::IsHelpOption(const po::options_description& log_config) const {
#ifdef USE_LOGGING
  if (log_variables_.count("help")) {
    std::cout << log_config << "Logging levels are as follows:\n"
              << "Verbose(V), Info(I), Success(S), Warning(W), Error(E), Always(A)\n\n\n";
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
  std::strftime(mbstr, 100, "%Y-%m-%d_%H-%M-%S_", std::gmtime(&start_time_));  // NOLINT (Fraser)
  fs::path name(log_folder_ / mbstr);
  name += project + ".log";
  return name;
}

void Logging::SetStreams() {
  if (log_folder_.empty() || !SetupLogFolder(log_folder_))
    return;

  for (auto& entry : filter_) {
    auto log_file(make_unique<LogFile>());
    log_file->stream.open(GetLogfileName(entry.first).c_str(), std::ios_base::trunc);
    project_logfile_streams_.insert(std::make_pair(entry.first, std::move(log_file)));
  }

  if (filter_.size() != 1) {
    std::lock_guard<std::mutex> lock(combined_logfile_stream_.mutex);
    combined_logfile_stream_.stream.open(GetLogfileName("combined").c_str(), std::ios_base::trunc);
  }
}

void Logging::Send(std::function<void()> message_functor) {
#if USE_LOGGING
  background_->Send(message_functor);
#else
  message_functor();
#endif
}

void Logging::WriteToLogfile(const std::string& message, LogFile& log_file) {
  std::lock_guard<std::mutex> lock(log_file.mutex);
  if (log_file.stream.good()) {
    log_file.stream.write(message.c_str(), message.size());
    log_file.stream.flush();
  }
}

void Logging::WriteToCombinedLogfile(const std::string& message) {
  WriteToLogfile(message, combined_logfile_stream_);
}

void Logging::WriteToVisualiserLogfile(const std::string& message) {
  WriteToLogfile(message, visualiser_.logfile);
}

void Logging::WriteToVisualiserServer(const std::string& message) {
  if (!visualiser_.server_stream) {
    visualiser_.server_stream.clear();
    visualiser_.server_stream.connect(
        visualiser_.server_name, std::to_string(static_cast<unsigned>(visualiser_.server_port)));
    if (!visualiser_.server_stream) {
      LOG(kError) << "Failed to re-connect to VLOG server: "
                  << visualiser_.server_stream.error().message();
      return;
    }
  }

  visualiser_.server_stream << "POST " << visualiser_.server_dir << " HTTP/1.1\r\n"
                            << "Host: " << visualiser_.server_name << ':' << visualiser_.server_port
                            << "\r\n"
                            << "Content-Type: application/json\r\n"
                            << "Content-Length: " << std::to_string(message.size()) << "\r\n"
                            << "\r\n" << message << "\r\n" << std::flush;
  auto read_response([&](char delimiter) -> std::string {
    std::string response;
    if (!std::getline(visualiser_.server_stream, response, delimiter)) {
      LOG(kWarning) << "Failed to read VLOG server response.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));
    }
    return response;
  });
  try {
    read_response(' ');  // "HTTP/1.1"
    unsigned http_code{static_cast<unsigned>(std::stoul(read_response(' ')))};
    if (http_code != 200) {
      std::string http_code_message{read_response('\r')};
      LOG(kWarning) << "VLOG server responded with \"" << http_code << ": " << http_code_message
                    << "\" to request \"" << message << "\"";
    }
  } catch (const std::exception& e) {
    LOG(kWarning) << e.what();
  }
  std::array<char, 100> discarded;
  while (visualiser_.server_stream.readsome(&discarded[0], discarded.size())) {
  }
}

void Logging::WriteToProjectLogfile(const std::string& project, const std::string& message) {
  auto itr(project_logfile_streams_.find(project));
  if (itr != project_logfile_streams_.end())
    WriteToLogfile(message, *(itr->second));
}

void Logging::Flush() {
  for (auto& stream : project_logfile_streams_) {
    std::lock_guard<std::mutex> lock(stream.second->mutex);
    stream.second->stream.flush();
  }
  std::lock_guard<std::mutex> lock(combined_logfile_stream_.mutex);
  combined_logfile_stream_.stream.flush();
}

std::string Logging::VlogPrefix() const {
  if (!visualiser_.initialised)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unable_to_handle_request));
  return visualiser_.prefix;
}

std::string Logging::VlogSessionId() const {
  if (!visualiser_.initialised)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unable_to_handle_request));
  return visualiser_.session_id;
}

namespace detail {

std::string GetLocalTime() { return GetTime<TimeType::kLocal>(); }

std::string GetUTCTime() { return GetTime<TimeType::kUTC>(); }

}  // namespace detail

}  // namespace log

}  // namespace maidsafe
