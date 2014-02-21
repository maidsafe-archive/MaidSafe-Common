/*  Copyright 2012 MaidSafe.net limited

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

#include <thread>
#include <string>
#include <vector>

#include "boost/filesystem.hpp"

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/log.h"

#include "maidsafe/client_manager/config.h"
#include "maidsafe/client_manager/download_manager.h"
#include "maidsafe/client_manager/return_codes.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace client_manager {

namespace test {

class DownloadManagerTest : public testing::Test {
 public:
  DownloadManagerTest() : download_manager_() {}

 protected:
  void InitialiseDownloadManager(const std::string& remote_subdir) {
    download_manager_.reset(new DownloadManager(detail::kDownloadManagerLocation + remote_subdir));
  }
  fs::path GetCurrentVersionDownloadPath() const {
    return download_manager_->local_path_ / download_manager_->latest_remote_version_;
  }
  void SetLatestLocalVersion(const std::string& version) {
    download_manager_->latest_local_version_ = version;
  }
  std::unique_ptr<DownloadManager> download_manager_;
};

TEST_F(DownloadManagerTest, BEH_UpdateSuccessful) {
  InitialiseDownloadManager("/download_manager_tests/successful");
  std::vector<fs::path> updated_files;
  SetLatestLocalVersion("1.1.001");
  EXPECT_EQ(kSuccess, download_manager_->Update(updated_files));

  EXPECT_EQ(3U, updated_files.size());
  fs::path local_path(GetCurrentVersionDownloadPath());
  EXPECT_TRUE(std::find(updated_files.begin(), updated_files.end(),
                        (local_path / "test_file1.gz").string()) != updated_files.end());
  EXPECT_TRUE(std::find(updated_files.begin(), updated_files.end(),
                        (local_path / "test_file2.gz").string()) != updated_files.end());
  EXPECT_TRUE(std::find(updated_files.begin(), updated_files.end(),
                        (local_path / "test_file3.gz").string()) != updated_files.end());

  boost::system::error_code error;
  EXPECT_TRUE(fs::exists(local_path, error));
  EXPECT_EQ(boost::system::errc::success, error.value());

  for (auto updated_file : updated_files) {
    error.clear();
    EXPECT_TRUE(fs::exists(updated_file, error));
    EXPECT_EQ(boost::system::errc::success, error.value());
  }
}

TEST_F(DownloadManagerTest, BEH_UpdateHasLatestVersion) {
  InitialiseDownloadManager("/download_manager_tests/has_latest");
  std::vector<fs::path> updated_files;
  SetLatestLocalVersion("1.1.002");
  EXPECT_EQ(kNoVersionChange, download_manager_->Update(updated_files));
  EXPECT_TRUE(updated_files.empty());
  boost::system::error_code error;
  EXPECT_FALSE(fs::exists(GetCurrentVersionDownloadPath(), error));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error.value()) << error.message();
}

TEST_F(DownloadManagerTest, BEH_UpdateNoManifestFile) {
  InitialiseDownloadManager("/download_manager_tests/no_manifest");
  std::vector<fs::path> updated_files;
  SetLatestLocalVersion("1.1.001");
  EXPECT_EQ(kManifestFailure, download_manager_->Update(updated_files));
  EXPECT_TRUE(updated_files.empty());
  fs::path local_path(GetCurrentVersionDownloadPath());
  boost::system::error_code error;
  EXPECT_FALSE(fs::exists(local_path / "test_file1.gz", error));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error.value());
  error.clear();
  EXPECT_FALSE(fs::exists(local_path / "test_file2.gz", error));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error.value());
  error.clear();
  EXPECT_FALSE(fs::exists(local_path / "test_file3.gz", error));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error.value());
}

class DownloadManagerCommonTest : public DownloadManagerTest,
                                  public ::testing::WithParamInterface<std::string> {
};  // NOLINT (Fraser)

TEST_P(DownloadManagerCommonTest, BEH_UpdateThirdFileFail) {
  InitialiseDownloadManager("/download_manager_tests/" + GetParam());
  std::vector<fs::path> updated_files;
  SetLatestLocalVersion("1.1.001");
  EXPECT_EQ(kSuccess, download_manager_->Update(updated_files));

  EXPECT_EQ(2U, updated_files.size());
  fs::path local_path(GetCurrentVersionDownloadPath());
  EXPECT_TRUE(std::find(updated_files.begin(), updated_files.end(),
                        (local_path / "test_file1.gz").string()) != updated_files.end());
  EXPECT_TRUE(std::find(updated_files.begin(), updated_files.end(),
                        (local_path / "test_file2.gz").string()) != updated_files.end());
  EXPECT_FALSE(std::find(updated_files.begin(), updated_files.end(),
                         (local_path / "test_file3.gz").string()) != updated_files.end());

  boost::system::error_code error;
  EXPECT_TRUE(fs::exists(local_path, error));
  EXPECT_EQ(boost::system::errc::success, error.value());
  error.clear();
  EXPECT_TRUE(fs::exists(local_path / "test_file1.gz", error));
  EXPECT_EQ(boost::system::errc::success, error.value());
  error.clear();
  EXPECT_TRUE(fs::exists(local_path / "test_file2.gz", error));
  EXPECT_EQ(boost::system::errc::success, error.value());
  error.clear();
  EXPECT_FALSE(fs::exists(local_path / "test_file3.gz", error));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error.value());
}

INSTANTIATE_TEST_CASE_P(
    AllFail, DownloadManagerCommonTest,
    testing::Values("incorrect_manifest", "no_signature", "incorrect_signature"));

}  // namespace test

}  // namespace client_manager

}  // namespace maidsafe
