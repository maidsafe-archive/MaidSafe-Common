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

#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace ipc {

namespace test {

namespace bi = boost::interprocess;

TEST_CASE("IPC functions", "[ipc][Unit]") {
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

  // Set up objects for sharing via IPC.
  struct Simple {
    int a;
    bi::string str;
  } simple;
  simple.a = RandomInt32();
  simple.str.assign(RandomString(100).c_str(), 100);
  const Simple kSimpleOriginal(simple);

  int int_val(RandomInt32());
  const int kIntOriginal(int_val);

  bi::string str(RandomString(200).c_str(), 200);
  const bi::string kStrOriginal(str);

  // Check reading shared memory that hasn't been created yet throws.
  auto all_should_throw([=] {
    CHECK_THROWS_AS(ReadSharedMemory<Simple>(kStructName), bi::interprocess_exception);
    CHECK_THROWS_AS(ReadSharedMemory<int>(kIntName), bi::interprocess_exception);
    CHECK_THROWS_AS(ReadSharedMemory<bi::string>(kStringName), bi::interprocess_exception);
  });
  std::thread reader_before_creation(all_should_throw);
  reader_before_creation.join();

  // Create the shared memory segments.
  CHECK_NOTHROW(CreateSharedMemory<Simple>(kStructName, simple));
  CHECK_NOTHROW(CreateSharedMemory<int>(kIntName, int_val));
  CHECK_NOTHROW(CreateSharedMemory<bi::string>(kStringName, str));

  // Check reading works.
  auto all_should_match([=] {
    CHECK(kSimpleOriginal.a == ReadSharedMemory<Simple>(kStructName).a);
    CHECK(kSimpleOriginal.str == ReadSharedMemory<Simple>(std::string(kStructName)).str);
    CHECK(kIntOriginal == ReadSharedMemory<int>(kIntName));
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

}  // namespace test

}  // namespace ipc

}  // namespace maidsafe
