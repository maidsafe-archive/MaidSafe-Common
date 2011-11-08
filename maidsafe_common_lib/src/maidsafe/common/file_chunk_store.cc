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

#include "boost/lexical_cast.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

FileChunkStore::~FileChunkStore() {
  info_file_.close();
}

bool FileChunkStore::Init(const fs::path &storage_location,
                          unsigned int dir_depth) {
  try {
    if (storage_location.empty())
      return false;

    if (fs::exists(storage_location)) {
      //  retrieve the number of chunks and total size
      RestoredChunkStoreInfo chunk_info = RetrieveChunkInfo(storage_location);
      ResetChunkCount(chunk_info.first);
      IncreaseSize(chunk_info.second);
    } else {
      if (!fs::create_directories(storage_location))
        return false;
      ResetChunkCount();
      ChunkStore::Clear();
    }

    storage_location_ = storage_location;
    dir_depth_ = dir_depth;
    std::string info_name("info");
    if (kReferenceCounting)
      info_name += "_ref";
    info_file_.open(storage_location_ / info_name,
                    std::ios_base::out | std::ios_base::trunc);
    SaveChunkStoreState();
    initialised_ = info_file_.good();
  }
  catch(const std::exception &e) {
    DLOG(ERROR) << "Init - " << e.what();
    return false;
  }
  return true;
}

std::string FileChunkStore::Get(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return "";

  if (name.empty())
    return "";

  fs::path file_path(ChunkNameToFilePath(name));
  std::uintmax_t ref_count(GetChunkReferenceCount(file_path));
  if (ref_count == 0)
    return "";

  if (kReferenceCounting)
    file_path.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

  std::string content;
  if (ReadFile(file_path, &content))
    return content;
  else
    return "";
}

bool FileChunkStore::Get(const std::string &name,
                         const fs::path &sink_file_name) const {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty() || sink_file_name.empty())
    return false;

  fs::path source_file_path(ChunkNameToFilePath(name));
  std::uintmax_t ref_count(GetChunkReferenceCount(source_file_path));
  if (ref_count == 0)
    return false;

  if (kReferenceCounting)
    source_file_path.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

  boost::system::error_code ec;
  fs::copy_file(source_file_path, sink_file_name,
                fs::copy_option::overwrite_if_exists, ec);
  return !ec;
}

bool FileChunkStore::Store(const std::string &name,
                           const std::string &content) {
  if (!IsChunkStoreInitialised())
    return false;

  if (!chunk_validation_ || !chunk_validation_->ValidName(name))
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name, true));
  if (kReferenceCounting) {
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    if (ref_count == 0) {
      //  new chunk!
      if (content.empty())
        return false;

      if (!Vacant(content.size()))
        return false;

      //  this is the first entry of this chunk
      chunk_file.replace_extension(".1");

      if (!WriteFile(chunk_file, content))
        return false;

      ChunkAdded(content.size());
      return true;
    } else {
      //  chunk already exists
      if (!chunk_validation_->Hashable(name))
        return false;
      fs::path old_path(chunk_file), new_path(chunk_file);
      old_path.replace_extension(
          "." + boost::lexical_cast<std::string>(ref_count));
      ++ref_count;
      new_path.replace_extension(
          "." + boost::lexical_cast<std::string>(ref_count));

      //  do a rename
      boost::system::error_code ec;
      fs::rename(old_path, new_path, ec);
      return !ec;
    }
  } else {
    //  Not using reference counting
    if (Has(name))
      return chunk_validation_->Hashable(name);

    if (content.empty())
      return false;

    if (!Vacant(content.size()))
      return false;

    if (!WriteFile(chunk_file, content))
      return false;

    ChunkAdded(content.size());
    return true;
  }
}

