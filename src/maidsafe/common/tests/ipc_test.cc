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

TEST_CASE("IPC functions threaded", "[ipc][Unit]") {
  const std::string kStructName("threaded_struct_test");
  const std::string kIntName("threaded_int_test");
  const std::string kStringName("threaded_str_test");

  // Add scoped cleanup mechanism.
  std::function<void()> remove_shared_memory([=] {
    RemoveSharedMemory(kStructName);
    RemoveSharedMemory(kIntName);
    RemoveSharedMemory(kStringName);
  });
  on_scope_exit cleanup(remove_shared_memory);
  remove_shared_memory();

  // Set up objects for sharing via IPC.
  struct Simple {
    int a;
    bi::string str;
  } simple;
  simple.a = RandomInt32();
  auto rand_string(RandomString(100));
  bi::string tmp(rand_string.c_str(), rand_string.size());
  simple.str = tmp;
  const Simple kSimpleOriginal(simple);

  int32_t int_val(RandomInt32());
  const int32_t kIntOriginal(int_val);

  auto rand_string2(RandomString(200));
  bi::string str(rand_string2.c_str(), rand_string.size());
  const bi::string kStrOriginal(str);

  // Check reading shared memory that hasn't been created yet throws.
  auto all_should_throw([=] {
    CHECK_THROWS_AS(ReadSharedMemory<Simple>(kStructName), std::exception);
    CHECK_THROWS_AS(ReadSharedMemory<int32_t>(kIntName), bi::interprocess_exception);
    CHECK_THROWS_AS(ReadSharedMemory<bi::string>(kStringName), bi::interprocess_exception);
  });
  std::thread reader_before_creation(all_should_throw);
  reader_before_creation.join();

  // Create the shared memory segments.
  CHECK_NOTHROW(CreateSharedMemory<Simple>(kStructName, simple));
  CHECK_NOTHROW(CreateSharedMemory<int32_t>(kIntName, int_val));
  CHECK_NOTHROW(CreateSharedMemory<bi::string>(kStringName, str));

  // Check reading works.
  auto all_should_match([=] {
    CHECK(kSimpleOriginal.a == ReadSharedMemory<Simple>(kStructName).a);
    CHECK(kSimpleOriginal.str == ReadSharedMemory<Simple>(std::string(kStructName)).str);
    CHECK(kIntOriginal == ReadSharedMemory<int32_t>(kIntName));
    CHECK(kStrOriginal == ReadSharedMemory<bi::string>(kStringName));
  });
  std::thread reader(all_should_match);
  reader.join();

  // Check modifying the original objects doesn't affect reading from shared memory
  ++simple.a;
  simple.str.clear();
  ++int_val;
  str.clear();
  std::thread another_reader(all_should_match);
  another_reader.join();

  // Check deleting works.  Always passes, even if named shared memory doesn't exist.
  CHECK_NOTHROW(remove_shared_memory());
  CHECK_NOTHROW(RemoveSharedMemory("vec_test"));
  std::thread reader_after_deletion(all_should_throw);
  reader_after_deletion.join();
}

TEST_CASE("IPC functions using boost process", "[ipc][Unit]") {
  const std::string kStructName("struct_test");
  const std::string kIntName("int_test");
  const std::string kStringName("str_test");

  // Add scoped cleanup mechanism.
  std::function<void()> remove_shared_memory([=] {
    RemoveSharedMemory(kStructName);
    RemoveSharedMemory(kIntName);
    RemoveSharedMemory(kStringName);
  });
  on_scope_exit cleanup(remove_shared_memory);
  remove_shared_memory();

  // Set up objects for sharing via IPC.
  struct Simple {
    int a;
    bi::string str;
  } simple;
  simple.a = RandomInt32();
  auto rand_string(RandomAlphaNumericString(100));
  bi::string tmp(rand_string.c_str(), rand_string.size());
  simple.str = tmp;
  const Simple kSimpleOriginal(simple);

  int32_t int_val(RandomInt32());
  const int32_t kIntOriginal(int_val);

  auto rand_string2(RandomAlphaNumericString(200));
  bi::string str(rand_string2.c_str(), rand_string.size());
  const bi::string kStrOriginal(str);

  // Set up boost::process args for passing to 'ipc_child_process' executable
  const auto kExePath(GetIpcChildProcessLocation());
  std::vector<std::string> process_args;
  process_args.push_back(kExePath);
  process_args.push_back(kStructName);
  process_args.push_back(kIntName);
  process_args.push_back(kStringName);
  process_args.push_back(std::to_string(kSimpleOriginal.a));
  process_args.push_back(kSimpleOriginal.str.c_str());
  process_args.push_back(std::to_string(kIntOriginal));
  process_args.push_back(kStrOriginal.c_str());
  const auto kCommandLine(ConstructCommandLine(process_args));
  boost::system::error_code error_code;

  // Check reading shared memory that hasn't been created yet throws.
  bp::child child(bp::execute(bp::initializers::run_exe(kExePath),
                              bp::initializers::set_cmd_line(kCommandLine),
                              bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  auto exit_code(wait_for_exit(child, error_code));
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 3UL);  // kStructName unavailable

  // Create kStructName, leave others unavailable
  CHECK_NOTHROW(CreateSharedMemory<Simple>(kStructName, simple));
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 7UL);  // kIntName unavailable

  // Create kIntName, leave kStringName unavailable
  CHECK_NOTHROW(CreateSharedMemory<int32_t>(kIntName, int_val));
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 9UL);  // kStringName unavailable

  // Create kStringName and check for success
  CHECK_NOTHROW(CreateSharedMemory<bi::string>(kStringName, str));
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 0UL);

  // Check modifying the original objects doesn't affect reading from shared memory
  ++simple.a;
  simple.str.clear();
  ++int_val;
  str.clear();
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 0);

  // Check deleting works.
  CHECK_NOTHROW(remove_shared_memory());
  child = bp::child(bp::execute(bp::initializers::run_exe(kExePath),
                                bp::initializers::set_cmd_line(kCommandLine),
                                bp::initializers::set_on_error(error_code)));
  REQUIRE_FALSE(error_code);
  exit_code = wait_for_exit(child, error_code);
  REQUIRE_FALSE(error_code);
  CHECK(exit_code == 3);  // kStructName unavailable
}

}  // namespace test

}  // namespace ipc

}  // namespace maidsafe
