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

#ifndef MAIDSAFE_COMMON_IPC_H_
#define MAIDSAFE_COMMON_IPC_H_

#include <string>

#include "boost/interprocess/managed_shared_memory.hpp"
#include "boost/interprocess/containers/string.hpp"

namespace maidsafe {

namespace ipc {

// NOTE!, dereferencing pointers in containers is not allowed in these functions as is
// therefor no stl containers or types that require dereferencing will work
// unless boost::interprocess::offset_ptr is used to create the pointer types in containers
// or classes. No VECTORS, MAP or SETS etc. please roll your own in that case from
// http://www.boost.org/doc/libs/release/doc/html/interprocess.html

// This is an extreme simplification of boost::ipc to allow simple types to be passed.
// there is no mem trimming at all, we always use the full 65536 bytes
// there is also only the ability to have one item in the SHM
// increasing number of items is trivial if required and is done by naming
// each instance (where now we have "a" declared) and adding several
// items to the SHM. "a" is used here to make mem surfing a little harder

void RemoveSharedMemory(std::string name);

template <typename Type>
void CreateSharedMemory(std::string name, Type type) {
  RemoveSharedMemory(name);
  // Create a managed shared memory segment of large arbitary size !
  boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, name.c_str(),
                                                     65536);
  // Create an object of Type initialized to type
  segment.construct<Type>("a")(type);
}

template <typename Type>
auto ReadSharedMemory(std::string name)->Type {
  // Open managed segment
  boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, name.c_str());
  return *segment.find<Type>("a").first;
}



}  // namespace ipc

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_IPC_H_
