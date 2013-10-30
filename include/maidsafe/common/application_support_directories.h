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

#ifndef MAIDSAFE_COMMON_APPLICATION_SUPPORT_DIRECTORIES_H_
#define MAIDSAFE_COMMON_APPLICATION_SUPPORT_DIRECTORIES_H_

#ifndef MAIDSAFE_WIN32
#include <pwd.h>
#endif

#include <cstdlib>
#include <string>

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/preprocessor/stringize.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

#ifdef COMPANY_NAME
inline std::string kCompanyName() { return BOOST_PP_STRINGIZE(COMPANY_NAME); }
#else
#error COMPANY_NAME must be defined.
#endif

#ifdef APPLICATION_NAME
inline std::string kApplicationName() { return BOOST_PP_STRINGIZE(APPLICATION_NAME); }
#else
#error APPLICATION_NAME must be defined.
#endif

// Retrieve homedir from environment
inline boost::filesystem::path GetHomeDir() {
#if defined(MAIDSAFE_WIN32)
  std::string env_home2(std::getenv("HOMEPATH"));
  std::string env_home_drive(std::getenv("HOMEDRIVE"));
  if ((!env_home2.empty()) && (!env_home_drive.empty()))
    return boost::filesystem::path(env_home_drive + env_home2);
#elif defined(MAIDSAFE_APPLE) || defined(MAIDSAFE_LINUX)
  struct passwd* p = getpwuid(getuid());  // NOLINT (dirvine)
  std::string home(p->pw_dir);
  if (!home.empty())
    return boost::filesystem::path(home);
  std::string env_home(std::getenv("HOME"));
  if (!env_home.empty())
    return boost::filesystem::path(env_home);
#endif
  LOG(kError) << "Cannot deduce home directory path";
  return boost::filesystem::path();
}

// Application support directory in userspace; uses kCompanyName() and kApplicationName()
inline boost::filesystem::path GetUserAppDir() {
  const boost::filesystem::path kHomeDir(GetHomeDir());
  if (kHomeDir.empty()) {
    LOG(kError) << "Cannot deduce user application directory path";
    return boost::filesystem::path();
  }
#if defined(MAIDSAFE_WIN32)
  return boost::filesystem::path(std::getenv("APPDATA")) / kCompanyName() / kApplicationName();
#elif defined(MAIDSAFE_APPLE)
  return kHomeDir / "/Library/Application Support/" / kCompanyName() / kApplicationName();
#elif defined(MAIDSAFE_LINUX)
  return kHomeDir / ".config" / kCompanyName() / kApplicationName();
#else
  LOG(kError) << "Cannot deduce user application directory path";
  return boost::filesystem::path();
#endif
}

// Application support directory for all users; uses kCompanyName() and kApplicationName()
inline boost::filesystem::path GetSystemAppSupportDir() {
#if defined(MAIDSAFE_WIN32)
  return boost::filesystem::path(std::getenv("ALLUSERSPROFILE")) / kCompanyName() /
         kApplicationName();
#elif defined(MAIDSAFE_APPLE)
  return boost::filesystem::path("/Library/Application Support/") / kCompanyName() /
         kApplicationName();
#elif defined(MAIDSAFE_LINUX)
  return boost::filesystem::path("/usr/share/") / kCompanyName() / kApplicationName();
#else
  LOG(kError) << "Cannot deduce system wide application directory path";
  return boost::filesystem::path();
#endif
}

// Application install directory; uses kCompanyName() and kApplicationName()
inline boost::filesystem::path GetAppInstallDir() {
#if defined(MAIDSAFE_WIN32)
  std::string program_files =
      std::getenv(kTargetArchitecture() == "x86_64" ? "ProgramFiles(x86)" : "ProgramFiles");
  return boost::filesystem::path(program_files) / kCompanyName() / kApplicationName();
#elif defined(MAIDSAFE_APPLE)
  return boost::filesystem::path("/Applications/");
#elif defined(MAIDSAFE_LINUX)
  return boost::filesystem::path("/usr/bin/");
#else
  LOG(kError) << "Cannot deduce application directory path";
  return boost::filesystem::path();
#endif
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_APPLICATION_SUPPORT_DIRECTORIES_H_
