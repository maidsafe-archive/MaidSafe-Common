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

#include <limits.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <mutex>

#include "boost/filesystem/operations.hpp"

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
  std::call_once(flag, [argv] {
    const char *path = argv[0];
#ifdef WIN32
    char buffer[32768];
    if (_get_pgmptr(const_cast<char **>(&path))) {
      if (GetModuleFileNameA(nullptr, buffer, sizeof(buffer)))
        path = buffer;
    }
#endif
#ifdef __linux__
    char buffer[PATH_MAX];
    size_t len = 0;
    if ((len = readlink("/proc/self/exe", buffer, sizeof(buffer))) > 0) {
      buffer[len] = 0;
      path = buffer;
    }
#endif
#ifdef __FreeBSD__
    char buffer[PATH_MAX];
    size_t bufferlen = sizeof(buffer);
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    if (!sysctl(mib, 4, buffer, &bufferlen, nullptr, 0))
      path = buffer;
#endif
#ifdef __APPLE__
    char buffer[MAXPATHLEN];
    uint32_t bufferlen = sizeof(buffer);
    if (!_NSGetExecutablePath(buffer, &bufferlen))
      path = buffer;
#endif
    try {
      g_this_executable_path = boost::filesystem::absolute(path);
    }
    catch (const std::exception&) {
      g_this_executable_path = boost::filesystem::path(path);
    }
  });
}

template <>
void SetThisExecutablePath(const wchar_t* const argv[]) {
  static std::once_flag flag;
  std::call_once(flag, [argv] {
    const wchar_t *path = argv[0];
#ifdef WIN32
    wchar_t buffer[32768];
    if (_get_wpgmptr(const_cast<wchar_t **>(&path)))
      if (GetModuleFileNameW(nullptr, buffer, sizeof(buffer)))
        path = buffer;
#endif
    try {
      g_this_executable_path = boost::filesystem::absolute(path);
    }
    catch (const std::exception&) {
      g_this_executable_path = boost::filesystem::path(path);
    }
  });
}

boost::filesystem::path ThisExecutablePath() {
  if (g_this_executable_path.empty())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  return g_this_executable_path;
}

boost::filesystem::path ThisExecutableDir() {
  return ThisExecutablePath().parent_path();
}

namespace tcp {

const unsigned kMaxRangeAboveDefaultPort(10);

}  // namespace tcp

}  // namespace maidsafe
