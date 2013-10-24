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

#include "boost/process/child.hpp"
#include "boost/process/execute.hpp"
#include "boost/process/initializers.hpp"
#include "boost/process/wait_for_exit.hpp"
#include "boost/process/terminate.hpp"
#include "boost/system/error_code.hpp"

#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/crypto.h"

#include "ipc_child_process_location.h"  // NOLINT

namespace maidsafe {

namespace ipc {

namespace test {

namespace bi = boost::interprocess;
namespace bp = boost::process;

namespace {

#ifdef MAIDSAFE_WIN32
std::wstring StringToWstring(const std::string& input) {
  std::unique_ptr<wchar_t[]> buffer(new wchar_t[input.size()]);
  size_t num_chars = mbstowcs(buffer.get(), input.c_str(), input.size());
  return std::wstring(buffer.get(), num_chars);
}

std::wstring ConstructCommandLine(std::vector<std::string> process_args) {
  std::string args;
  for (auto arg : process_args)
    args += (arg + " ");
  return StringToWstring(args);
}
#else
std::string ConstructCommandLine(std::vector<std::string> process_args) {
  std::string args;
  for (auto arg : process_args)
    args += (arg + " ");
  return args;
}
#endif

}  // unnamed namespace

TEST_CASE("ipc create", "[ipc][Unit]") {

  std::string a = "test string 1xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string b = "test string 2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string c = "test string 3xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string d = "test string 4xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  std::vector<std::string> test_vec;
  test_vec.push_back(a);
  test_vec.push_back(b);
  test_vec.push_back(c);
  test_vec.push_back(a);
  CHECK_NOTHROW(CreateSharedMemory("test", test_vec));
 }

TEST_CASE("ipc read", "[ipc][Unit]") {
  std::string a = "test string 1xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string b = "test string 2xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string c = "test string 3xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  std::string d = "test string 4xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  std::vector<std::string> test_vec;
  test_vec.push_back(a);
  test_vec.push_back(b);
  test_vec.push_back(c);
  test_vec.push_back(a);
  REQUIRE_NOTHROW(ReadSharedMemory("test", 4));
  CHECK(test_vec == ReadSharedMemory("test", 4));
  }

TEST_CASE("ipc delete", "[ipc][Unit]") {
  // always passes, even if SHM noexists
  CHECK_NOTHROW(RemoveSharedMemory("test"));
  CHECK_NOTHROW(RemoveSharedMemory("test"));
}

TEST_CASE("IPC functions threaded", "[ipc][Unit]") {

  // Add scoped cleanup mechanism.
  struct Clean {
    Clean() { RemoveSharedMemory("thread_test"); }
    ~Clean() { RemoveSharedMemory("thread_test"); }
  } cleanup;

  // Set up object for sharing via IPC.
  std::vector<std::string> test1_vec;
  for (auto i(0); i < 5; ++i)
    test1_vec.push_back(RandomString(10 * i));

  // Check reading shared memory that hasn't been created yet throws.
  auto all_should_throw([=] {
    CHECK_THROWS_AS(ReadSharedMemory("thread_test", 1), bi::interprocess_exception);
  });
  std::thread reader_before_creation(all_should_throw);
  reader_before_creation.join();

  // Create the shared memory segments.
  CHECK_NOTHROW(CreateSharedMemory("thread_test", test1_vec));


  // Check reading works.
  auto all_should_match([=] {
    CHECK((test1_vec) == (ReadSharedMemory("thread_test", test1_vec.size())));
  });
  std::thread reader(all_should_match);
  reader.join();

  // Check modifying the original objects doesn't affect reading from shared memory
  test1_vec.clear();
  CHECK(test1_vec.empty());
  std::thread another_reader(all_should_match);
  another_reader.join();

  // Check deleting works.  Always passes, even if named shared memory doesn't exist.
  CHECK_NOTHROW(RemoveSharedMemory("thread_test"));
  std::thread reader_after_deletion(all_should_throw);
  reader_after_deletion.join();
}

TEST_CASE("IPC functions using boost process", "[ipc][Unit]") {
  const std::string kTestName("bptest");
  // Add scoped cleanup mechanism.
  struct Clean {
    Clean() { RemoveSharedMemory("bptest"); }
    ~Clean() { RemoveSharedMemory("bptest"); }
  } cleanup;

  // Set up objects for sharing via IPC.
  std::vector<std::string> test1_vec;
  std::string total;
  for (auto i(0); i < 5; ++i) {
    std::string test_string(RandomString(10 * (i + 1)));
    test1_vec.push_back(test_string);
    total += test_string;
  }
  std::string kAnswer(maidsafe::Base64Encode(
                                crypto::Hash<crypto::SHA512>(total).string()));
  // Set up boost::process args for passing to 'ipc_child_process' executable
  const auto kExePath(GetIpcChildProcessLocation());
  std::vector<std::string> process_args;
  process_args.push_back(kExePath);
  process_args.push_back(kTestName);
  process_args.push_back(std::to_string(test1_vec.size()));
  process_args.push_back(kAnswer);
  const auto kCommandLine(ConstructCommandLine(process_args));
  boost::system::error_code error_code;

  int exit_code(99);

  // Create kStructName, leave others unavailable
  CHECK_NOTHROW(CreateSharedMemory(kTestName, test1_vec));
  bp::child child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 0);  // kIntName unavailable
  exit_code = 99;
    // Check modifying the original objects doesn't affect reading from shared memory
  test1_vec.clear();
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 0);
  RemoveSharedMemory("bptest"); 
}

}  // namespace test

}  // namespace ipc

}  // namespace maidsafe
