/* Copyright (c) 2011 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/file_chunk_store.h"
#include "maidsafe/common/utils.h"

#include "boost/filesystem/fstream.hpp"

namespace fs = boost::filesystem;

namespace maidsafe {

std::string FileChunkStore::Get(const std::string &name) {

  fs::path file_path(ChunkNameToFilePath(name));

  boost::system::error_code ec;

  if (fs::exists(file_path, ec)) {
    if (ec)
      return "";

    std::uintmax_t file_size(fs::file_size(file_path, ec));

    if (!file_size || ec)
      return "";

    fs::ifstream file_in(file_path, std::ios::in | std::ios::binary);

    if (!file_in.good())
      return "";

    std::string chunk_content;
    chunk_content.resize(file_size);

    file_in.read(&((chunk_content)[0]), file_size);

    file_in.close();

    return chunk_content;
  }

  return "";
}

bool FileChunkStore::Get(const std::string &name,
                         const fs::path &sink_file_name) {

  fs::path source_file_path(ChunkNameToFilePath(name));

  boost::system::error_code ec;

  if (fs::exists(source_file_path, ec)) {
    if (ec)
      return false;
    fs::copy_file(source_file_path, sink_file_name,
                  fs::copy_option::overwrite_if_exists, ec);
    if (ec)
      return false;

    return true;
  }

  return false;
}

bool FileChunkStore::Store(const std::string &name,
                           const std::string &content) {
  if (content.empty() || name.empty())
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name));
  boost::system::error_code ec;

  if (fs::exists(chunk_file, ec))
    return true;

  //if (ec)
  //  return false;

  try {
    fs::ofstream file_out(chunk_file,
                        std::ios::out | std::ios::trunc | std::ios::binary);

    file_out.write(content.data(), content.size());
    file_out.close();
    return true;
  } catch(...) {
    return false;
  }
  return false;
}

bool FileChunkStore::Store(const std::string &name,
                           const fs::path &source_file_name,
                           bool delete_source_file) {

  if (name.empty())
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name));
  boost::system::error_code ec;

  //  does the chunk already exist
  if (!fs::exists(chunk_file, ec)) {

    std::uintmax_t file_size(fs::file_size(source_file_name, ec));

    //  is source file valid
    if (file_size && !ec) {

      if (delete_source_file)
        fs::rename(source_file_name, chunk_file, ec);
      else
        fs::copy_file(source_file_name, chunk_file,
                  fs::copy_option::overwrite_if_exists, ec);

      if (!ec)
        return true;
    }
  } else {
    if (delete_source_file)
      fs::remove(source_file_name, ec);
    return true;
  }
  return false;
}

bool FileChunkStore::Delete(const std::string &name) {
  fs::path chunk_file(ChunkNameToFilePath(name));
  boost::system::error_code ec;

  fs::remove(chunk_file, ec);

  if (ec)
    return false;

  return true;
}

bool FileChunkStore::MoveTo(const std::string &name,
                            ChunkStore *sink_chunk_store) {
  bool success(false);

  return success;
}

bool FileChunkStore::Has(const std::string &name) {
  fs::path chunk_file(ChunkNameToFilePath(name));
  boost::system::error_code ec;

  if (fs::exists(chunk_file, ec))
    return true;
  return false;
}

std::uintmax_t FileChunkStore::Size(const std::string &name) {
  if (name.empty())
    return 0;

  try {
    fs::path chunk_file(ChunkNameToFilePath(name));
    return fs::file_size(chunk_file);
  } catch (...) {
    return 0;
  }
}

bool FileChunkStore::Validate(const std::string &name) {
  bool valid(false);

  return valid;
}

std::uintmax_t FileChunkStore::Count() {
  return GetChunkCount(storage_location_);
}

bool FileChunkStore::Empty() {
  if (fs::is_empty(storage_location_))
    return true;
  return false;
}

void FileChunkStore::Clear() {
}

fs::path FileChunkStore::ChunkNameToFilePath(const std::string &chunk_name) {
  return fs::path(storage_location_ / EncodeToHex(chunk_name));
}

std::uintmax_t FileChunkStore::GetChunkCount(const fs::path &location) {
  boost::uintmax_t count(0);

  for (fs::directory_iterator it(location);
       it != boost::filesystem::directory_iterator(); ++it) {
    if (boost::filesystem::is_regular_file(it->status())) {
      ++count;
      } else if (fs::is_directory(it->path())) {
        count += GetChunkCount(it->path());
      }
    }
  return count;
}

}  // namespace maidsafe