bool FileChunkStore::Store(const std::string &name,
                           const fs::path &source_file_name,
                           bool delete_source_file) {
  if (!IsChunkStoreInitialised())
    return false;

  if (!chunk_validation_ || !chunk_validation_->ValidName(name))
    return false;

  boost::system::error_code ec;
  fs::path chunk_file(ChunkNameToFilePath(name, true));

  if (kReferenceCounting) {
    //  retrieve the ref count based on extension
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    if (ref_count == 0) {
      //  new chunk!
      try {
        std::uintmax_t file_size(fs::file_size(source_file_name, ec));

        if (!Vacant(file_size))
          return false;

        //  this is the first entry of this chunk
        chunk_file.replace_extension(".1");

        if (delete_source_file)
          fs::rename(source_file_name, chunk_file);
        else
          fs::copy_file(source_file_name, chunk_file,
                        fs::copy_option::overwrite_if_exists);

        ChunkAdded(file_size);
        return true;
      }
      catch(...) {
        return false;
      }
    } else {
      //  chunk already exists
      if (!chunk_validation_->Hashable(name))
        return false;
      fs::path old_path(chunk_file), new_path(chunk_file);
      old_path.replace_extension(
          "." + boost::lexical_cast<std::string>(ref_count));
      ++ref_count;
      new_path.replace_extension(
          "." + boost::lexical_cast<std::string>(ref_count));

      //  do a rename
      fs::rename(old_path, new_path, ec);
      if (!ec) {
        if (delete_source_file)
          fs::remove(source_file_name, ec);
        return true;
      }
    }
  } else {
    //  not using reference counting

    //  does the chunk already exist
    if (!fs::exists(chunk_file, ec)) {
      std::uintmax_t file_size(fs::file_size(source_file_name, ec));

      //  is source file valid
      if (ec || file_size == 0)
        return false;

      if (!Vacant(file_size))
        return false;

      if (delete_source_file)
        fs::rename(source_file_name, chunk_file, ec);
      else
        fs::copy_file(source_file_name, chunk_file,
                      fs::copy_option::overwrite_if_exists, ec);
      if (!ec) {
        ChunkAdded(file_size);
        return true;
      }
    } else {
      if (!chunk_validation_->Hashable(name))
        return false;
      if (delete_source_file)
        fs::remove(source_file_name, ec);
      return true;
    }
  }
  return false;
}

bool FileChunkStore::Delete(const std::string &name) {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  if (kReferenceCounting) {
    fs::path chunk_file(ChunkNameToFilePath(name));
    boost::system::error_code ec;

    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    //  if file does not exist
    if (ref_count == 0)
      return true;

    chunk_file.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

    //  check if last reference
    if (ref_count == 1) {
      std::uintmax_t file_size(fs::file_size(chunk_file, ec));
      fs::remove(chunk_file, ec);

      if (!ec) {
        ChunkRemoved(file_size);
        return true;
      }
    } else {
      //  reduce the reference counter, but retain the file
      --ref_count;
      fs::path new_chunk_path(chunk_file);
      new_chunk_path.replace_extension(
          "." + boost::lexical_cast<std::string>(ref_count));

      //  do a rename
      fs::rename(chunk_file, new_chunk_path, ec);
      if (!ec)
        return true;
    }
  } else {
    //  non reference counting chunk store
    //  check non existant chunk
    if (!Has(name))
      return true;

    fs::path chunk_file(ChunkNameToFilePath(name));

    boost::system::error_code ec;
    std::uintmax_t file_size(fs::file_size(chunk_file, ec));
    fs::remove(chunk_file, ec);

    if (!ec) {
      ChunkRemoved(file_size);
      return true;
    }
  }
  return false;
}

bool FileChunkStore::MoveTo(const std::string &name,
                            ChunkStore *sink_chunk_store) {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty() || !sink_chunk_store)
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name));
  if (kReferenceCounting) {
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    //  this store does not have the file
    //  Not calling Has here to avoid two calls to GetChunkReferenceCount
    if (ref_count == 0)
      return false;

    chunk_file.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

    if (ref_count == 1) {
      // avoid copy on last reference
      boost::system::error_code ec;
      std::uintmax_t size = fs::file_size(chunk_file, ec);

      if (ec || size == 0)
        return false;

      if (sink_chunk_store->Store(name, chunk_file, true)) {
        ChunkRemoved(size);
        return true;
      }
    } else {
      if (sink_chunk_store->Store(name, chunk_file, false)) {
        Delete(name);
        return true;
      }
    }
  } else {
    if (!Has(name))
      return false;

    boost::system::error_code ec;
    std::uintmax_t size = fs::file_size(chunk_file, ec);

    if (ec || size == 0)
      return false;

    if (sink_chunk_store->Store(name, chunk_file, true)) {
      ChunkRemoved(size);
      return true;
    }
  }
  return false;
}

bool FileChunkStore::Has(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  return GetChunkReferenceCount(ChunkNameToFilePath(name)) > 0;
}

bool FileChunkStore::Validate(const std::string &name) const {
  if (!chunk_validation_)
    return false;

  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name));
  std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));
  if (ref_count == 0)
    return false;

  if (kReferenceCounting)
    chunk_file.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

  return chunk_validation_->ValidChunk(name, chunk_file);
}

std::string FileChunkStore::Version(const std::string &name) const {
  if (!chunk_validation_)
    return "";

  if (!IsChunkStoreInitialised())
    return "";

  if (name.empty())
    return "";

  fs::path chunk_file(ChunkNameToFilePath(name));
  std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));
  if (ref_count == 0)
    return "";

  if (kReferenceCounting)
    chunk_file.replace_extension(
        "." + boost::lexical_cast<std::string>(ref_count));

  return chunk_validation_->Version(name, chunk_file);
}

