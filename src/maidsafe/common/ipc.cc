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
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace ipc {

void RemoveSharedMemory(std::string name_in) {
  std::string name(HexEncode(name_in));
  boost::interprocess::shared_memory_object::remove(name.c_str());
}

void CreateSharedMemory(std::string name_in, std::vector<std::string> items) {
  std::string name(HexEncode(name_in));
  RemoveSharedMemory(name);
  // Create a managed shared memory segment of large arbitary size !
  bi::managed_shared_memory segment(bi::create_only , name.c_str(),
                                                     65536);
  // Create an object of Type initialized to type
  CharAllocator charallocator(segment.get_segment_manager());
  for (size_t i(0); i < items.size(); ++i) {
    bi_string str(charallocator);
    str = HexEncode(items.at(i)).c_str();
    segment.construct<bi_string>(std::to_string(i).c_str())(str);
  }
}

std::vector<std::string> ReadSharedMemory(std::string name_in, int number) {
  std::string name(HexEncode(name_in));
  // Open managed segment
  boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, name.c_str());
  CharAllocator charallocator(segment.get_segment_manager());
  bi_string str(charallocator);
  std::vector<std::string> ret_vec;
  for (int i(0); i < number; ++i) {
    auto res = segment.find<bi_string>(std::to_string(i).c_str());
    ret_vec.push_back(HexDecode(std::string(res.first->c_str(), res.first->size())));
  }
  return ret_vec;
}


}  // namespace ipc

}  // namespace maidsafe
