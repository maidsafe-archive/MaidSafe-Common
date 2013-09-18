/*  Copyright 2011 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_BREAKPAD_H_
#define MAIDSAFE_COMMON_BREAKPAD_H_

#include <string>

#ifdef WIN32
#  include "breakpad/client/windows/handler/exception_handler.h"
#else
#  include "breakpad/client/linux/handler/exception_handler.h"
#endif

#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1200
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace maidsafe {

namespace crash_report {

#ifdef WIN32
bool DumpCallback(const wchar_t* dump_path,
                  const wchar_t* minidump_id,
                  void* context,
                  EXCEPTION_POINTERS* /*exinfo*/,
                  MDRawAssertionInfo* /*assertion*/,
                  bool succeeded);
#else
bool DumpCallback(const char* dump_path,
                  const char* minidump_id,
                  void* context,
                  bool succeeded);
#endif

struct ProjectInfo {
  ProjectInfo(std::string project_name, std::string project_version);
  std::string version;
  std::string name;
};

}  // namespace crash_report

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BREAKPAD_H_
