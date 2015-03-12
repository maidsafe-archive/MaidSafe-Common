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

#include <string>
#include <vector>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/encode.h"
#include "maidsafe/common/ipc.h"
#include "maidsafe/common/utils.h"

int main(int argc, char* argv[]) {
  int args_required(4);
  if (argc != args_required)
    return -1;

  const std::string TestName(maidsafe::hex::DecodeToString(argv[1]));
  const std::string TestAnswer(argv[3]);
  const int TestNumber(std::stoi(std::string(argv[2])));

  try {
    auto vec_strings = maidsafe::ipc::ReadSharedMemory(TestName, TestNumber);
    std::string answer;
    for (auto& vec_string : vec_strings)
      answer += vec_string;

    if (TestAnswer !=
        maidsafe::hex::Encode(maidsafe::crypto::Hash<maidsafe::crypto::SHA512>(answer).string())) {
      return -2;
    }
  } catch (...) {
    return -3;
  }

  return 0;
}
