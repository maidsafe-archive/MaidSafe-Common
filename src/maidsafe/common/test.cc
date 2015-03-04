/*  Copyright 2011 MaidSafe.net limited

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

#include "maidsafe/common/test.h"

#ifndef MAIDSAFE_WIN32
#include <sys/resource.h>
#endif
#include <cstdint>
#include <future>
#include <iostream>
#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/program_options.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace maidsafe {

namespace test {

namespace {

boost::optional<fs::path> BootstrapFilePath(const fs::path& bootstrap_file_path = fs::path{""}) {
  static boost::optional<fs::path> bootstrap_file;
  if (!bootstrap_file_path.empty())
    bootstrap_file = bootstrap_file_path;
  return bootstrap_file;
}

po::options_description AvailableOptions() {
  po::options_description test_options("Test options");
  test_options.add_options()("seed", po::value<uint32_t>(),
                             "Seed for main psuedo random number generator.")(
      "delay", po::value<uint32_t>(),
      "Initial delay at start of execution of 'main' (in seconds).")(
      "bootstrap_file", po::value<std::string>(), "Path to bootstrap file.");
  return test_options;
}

po::variables_map ParseOptions(int argc, char* argv[],
                               const po::options_description& test_options) {
  po::variables_map variables_map;
  try {
    po::store(po::command_line_parser(argc, argv).options(test_options).allow_unregistered().run(),
              variables_map);
    po::notify(variables_map);
  } catch (const std::exception& e) {
    std::cout << "Parser error:\n " << e.what() << "\nRun with -h to see all options.\n";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  return variables_map;
}

void HandleHelp(const po::variables_map& variables_map) {
  if (variables_map.count("help")) {
    std::cout << AvailableOptions() << "\n\n";
    throw MakeError(CommonErrors::success);
  }
}

void HandleSeed(const po::variables_map& variables_map) {
  if (variables_map.count("seed"))
    maidsafe::detail::set_random_number_generator_seed(variables_map["seed"].as<uint32_t>());
}

void HandleDelay(const po::variables_map& variables_map) {
  if (variables_map.count("delay"))
    Sleep(std::chrono::seconds(variables_map["delay"].as<uint32_t>()));
}

void HandleBootstrapFile(const po::variables_map& variables_map) {
  if (variables_map.count("bootstrap_file"))
    BootstrapFilePath(variables_map["bootstrap_file"].as<std::string>());
}

}  // unnamed namespace

TestPath CreateTestPath(std::string test_prefix) {
  if (test_prefix.empty())
    test_prefix = "MaidSafe_Test";

  if (test_prefix.substr(0, 13) != "MaidSafe_Test")
    LOG(kWarning) << "Test prefix should preferably be \"MaidSafe_Test<optional test name>\".";

  test_prefix += "_%%%%-%%%%-%%%%";

  boost::system::error_code error_code;
  fs::path* test_path(
      new fs::path(fs::unique_path(fs::temp_directory_path(error_code) / test_prefix)));
  std::string debug(test_path->string());
  TestPath test_path_ptr(test_path, [debug](fs::path* delete_path) {
    if (!delete_path->empty()) {
      boost::system::error_code ec;
      if (fs::remove_all(*delete_path, ec) == 0) {
        LOG(kWarning) << "Failed to remove " << *delete_path;
      }
      if (ec.value() != 0) {
        LOG(kWarning) << "Error removing " << *delete_path << "  " << ec.message();
      }
    }
    delete delete_path;
  });
  if (error_code) {
    LOG(kWarning) << "Can't get a temp directory: " << error_code.message();
    return TestPath(new fs::path);
  }

  if (!fs::create_directories(*test_path, error_code) || error_code) {
    LOG(kWarning) << "Failed to create test directory " << *test_path
                  << "  (error message: " << error_code.message() << ")";
    return TestPath(new fs::path);
  }

  LOG(kInfo) << "Created test directory " << *test_path;
  return test_path_ptr;
}

void RunInParallel(int thread_count, std::function<void()> functor) {
  functor();
  std::vector<std::future<void>> futures;
  for (int i = 0; i < thread_count; ++i)
    futures.push_back(std::async(std::launch::async, functor));
  for (auto& future : futures)
    future.get();
}

uint16_t GetRandomPort() {
  static std::set<uint16_t> already_used_ports;
  if (already_used_ports.size() == 10000) {
    LOG(kInfo) << "Clearing already-used ports list.";
    already_used_ports.clear();
  }
  uint16_t port(0);
  do {
    port = (RandomUint32() % 64511) + 1025;
  } while (!already_used_ports.insert(port).second);
  return port;
}

std::string GetRandomIPv4AddressAsString() {
  auto address = std::to_string(RandomUint32() % 256);
  for (int i = 0; i != 3; ++i)
    address += '.' + std::to_string(RandomUint32() % 256);
  return address;
}

std::string GetRandomIPv6AddressAsString() {
  std::stringstream address;
  address << std::hex << (RandomUint32() % 65536);
  for (int i = 0; i != 7; ++i)
    address << ':' << RandomUint32() % 65536;
  return address.str();
}

#ifdef TESTING

void HandleTestOptions(int argc, char* argv[]) {
  try {
    auto test_options(AvailableOptions());
    test_options.add_options()("help,h", "");
    auto variables_map(ParseOptions(argc, argv, test_options));
    HandleHelp(variables_map);
    HandleSeed(variables_map);
    HandleDelay(variables_map);
    HandleBootstrapFile(variables_map);
  } catch (const maidsafe::maidsafe_error& error) {
    // Success is thrown when Help option is invoked.
    if (error.code() != maidsafe::make_error_code(maidsafe::CommonErrors::success))
      std::cout << "Exception: " << error.what() << '\n';
  } catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << '\n';
  }
}

RandomNumberSeeder::RandomNumberSeeder()
    : current_seed_(maidsafe::detail::random_number_generator_seed()) {}

void RandomNumberSeeder::OnTestStart(const testing::TestInfo& /*test_info*/) {
  // We need to set the seed at the start of every test so that when we run all tests (e.g. if we
  // just run ./test_encrypt rather than using CTest where each test is run as a standalone
  // process), they don't all use the first test's seed.
  maidsafe::detail::set_random_number_generator_seed(current_seed_);
}

