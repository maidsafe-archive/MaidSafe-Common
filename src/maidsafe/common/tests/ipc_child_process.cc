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

#include <iostream>
#include <string>
#include <vector>

#include "maidsafe/common/ipc.h"

namespace bi = boost::interprocess;

int main(int argc, char* argv[]) {
  if (argc != 8) {
    std::cout << "Invalid args.  Returning 1.\n";
    return 1;
  }

  const std::string kStructName(argv[1]);
  const std::string kIntName(argv[2]);
  const std::string kStringName(argv[3]);
  struct Simple {
    int a;
    bi::string str;
  } simple;
  simple.a = std::stoi(std::string(argv[4]));
  simple.str = argv[5];
  const Simple kSimpleOriginal(simple);
  const int32_t kIntOriginal(static_cast<int32_t>(std::stoi(std::string(argv[6]))));
  const bi::string kStrOriginal(argv[7]);

  try {
    if (kSimpleOriginal.a != maidsafe::ipc::ReadSharedMemory<Simple>(kStructName).a) {
      std::cout << "kSimpleOriginal.a [" << kSimpleOriginal.a
                << "] != ReadSharedMemory<Simple>(kStructName).a ["
                << maidsafe::ipc::ReadSharedMemory<Simple>(kStructName).a << "].  Returning 2.\n";
      return 2;
    }
  }
  catch (const std::exception& e) {
    std::cout << e.what() << "  Returning 3.\n";
    return 3;
  }

  try {
    if (kSimpleOriginal.str !=
        maidsafe::ipc::ReadSharedMemory<Simple>(std::string(kStructName)).str) {
      std::cout << "kSimpleOriginal.str [" << kSimpleOriginal.str
                << "] != ReadSharedMemory<Simple>(std::string(kStructName)).str ["
                << maidsafe::ipc::ReadSharedMemory<Simple>(std::string(kStructName)).str
                << "].  Returning 4.\n";
      return 4;
    }
  }
  catch (const std::exception& e) {
    std::cout << e.what() << "  Returning 5.\n";
    return 5;
  }

  try {
    if (kIntOriginal != maidsafe::ipc::ReadSharedMemory<int32_t>(kIntName)) {
      std::cout << "kIntOriginal [" << kIntOriginal
                << "] != ReadSharedMemory<int32_t>(kIntName) ["
                << maidsafe::ipc::ReadSharedMemory<int32_t>(kIntName) << "].  Returning 6.\n";
      return 6;
    }
  }
  catch (const std::exception& e) {
    std::cout << e.what() << "  Returning 7.\n";
    return 7;
  }

  try {
    if (kStrOriginal != maidsafe::ipc::ReadSharedMemory<bi::string>(kStringName)) {
      std::cout << "kStrOriginal [" << kStrOriginal
                << "] != ReadSharedMemory<bi::string>(kStringName) ["
                << maidsafe::ipc::ReadSharedMemory<bi::string>(kStringName) << "].  Returning 8.\n";
      return 8;
    }
  }
  catch (const std::exception& e) {
    std::cout << e.what() << "  Returning 9.\n";
    return 9;
  }

  return 0;
}
