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

#include "maidsafe/client_manager/download_manager.h"

#include <cstdint>
#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

#include "boost/algorithm/string.hpp"
#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/read_until.hpp"
#include "boost/asio/write.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/client_manager/config.h"
#include "maidsafe/client_manager/return_codes.h"
#include "maidsafe/client_manager/utils.h"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace fs = boost::filesystem;

namespace maidsafe {

namespace client_manager {

DownloadManager::DownloadManager(std::string location, std::string site, std::string protocol)
    : location_(std::move(location)),
      site_(std::move(site)),
      protocol_(std::move(protocol)),
      latest_local_version_(kApplicationVersion()),
      latest_remote_version_("0.0.000"),
      maidsafe_public_key_(detail::kMaidSafePublicKey()),
      io_service_(),
      resolver_(io_service_),
      query_(site_, protocol_),
      local_path_(GetSystemAppSupportDir()),
      latest_remote_path_(),
      initialised_(false) {
  if (InitialiseLocalPath())
    initialised_ = InitialisePublicKey();
}

DownloadManager::~DownloadManager() {
#ifdef TESTING
  boost::system::error_code error_code;
  if (!local_path_.empty())
    fs::remove_all(local_path_, error_code);
#endif
}

bool DownloadManager::InitialiseLocalPath() {
  boost::system::error_code error_code;
#ifdef TESTING
  local_path_ = fs::unique_path(fs::temp_directory_path(error_code) /
                                "MaidSafe_Test_DownloadManager_%%%%-%%%%-%%%%");
#endif
  if (!fs::exists(local_path_, error_code)) {
    if (!fs::create_directories(local_path_, error_code) || error_code) {
      LOG(kError) << "Problem establishing temporary path for downloads: " << error_code.message();
      local_path_.clear();
      return false;
    }
  }
  if (!fs::is_directory(local_path_, error_code) || error_code) {
    LOG(kError) << "Download local path is not a directory: " << error_code.message();
    local_path_.clear();
    return false;
  }
  return true;
}

bool DownloadManager::InitialisePublicKey() {
  try {
#ifdef TESTING
    maidsafe_public_key_ =
        asymm::DecodeKey(asymm::EncodedPublicKey(DownloadFile("test_public_key.dat")));
#endif
    if (!asymm::ValidateKey(maidsafe_public_key_)) {
      LOG(kError) << "MaidSafe public key invalid.";
      return false;
    }
  }
  catch (const std::exception& e) {
    LOG(kError) << "Exception validating MaidSafe public key: " << e.what();
    return false;
  }
  return true;
}

std::string DownloadManager::GetBootstrapInfo() {
  if (!initialised_) {
    LOG(kError) << "DownloadManager is not initialised.";
    return "";
  }
  std::string bootstrap_content(GetAndVerifyFile(detail::kGlobalBootstrapFilename));
  if (bootstrap_content.empty())
    LOG(kError) << "Failed to download bootstrap file.";
  return bootstrap_content;
}

int DownloadManager::Update(std::vector<fs::path>& updated_files) {
  updated_files.clear();
  if (!initialised_) {
    LOG(kError) << "DownloadManager is not initialised.";
    return kUninitialised;
  }
  int latest_remote_version(GetAndCheckLatestRemoteVersion());
  if (latest_remote_version < 0)
    return latest_remote_version;

  boost::system::error_code error_code;
  if (!fs::create_directories(local_path_ / latest_remote_version_, error_code) || error_code) {
    LOG(kError) << "Failed to create download directory for latest version at "
                << local_path_ / latest_remote_version_ << ": " << error_code.message();
    return kLocalFailure;
  }

  std::vector<std::string> files_in_manifest;
  if (!GetManifest(files_in_manifest))
    return kManifestFailure;

  GetNewFiles(files_in_manifest, updated_files);
  if (updated_files.empty())
    return kDownloadFailure;

  latest_local_version_ = latest_remote_version_;
  return kSuccess;
}

int DownloadManager::GetAndCheckLatestRemoteVersion() {
  latest_remote_version_ = GetAndVerifyFile(detail::kVersionFilename);
  if (latest_remote_version_.empty()) {
    LOG(kError) << "Failed to download version file.";
    latest_remote_version_ = "0.0.000";
    return kInvalidVersion;
  }

  LOG(kVerbose) << "Latest local version is " << latest_local_version_
                << " and latest remote version is " << latest_remote_version_;

  int latest_remote_version(VersionToInt(latest_remote_version_));
  if (latest_remote_version == kInvalidVersion) {
    LOG(kError) << "Downloaded version file yields invalid version: " << latest_remote_version_;
    latest_remote_version_ = "0.0.000";
    return kDownloadFailure;
  }

  latest_remote_path_ = fs::path(latest_remote_version_) / detail::kTargetPlatformAndArchitecture;
  if (latest_remote_version <= VersionToInt(latest_local_version_)) {
    LOG(kInfo) << "No version change.";
    return kNoVersionChange;
  }

  return latest_remote_version;
}

bool DownloadManager::GetManifest(std::vector<std::string>& files_in_manifest) {
  std::string manifest_content(GetAndVerifyFile(latest_remote_path_ / detail::kManifestFilename));
  if (manifest_content.empty()) {
    LOG(kError) << "Failed to download manifest file";
    return false;
  }

  boost::split(files_in_manifest, manifest_content, boost::is_any_of("\n"));
  auto itr(files_in_manifest.begin());
  while (itr != files_in_manifest.end()) {
    if ((*itr).empty())
      itr = files_in_manifest.erase(itr);
    else
      ++itr;
  }

#ifdef DEBUG
  for (auto file : files_in_manifest)
    LOG(kInfo) << "file in manifest: " << file;
#endif

  return !files_in_manifest.empty();
}

void DownloadManager::GetNewFiles(const std::vector<std::string>& files_in_manifest,
                                  std::vector<fs::path>& updated_files) {
  for (const auto& file : files_in_manifest) {  // NOLINT (Fraser)
    std::string content(GetAndVerifyFile(latest_remote_path_ / file));
    if (content.empty())
      continue;

    fs::path new_file_path(local_path_ / latest_remote_version_ / file);
    if (!WriteFile(new_file_path, content)) {
      LOG(kError) << "Failed to write downloaded file to " << new_file_path;
      continue;
    }

    LOG(kInfo) << "Updated file: " << new_file_path;
    updated_files.push_back(new_file_path);
  }
}

std::string DownloadManager::GetAndVerifyFile(const fs::path& remote_path) {
  try {
    asymm::Signature signature(DownloadFile(remote_path.string() + detail::kSignatureExtension));
    asymm::PlainText contents(DownloadFile(remote_path));
    if (!asymm::CheckSignature(contents, signature, maidsafe_public_key_)) {
      LOG(kError) << "Signature of " << remote_path << " is invalid.";
      return "";
    }
    return contents.string();
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error getting and verifying " << remote_path << ": " << e.what();
    return "";
  }
}

bool DownloadManager::PrepareDownload(const fs::path& remote_path, asio::streambuf& response_buffer,
                                      std::istream& response_stream, ip::tcp::socket& socket) {
  try {
    asio::streambuf request_buffer;
    std::ostream request_stream(&request_buffer);
    asio::connect(socket, resolver_.resolve(query_));
    // Form the request. Use "Connection: close" header so that the server will close the socket
    // after transmitting the response; allowing us to treat all data up until the EOF as content.
    request_stream << "GET /" << location_ << "/" << remote_path.generic_string() << " HTTP/1.0\r\n"
                   << "Host: " << site_ << "\r\nAccept: */*\r\n"
                   << "Connection: close\r\n\r\n";
    // Send the request.
    asio::write(socket, request_buffer);
    // Read the response status line. The response streambuf will automatically grow to accommodate
    // the entire line. The growth may be limited by passing a maximum size to response_buffer ctor.
    asio::read_until(socket, response_buffer, "\r\n");
    // Check that response is OK.  Consumes entire header.
    if (!CheckResponse(remote_path, response_stream))
      return false;
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error preparing downloading of " << site_ << "/" << location_ << "/"
                << remote_path << "  : " << e.what();
    return false;
  }
  return true;
}

bool DownloadManager::CheckResponse(const fs::path& remote_path, std::istream& response_stream) {
  std::string http_version;
  response_stream >> http_version;
  unsigned int status_code;
  response_stream >> status_code;

  std::string status_message, header;
  while (std::getline(response_stream, header)) {
    if (header == "\r")
      break;
    status_message += header;
  }

  if (!response_stream || http_version.substr(0, 5) != "HTTP/" || status_code != 200) {
    LOG(kError) << "Error downloading " << site_ << "/" << location_ << "/" << remote_path
                << ".  Response header:\n" + status_message;
    return false;
  }
  return true;
}

std::string DownloadManager::DownloadFile(const fs::path& remote_path) {
  ip::tcp::socket socket(io_service_);
  asio::streambuf response_buffer;
  std::istream response_stream(&response_buffer);
  if (!PrepareDownload(remote_path, response_buffer, response_stream, socket))
    return "";

  try {
    // Read until EOF, puts whole file in memory, so this should be of manageable size.
    boost::system::error_code error_code;
    while (asio::read(socket, response_buffer, asio::transfer_at_least(1), error_code))
      boost::this_thread::interruption_point();
    if (error_code != asio::error::eof) {
      LOG(kWarning) << "Error downloading " << site_ << "/" << location_ << "/" << remote_path
                    << ": " << error_code.message();
      return "";
    }
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error downloading " << site_ << "/" << location_ << "/" << remote_path << ": "
                << e.what();
    return "";
  }

  return std::string(asio::buffer_cast<const char*>(response_buffer.data()),
                     response_buffer.size());
}

}  // namespace client_manager

}  // namespace maidsafe
