/* Copyright (c) 2012 maidsafe.net limited
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

  if (test_prefix.substr(0, 13) != "MaidSafe_Test" &&
      test_prefix.substr(0, 12) != "Sigmoid_Test") {
    LOG(kWarning) << "Test prefix should preferably be \"MaidSafe_Test<optional"
                 << " test name>\" or \"Sigmoid_Test<optional test name>\".";
  }

  test_prefix += "_%%%%-%%%%-%%%%";

  boost::system::error_code error_code;
  fs::path *test_path(new fs::path(fs::unique_path(
      fs::temp_directory_path(error_code) / test_prefix)));
  std::string debug(test_path->string());
  TestPath test_path_ptr(test_path, [debug](fs::path *delete_path) {
        if (!delete_path->empty()) {
          boost::system::error_code ec;
          if (fs::remove_all(*delete_path, ec) == 0) {
            LOG(kWarning) << "Failed to remove " << *delete_path;
          }
          if (ec.value() != 0) {
            LOG(kWarning) << "Error removing " << *delete_path << "  "
                          << ec.message();
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
  std::vector<std::future<void>> futures;
  for (int i = 0; i < thread_count; ++i)
    futures.push_back(std::async(std::launch::async, functor));
  for (auto& future : futures)
    future.get();
}

uint16_t GetRandomPort() {
  static std::set<uint16_t> already_used_ports;
  bool unique(false);
  uint16_t port(0);
  do {
    port = (RandomUint32() % 64511) + 1025;
    unique = (already_used_ports.insert(port)).second;
  } while (!unique);
  if (already_used_ports.size() == 10000) {
    LOG(kInfo) << "Clearing already-used ports list.";
    already_used_ports.clear();
  }
  return port;
}

int ExecuteMain(int argc, char **argv) {
  log::Logging::Instance().Initialise(argc, argv);
  testing::FLAGS_gtest_catch_exceptions = false;
  testing::InitGoogleTest(&argc, argv);
  int result(RUN_ALL_TESTS());
  int test_count = testing::UnitTest::GetInstance()->test_to_run_count();
  return (test_count == 0) ? -1 : result;
}

}  // namespace test

}  // namespace maidsafe
