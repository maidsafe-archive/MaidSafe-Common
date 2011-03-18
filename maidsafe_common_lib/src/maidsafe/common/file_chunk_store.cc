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

    if (ec || !file_size)
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
  bool success(false);

  return success;
}

bool FileChunkStore::Store(const std::string &name,
                           const fs::path &source_file_name,
                           bool delete_source_file) {
  bool success(false);

  return success;
}

bool FileChunkStore::Delete(const std::string &name) {
  bool success(false);

  return success;
}

bool FileChunkStore::MoveTo(const std::string &name,
                            ChunkStore *sink_chunk_store) {
  bool success(false);

  return success;
}

bool FileChunkStore::Has(const std::string &name) {
  bool found(false);

  return found;
}

std::uintmax_t FileChunkStore::Size(const std::string &name) {
  std::uintmax_t size_of_chunk(0);

  return size_of_chunk;
}

bool FileChunkStore::Validate(const std::string &name) {
  bool valid(false);

  return valid;
}

std::uintmax_t FileChunkStore::Count() {
  std::uintmax_t chunk_count(0);

  return chunk_count;
}

bool FileChunkStore::Empty() {
  bool is_empty(false);

  return is_empty;
}

void FileChunkStore::Clear() {
}

fs::path FileChunkStore::ChunkNameToFilePath(const std::string &chunk_name) {
  return fs::path(storage_location_ / EncodeToHex(chunk_name));
}

}  // namespace maidsafe
