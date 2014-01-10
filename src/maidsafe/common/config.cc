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

#include "maidsafe/common/config.h"
#include <mutex>

#include "maidsafe/common/error.h"

#if !defined TARGET_PLATFORM || !defined TARGET_ARCHITECTURE
#error TARGET_PLATFORM and TARGET_ARCHITECTURE must be defined.
#endif

namespace maidsafe {

namespace {

boost::filesystem::path g_this_executable_path;

}  // unnamed namespace

std::string kTargetPlatform() {
  static const std::string target_platform(BOOST_PP_STRINGIZE(TARGET_PLATFORM));
  return target_platform;
}

std::string kTargetArchitecture() {
  static const std::string target_architecture(BOOST_PP_STRINGIZE(TARGET_ARCHITECTURE));
  return target_architecture;
}

template <>
void SetThisExecutablePath(const char* const argv[]) {
  static std::once_flag flag;
  std::call_once(flag, [argv] { g_this_executable_path = boost::filesystem::path(argv[0]); });
}

template <>
void SetThisExecutablePath(const wchar_t* const argv[]) {
  static std::once_flag flag;
  std::call_once(flag, [argv] { g_this_executable_path = boost::filesystem::path(argv[0]); });
}

boost::filesystem::path ThisExecutablePath() {
  if (g_this_executable_path.empty())
    ThrowError(CommonErrors::uninitialised);
  return g_this_executable_path;
}

boost::filesystem::path ThisExecutableDir() {
  return ThisExecutablePath().parent_path();
}

}  // namespace maidsafe
