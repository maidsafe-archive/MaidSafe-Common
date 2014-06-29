/*  Copyright 2013 MaidSafe.net limited

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

#include "maidsafe/common/ipc.h"

#include <functional>
#include <string>
#include <thread>

#ifdef MAIDSAFE_BSD
extern "C" char **environ;
#endif

#include "boost/process/child.hpp"
#include "boost/process/execute.hpp"
#include "boost/process/initializers.hpp"
#include "boost/process/wait_for_exit.hpp"
#include "boost/process/terminate.hpp"
#include "boost/system/error_code.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/process.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace ipc {

namespace test {

namespace bi = boost::interprocess;
namespace bp = boost::process;

TEST(IPCTest, BEH_IPCcreate) {
  std::string a = "test string 1xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string b = "test string 2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string c = "test string 3xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string d = "test string 4xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  std::vector<std::string> test_vec;
  test_vec.push_back(a);
  test_vec.push_back(b);
  test_vec.push_back(c);
  test_vec.push_back(a);
  EXPECT_NO_THROW(CreateSharedMemory("test", test_vec));
}

TEST(IPCTest, BEH_IPCread) {
  std::string a = "test string 1xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string b = "test string 2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string c = "test string 3xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string d = "test string 4xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  std::vector<std::string> test_vec;
  test_vec.push_back(a);
  test_vec.push_back(b);
  test_vec.push_back(c);
  test_vec.push_back(a);
  ASSERT_NO_THROW(ReadSharedMemory("test", 4));
  EXPECT_TRUE(test_vec == ReadSharedMemory("test", 4));
}

TEST(IPCTest, BEH_IpcDelete) {
  // always passes, even if SHM noexists
  EXPECT_NO_THROW(RemoveSharedMemory("test"));
  EXPECT_NO_THROW(RemoveSharedMemory("test"));
}

TEST(IPCTest, FUNC_IpcFunctionsThreaded) {
  const std::string kTestName(RandomString(8));
  // Add scoped cleanup mechanism.
  struct Clean {
    explicit Clean(std::string test_name) : kTestName(test_name) { RemoveSharedMemory(kTestName); }
    ~Clean() { RemoveSharedMemory(kTestName); }
    const std::string kTestName;
  } cleanup(kTestName);

  // Set up object for sharing via IPC.
  std::vector<std::string> test1_vec;
  for (auto i(0); i < 5; ++i)
    test1_vec.push_back(RandomString(10 * i));

  // Check reading shared memory that hasn't been created yet throws.
  bool all_threw{ false };
  auto all_should_throw([&] {
    try {
      ReadSharedMemory(kTestName, 1);
    }
    catch (const bi::interprocess_exception&) {
      all_threw = true;
    }
  });
  std::thread reader_before_creation(all_should_throw);
  reader_before_creation.join();
  EXPECT_TRUE(all_threw);

  // Create the shared memory segments.
  EXPECT_NO_THROW(CreateSharedMemory(kTestName, test1_vec));

  // Check reading works.
  bool all_match{ false };
  auto all_should_match([&] {
    all_match = ((test1_vec) == (ReadSharedMemory(kTestName, static_cast<int>(test1_vec.size()))));
  });
  std::thread reader(all_should_match);
  reader.join();
  EXPECT_TRUE(all_match);

  // Check modifying the original objects doesn't affect reading from shared memory
  test1_vec.clear();
  EXPECT_TRUE(test1_vec.empty());
  all_match = false;
  std::thread another_reader(all_should_match);
  another_reader.join();
  EXPECT_TRUE(all_match);

  // Check deleting works.  Always passes, even if named shared memory doesn't exist.
  EXPECT_NO_THROW(RemoveSharedMemory(kTestName));
  all_threw = false;
  std::thread reader_after_deletion(all_should_throw);
  reader_after_deletion.join();
  EXPECT_TRUE(all_threw);
  RemoveSharedMemory(kTestName);
}

TEST(IPCTest, FUNC_IpcFunctionsUsingBoostProcess) {
  const std::string kTestName(RandomString(8));
  struct Clean {
    explicit Clean(std::string test_name) : kTestName(test_name) { RemoveSharedMemory(kTestName); }
    ~Clean() { RemoveSharedMemory(kTestName); }
    const std::string kTestName;
  } cleanup(kTestName);
  // Set up objects for sharing via IPC.
  std::vector<std::string> test1_vec;
  std::string total;
  for (auto i(0); i < 5; ++i) {
    std::string test_string(RandomString(10 * (i + 1)));
    test1_vec.push_back(test_string);
    total += test_string;
  }
  std::string kAnswer(maidsafe::HexEncode(
                                crypto::Hash<crypto::SHA512>(total).string()));
  // Set up boost::process args for passing to 'ipc_child_process' executable
  const auto kExePath(process::GetOtherExecutablePath("ipc_child_process").string());
  std::vector<std::string> process_args;
  process_args.push_back(kExePath);
  process_args.push_back(HexEncode(kTestName));
  process_args.push_back(std::to_string(test1_vec.size()));
  process_args.push_back(kAnswer);
  const auto kCommandLine(process::ConstructCommandLine(process_args));
  boost::system::error_code error_code;

  int exit_code(99);

  EXPECT_NO_THROW(CreateSharedMemory(kTestName, test1_vec));
  bp::child child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                              bp::initializers::set_cmd_line(kCommandLine),
                              bp::initializers::set_on_error(error_code)));
  ASSERT_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  ASSERT_FALSE(error_code);
  EXPECT_TRUE(exit_code == 0);
  exit_code = 99;
    // Check modifying the original objects doesn't affect reading from shared memory
  test1_vec.clear();
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  EXPECT_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  ASSERT_FALSE(error_code);
  EXPECT_TRUE(exit_code == 0);
  RemoveSharedMemory(kTestName);
}

}  // namespace test

}  // namespace ipc

}  // namespace maidsafe
