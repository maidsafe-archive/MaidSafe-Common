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

#ifndef MAIDSAFE_CLIENT_MANAGER_DOWNLOAD_MANAGER_H_
#define MAIDSAFE_CLIENT_MANAGER_DOWNLOAD_MANAGER_H_

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/asio/streambuf.hpp"

#include "maidsafe/common/rsa.h"

#include "maidsafe/client_manager/config.h"

namespace maidsafe {

namespace client_manager {

namespace test {
class DownloadManagerTest;
}

class DownloadManager {
 public:
  DownloadManager(std::string location = detail::kDownloadManagerLocation,
                  std::string site = detail::kDownloadManagerSite,
                  std::string protocol = detail::kDownloadManagerProtocol);
  ~DownloadManager();
  // Retrieves the latest bootstrap file from the server.
  std::string GetBootstrapInfo();
  // Check for an update and carry out required updates. Populates updated_files with list of files
  // that were updated. Return code indicates success/type of failure.
  int Update(std::vector<boost::filesystem::path>& updated_files);
  std::string latest_local_version() const { return latest_local_version_; }
  std::string latest_remote_version() const { return latest_remote_version_; }
  friend class test::DownloadManagerTest;

 private:
  bool InitialiseLocalPath();
  bool InitialisePublicKey();
  int GetAndCheckLatestRemoteVersion();
  bool GetManifest(std::vector<std::string>& files_in_manifest);
  void GetNewFiles(const std::vector<std::string>& files_in_manifest,
                   std::vector<boost::filesystem::path>& updated_files);
  std::string GetAndVerifyFile(const boost::filesystem::path& remote_path);
  bool PrepareDownload(const boost::filesystem::path& remote_path,
                       boost::asio::streambuf& response_buffer, std::istream& response_stream,
                       boost::asio::ip::tcp::socket& socket);
  bool CheckResponse(const boost::filesystem::path& remote_path, std::istream& response_stream);
  std::string DownloadFile(const boost::filesystem::path& remote_path);

  std::string location_, site_, protocol_, latest_local_version_, latest_remote_version_;
  asymm::PublicKey maidsafe_public_key_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ip::tcp::resolver::query query_;
  boost::filesystem::path local_path_, latest_remote_path_;
  bool initialised_;
};

}  // namespace client_manager

}  // namespace maidsafe

#endif  // MAIDSAFE_CLIENT_MANAGER_DOWNLOAD_MANAGER_H_