void RandomNumberSeeder::OnTestEnd(const testing::TestInfo& test_info) {
  if (test_info.result()->Failed()) {
    TLOG(kRed) << "To potentially replicate the failure, try re-running with:\n   --gtest_filter="
               << test_info.test_case_name() << '.' << test_info.name() << " --seed "
               << current_seed_ << '\n';
  }
  ++current_seed_;
}

#if !defined(MAIDSAFE_WIN32) && !defined(__ANDROID__)
UlimitConfigurer::UlimitConfigurer()
    : prev_open_files_(0),
      prev_file_size_(ulimit(UL_GETFSIZE)),
      kLimitsOpenFiles(1024),
      kLimitsFileSize(2048) {
  struct rlimit rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  prev_open_files_ = rlp.rlim_cur;
}

void UlimitConfigurer::OnTestProgramStart(const testing::UnitTest& /*unit_test*/) {
  // We need to set the ulimit at the start of test
  if (prev_file_size_ < kLimitsFileSize)
    ulimit(UL_SETFSIZE, kLimitsFileSize);

  if (prev_open_files_ < kLimitsOpenFiles) {
    LOG(kWarning) << "not enough max open files, currently is " << prev_open_files_
                  << " , however expected to be sudo " << kLimitsOpenFiles;
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = kLimitsOpenFiles;
    if (setrlimit(RLIMIT_NOFILE, &limit) != 0)
      LOG(kError) << "error in changing max open files";
  }
}

void UlimitConfigurer::OnTestProgramEnd(const testing::UnitTest& unit_test) {
  // Once the limit has been decreased, it seems the session doesn't allow the limit to be
  // increased again, so a Restore shall not be carried out
  /*  // Restore the ulimit configuration
    if (prev_file_size_ < kLimitsFileSize)
      ulimit(UL_SETFSIZE, prev_file_size_);
  */
  // setrlimit can only affect current session, so no need to restore
  if (unit_test.Failed() && (prev_open_files_ < kLimitsOpenFiles))
    LOG(kError) << "Failing tests may caused by current max open files ( " << prev_open_files_
                << " ) is not enough and failed to change";
}
#endif

boost::optional<fs::path> GetBootstrapFilePath() { return BootstrapFilePath(); }

void PrepareBootstrapFile(fs::path bootstrap_file) {
  try {
    if (bootstrap_file.string() == "none") {
      fs::remove(ThisExecutableDir() / "bootstrap_override.dat");
      return;
    }
    if (bootstrap_file.is_relative())
      bootstrap_file = fs::current_path() / bootstrap_file;
    fs::copy_file(bootstrap_file, ThisExecutableDir() / "bootstrap_override.dat",
                  fs::copy_option::overwrite_if_exists);
  } catch (const std::exception& e) {
    LOG(kError) << "Failed to handle bootstrap override file: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::filesystem_io_error));
  }
}

void BootstrapFileHandler::OnTestStart(const testing::TestInfo& /*test_info*/) {
  if (GetBootstrapFilePath())
    PrepareBootstrapFile(*GetBootstrapFilePath());
}

#endif

namespace detail {

int ExecuteGTestMain(int argc, char* argv[]) {
  HandleTestOptions(argc, argv);
  log::Logging::Instance().Initialise(argc, argv);
#if defined(USE_VLOGGING) && !defined(VLOG_TEST)
  log::Logging::Instance().InitialiseVlog("", "", "", 0, "");
#endif
  testing::FLAGS_gtest_catch_exceptions = false;
  testing::InitGoogleTest(&argc, argv);
  testing::UnitTest* unit_test{testing::UnitTest::GetInstance()};
  unit_test->listeners().Append(new BootstrapFileHandler);
#if !defined(MAIDSAFE_WIN32) && !defined(__ANDROID__)
  unit_test->listeners().Append(new UlimitConfigurer);
#endif
  unit_test->listeners().Append(new RandomNumberSeeder);

  int result{RUN_ALL_TESTS()};

  // If at least one test ran, just return the result of RUN_ALL_TESTS().
  if (unit_test->test_to_run_count())
    return result;

  // Assume that if the '--gtest_filter' was set to run a single disabled test, the test is
  // being invoked by CTest, so return MAIDSAFE_DISABLED_TEST_RETURN_CODE to allow CTest to mark
  // this as skipped.
  if (unit_test->reportable_test_count() == 1 && unit_test->reportable_disabled_test_count() == 1)
    return MAIDSAFE_DISABLED_TEST_RETURN_CODE;

  return -1;
}

}  // namespace detail

}  // namespace test

}  // namespace maidsafe
