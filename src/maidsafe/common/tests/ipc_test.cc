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

#include "maidsafe/common/ipc.h"

#include <string>
#include <vector>

#include "maidsafe/common/test.h"

namespace maidsafe {

namespace test {
namespace bi = boost::interprocess;
// when run in ctest these tests will be seperate processes. We must run them in sequence

TEST_CASE("ipc create", "[ipc][Unit]") {
  int int_val(123);
  bi::string str("test ipc");
  struct Simple {
    int a = 1;
    bi::string str = "a test string";
  } simple;
  CHECK_NOTHROW(ipc::CreateSharedMem<Simple>("struct_test", simple));
  CHECK_NOTHROW(ipc::CreateSharedMem<int>("int_test", int_val));
  CHECK_NOTHROW(ipc::CreateSharedMem<bi::string>("str_test", str));
}

TEST_CASE("ipc read", "[ipc][Unit]") {
  int int_val_orig(123);
  bi::string str_orig("test ipc");
  struct Simple {
    int a = 1;
    bi::string str = "a test string";
  } simple_orig;
  CHECK(simple_orig.a == ipc::ReadSharedMem<Simple>("struct_test").a);
  CHECK(simple_orig.str == ipc::ReadSharedMem<Simple>(std::string("struct_test")).str);
  CHECK(int_val_orig == ipc::ReadSharedMem<int>("int_test"));
  CHECK(str_orig == ipc::ReadSharedMem<bi::string>("str_test"));
}

TEST_CASE("ipc delete", "[ipc][Unit]") {
  // always passes, even if SHM noexists
  CHECK_NOTHROW(ipc::RemoveSharedMem("vec_test"));
  CHECK_NOTHROW(ipc::RemoveSharedMem("str_test"));
}

}  // namespace test

}  // namespace maidsafe
