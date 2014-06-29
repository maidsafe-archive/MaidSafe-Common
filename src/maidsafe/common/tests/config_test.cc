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

#include <cstring>
#include <iostream>
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/test.h"
#include "maidsafe/common/config.h"
#include "maidsafe/common/error.h"

namespace maidsafe {

namespace test {

char** g_argv;

TEST(ConfigTest, BEH_ApplicationVersion) {
  EXPECT_FALSE(maidsafe::kApplicationVersion().empty());
  std::cout << "Application version is " << maidsafe::kApplicationVersion() << '\n';
}

TEST(ConfigTest, BEH_TargetPlatform) {
  EXPECT_FALSE(maidsafe::kTargetPlatform().empty());
  std::cout << "Target platform is " << maidsafe::kTargetPlatform() << '\n';
}

TEST(ConfigTest, BEH_TargetArchitecture) {
  EXPECT_FALSE(maidsafe::kTargetArchitecture().empty());
  std::cout << "Target architecture is " << maidsafe::kTargetArchitecture() << '\n';
}

TEST(ConfigTest, BEH_ThisExecutableDir) {
  // Before calling SetThisExecutablePath
#ifdef MAIDSAFE_WIN32
  EXPECT_THROW(maidsafe::ThisExecutablePath(), maidsafe::common_error);
  EXPECT_THROW(maidsafe::ThisExecutableDir(), maidsafe::common_error);
#endif
  // Call SetThisExecutablePath
  maidsafe::SetThisExecutablePath(g_argv);
  auto this_exe_path(maidsafe::ThisExecutablePath());
  auto this_exe_dir(maidsafe::ThisExecutableDir());
  EXPECT_FALSE(this_exe_path.empty());
  EXPECT_FALSE(this_exe_dir.empty());
#ifdef MAIDSAFE_WIN32
  EXPECT_TRUE(this_exe_path.filename() == boost::filesystem::path("TESTconfig.exe"));
#else
  EXPECT_TRUE(this_exe_path.filename() == boost::filesystem::path("TESTconfig"));
#endif
  std::cout << "This executable's full path is " << this_exe_path << '\n';
  std::cout << "This executable's directory is " << this_exe_dir << '\n';

  // Call SetThisExecutablePath again - shouldn't change values
  char argv0[] = "New Path";
  char* new_argv[] = { argv0 };
  maidsafe::SetThisExecutablePath(new_argv);
  EXPECT_TRUE(maidsafe::ThisExecutablePath() == this_exe_path);
  EXPECT_TRUE(maidsafe::ThisExecutableDir() == this_exe_dir);
}

}  // namespace test

}  // namespace maidsafe
