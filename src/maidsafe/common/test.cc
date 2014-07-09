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

#include <future>
#include <vector>

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace test {

TestPath CreateTestPath(std::string test_prefix) {
  if (test_prefix.empty())
    test_prefix = "MaidSafe_Test";

  if (test_prefix.substr(0, 13) != "MaidSafe_Test" && test_prefix.substr(0, 12) != "Sigmoid_Test") {
    LOG(kWarning) << "Test prefix should preferably be \"MaidSafe_Test<optional"
                  << " test name>\" or \"Sigmoid_Test<optional test name>\".";
  }

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

namespace detail {

int ExecuteGTestMain(int argc, char* argv[]) {
  log::Logging::Instance().Initialise(argc, argv);
#if defined(__clang__) || defined(__GNUC__)
  // To allow Clang and GCC advanced diagnostics to work properly.
  testing::FLAGS_gtest_catch_exceptions = true;
#else
  testing::FLAGS_gtest_catch_exceptions = false;
#endif
  testing::InitGoogleTest(&argc, argv);
  int result(RUN_ALL_TESTS());
  int test_count = testing::UnitTest::GetInstance()->test_to_run_count();
  return (test_count == 0) ? -1 : result;
}

}  // namespace detail

}  // namespace test

}  // namespace maidsafe