std::uintmax_t FileChunkStore::Size(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return 0;

  if (name.empty())
    return 0;

  fs::path chunk_file(ChunkNameToFilePath(name));
  if (kReferenceCounting)
    chunk_file.replace_extension("." + boost::lexical_cast<std::string>(
        GetChunkReferenceCount(chunk_file)));

  boost::system::error_code ec;
  std::uintmax_t size = fs::file_size(chunk_file, ec);
  if (!ec)
    return size;
  return 0;
}

std::uintmax_t FileChunkStore::Count() const {
  if (!IsChunkStoreInitialised())
    return 0;

  return chunk_count_;
}

std::uintmax_t FileChunkStore::Count(const std::string &name) const {
  if (!IsChunkStoreInitialised() || name.empty())
    return 0;

  return GetChunkReferenceCount(ChunkNameToFilePath(name, false));
}

bool FileChunkStore::Empty() const {
  return !IsChunkStoreInitialised() || chunk_count_ == 0;
}

void FileChunkStore::Clear() {
  info_file_.close();
  ResetChunkCount();
  fs::remove_all(storage_location_);
  ChunkStore::Clear();
}

fs::path FileChunkStore::ChunkNameToFilePath(const std::string &chunk_name,
                                             bool generate_dirs) const {
  //  std::string encoded_file_name = EncodeToHex(chunk_name);
  std::string encoded_file_name = EncodeToBase32(chunk_name);

  unsigned int dir_depth_for_chunk = dir_depth_;
  if (encoded_file_name.length() < dir_depth_for_chunk) {
    dir_depth_for_chunk =
        static_cast<unsigned int>(encoded_file_name.length()) - 1;
  }

  fs::path storage_dir(storage_location_);
  for (unsigned int i = 0; i < dir_depth_for_chunk; ++i)
    storage_dir /= encoded_file_name.substr(i, 1);

  if (generate_dirs) {
    boost::system::error_code ec;
    fs::create_directories(storage_dir, ec);
  }

  return fs::path(storage_dir / encoded_file_name.substr(dir_depth_for_chunk));
}

FileChunkStore::RestoredChunkStoreInfo FileChunkStore::RetrieveChunkInfo(
    const fs::path &location) const {
  RestoredChunkStoreInfo chunk_store_info;
  chunk_store_info.first = 0;
  chunk_store_info.second = 0;

  std::string info_name("info");
  if (kReferenceCounting)
    info_name += "_ref";
  fs::fstream info(location / info_name, std::ios_base::in);
  if (info.good())
    info >> chunk_store_info.first >> chunk_store_info.second;

  info.close();

  return chunk_store_info;
}

void FileChunkStore::SaveChunkStoreState() {
  info_file_.seekp(0, std::ios_base::beg);
  info_file_ << chunk_count_ << std::endl << ChunkStore::Size();
  info_file_.flush();
}

void FileChunkStore::ChunkAdded(const std::uintmax_t &delta) {
  IncreaseSize(delta);
  IncreaseChunkCount();
  SaveChunkStoreState();
}

void FileChunkStore::ChunkRemoved(const std::uintmax_t &delta) {
  DecreaseSize(delta);
  DecreaseChunkCount();
  SaveChunkStoreState();
}

/**
 * Directory Iteration is required
 * The function receives a chunk name without extension
 * To get the file's ref count (extension), each file in the
 * dir needs to be checked for match after removing its extension
 *
 * @todo Add ability to merge reference counts of multiple copies of same chunk
 */
std::uintmax_t FileChunkStore::GetChunkReferenceCount(
    const fs::path &chunk_path) const {
  boost::system::error_code ec;

  if (!kReferenceCounting)
    return fs::exists(chunk_path, ec) ? 1 : 0;

  if (!fs::exists(chunk_path.parent_path(), ec))
    return 0;

  try {
    for (fs::directory_iterator it(chunk_path.parent_path());
        it != fs::directory_iterator(); ++it) {
      if (fs::is_regular_file(it->status())) {
        std::string ext(it->path().extension().string());
        //  check if file was stored without ref count, ignore it
        if (ext.empty())
          continue;

        if (it->path().stem() == chunk_path.filename())
          return GetNumFromString(ext.substr(1));
      }
    }
  }
  catch(const std::exception &e) {
    DLOG(ERROR) << "GetChunkReferenceCount - " << e.what();
  }

  return 0;
}

std::uintmax_t FileChunkStore::GetNumFromString(const std::string &str) const {
  try {
    return boost::lexical_cast<uintmax_t>(str);
  } catch(boost::bad_lexical_cast &) {
    return 0;
  }
}

}  // namespace maidsafe
