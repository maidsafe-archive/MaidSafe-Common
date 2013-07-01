/* Copyright 2011 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

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
