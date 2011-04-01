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
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

#include "boost/filesystem/fstream.hpp"
#include "boost/lexical_cast.hpp"

namespace fs = boost::filesystem;

namespace maidsafe {

bool FileChunkStore::Init(const fs::path &storage_location,
                          int dir_depth) {
  try {
    if (storage_location.string().empty())
      return false;

    if (fs::exists(storage_location)) {
      //  retrieve the number of chunks and total size
      RestoredChunkStoreInfo chunk_info = RetrieveChunkInfo(storage_location);
      ResetChunkCount(chunk_info.first);
      //  assuming capacity as infinite as it is set to 0 in ChunkStore ctor
      IncreaseSize(chunk_info.second);
    } else {
      if (fs::create_directory(storage_location)) {
        ResetChunkCount();

        ChunkStore::Clear();
      } else {
        return false;
      }
    }
    ChunkStore::SetCapacity(0);
    storage_location_ = storage_location;
    dir_depth_ = dir_depth;
    initialised_ = true;
  } catch(const std::exception &e) {
    DLOG(ERROR) << "FileChunkStore::Init - " << e.what() << std::endl;
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

  if (kReferenceCounting) {
    std::uintmax_t ref_count(GetChunkReferenceCount(file_path));
    if (ref_count == 0)
      return "";

    file_path = file_path.string() + GetExtensionWithReferenceCount(ref_count);
  }

  boost::system::error_code ec;
  if (fs::exists(file_path, ec)) {
    if (!ec) {
      std::string content;
      if (ReadFile(file_path, &content))
        return content;
    }
  }
  return "";
}

bool FileChunkStore::Get(const std::string &name,
                         const fs::path &sink_file_name) const {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty() || sink_file_name.string().empty())
    return false;

  fs::path source_file_path(ChunkNameToFilePath(name));
  if (kReferenceCounting) {
    std::uintmax_t ref_count(GetChunkReferenceCount(source_file_path));
    if (ref_count == 0)
      return false;

    source_file_path = source_file_path.string() +
                       GetExtensionWithReferenceCount(ref_count);
  }

  boost::system::error_code ec;
  if (fs::exists(source_file_path, ec)) {
    if (!ec) {
      ec.clear();
      fs::copy_file(source_file_path, sink_file_name,
                    fs::copy_option::overwrite_if_exists, ec);
      if (!ec)
        return true;
    }
  }
  return false;
}

bool FileChunkStore::Store(const std::string &name,
                           const std::string &content) {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  if (kReferenceCounting) {
    fs::path chunk_file(ChunkNameToFilePath(name, true));

    //  retrieve the ref count based on extension
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    if (ref_count == 0) {
      //  new chunk!
      if (content.empty())
        return false;

      if (!Vacant(content.size()))
        return false;

      //  this is the first entry of this chunk
      chunk_file = chunk_file.string() + GetExtensionWithReferenceCount(1);

      if (!WriteFile(chunk_file, content))
        return false;

      ChunkAdded(content.size());
      return true;
    } else {
      //  chunk already exists
      std::string existing_file(chunk_file.string() +
                                    GetExtensionWithReferenceCount(ref_count));

      //  ref count for new file name
      ++ref_count;

      std::string file_to_write(chunk_file.string() +
                                    GetExtensionWithReferenceCount(ref_count));

      //  do a rename
      boost::system::error_code ec;
      fs::rename(fs::path(existing_file), fs::path(file_to_write), ec);
      if (!ec)
        return true;
    }
  } else {
    //  Not using reference counting
    if (Has(name))
      return true;

    fs::path chunk_file(ChunkNameToFilePath(name, true));

    if (content.empty())
      return false;

    if (!Vacant(content.size()))
      return false;

    if (!WriteFile(chunk_file, content))
      return false;

    ChunkAdded(content.size());
    return true;
  }
  return false;
}

bool FileChunkStore::Store(const std::string &name,
                           const fs::path &source_file_name,
                           bool delete_source_file) {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name, true));
  boost::system::error_code ec;

  if (kReferenceCounting) {
    //  retrieve the ref count based on extension
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    if (ref_count == 0) {
      //  new chunk!
      try {
        std::uintmax_t file_size(fs::file_size(source_file_name));

        if (!Vacant(file_size))
            return false;

        //  this is the first entry of this chunk
        std::string file_to_write_str(chunk_file.string() +
                                      GetExtensionWithReferenceCount(1));

        chunk_file = file_to_write_str;

        if (delete_source_file)
          fs::rename(source_file_name, chunk_file);
        else
          fs::copy_file(source_file_name, chunk_file,
                        fs::copy_option::overwrite_if_exists);

        ChunkAdded(file_size);
        return true;
      } catch(...) {
          return false;
      }
    } else {
      //  chunk already exists
      std::string existing_file(chunk_file.string() +
                                    GetExtensionWithReferenceCount(ref_count));

      //  ref count for new file name
      ++ref_count;

      std::string file_to_write(chunk_file.string() +
                                    GetExtensionWithReferenceCount(ref_count));

      //  do a rename
      boost::system::error_code ec;
      fs::rename(fs::path(existing_file), fs::path(file_to_write), ec);
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
      if (file_size && !ec) {
        if (!Vacant(file_size))
          return false;

        boost::system::error_code ec;
        if (delete_source_file)
          fs::rename(source_file_name, chunk_file, ec);
        else
          fs::copy_file(source_file_name, chunk_file,
                    fs::copy_option::overwrite_if_exists, ec);
        if (!ec) {
          ChunkAdded(file_size);
          return true;
        }
      }
    } else {
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

    //  chunk already exists
    std::string existing_file(chunk_file.string() +
                              GetExtensionWithReferenceCount(ref_count));

    //  check if last reference
    if (ref_count == 1) {
      chunk_file = fs::path(existing_file);

      std::uintmax_t file_size(fs::file_size(chunk_file, ec));
      fs::remove(chunk_file, ec);

      if (!ec) {
        ChunkRemoved(file_size);
        return true;
      }
    } else {
      //  reduce the reference counter, but retain the file
      --ref_count;

      std::string file_to_write(chunk_file.string() +
                                GetExtensionWithReferenceCount(ref_count));

      //  do a rename
      boost::system::error_code ec;
      fs::rename(fs::path(existing_file), fs::path(file_to_write), ec);
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

  if (kReferenceCounting) {
    fs::path chunk_file(ChunkNameToFilePath(name));
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));

    //  this store does not have the file
    //  Not calling Has here to avoid two calls to GetChunkReferenceCount
    if (ref_count == 0)
      return false;

    fs::path existing_file(chunk_file.string() +
                           GetExtensionWithReferenceCount(ref_count));

    if (sink_chunk_store->Store(name, existing_file, 0)) {
      Delete(name);
      return true;
    }
  } else {
    if (!Has(name))
      return false;

    fs::path chunk_file(ChunkNameToFilePath(name));
    boost::system::error_code ec;

    std::uintmax_t file_size(Size(name));

    if (file_size) {
      if (sink_chunk_store->Store(name, chunk_file, 1)) {
        ChunkRemoved(file_size);
        return true;
      }
    }
  }
  return false;
}

bool FileChunkStore::Has(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  fs::path chunk_file(ChunkNameToFilePath(name));
  boost::system::error_code ec;

  if (kReferenceCounting) {
    if (GetChunkReferenceCount(chunk_file) != 0) {
      return true;
    }
  } else if (fs::exists(chunk_file, ec)) {
    return true;
  }

  return false;
}

bool FileChunkStore::Validate(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return false;

  if (name.empty())
    return false;

  if (kReferenceCounting) {
    fs::path chunk_file(ChunkNameToFilePath(name));
    std::uintmax_t ref_count(GetChunkReferenceCount(chunk_file));
    std::string existing_file(chunk_file.string() +
                              GetExtensionWithReferenceCount(ref_count));

    return name == crypto::HashFile<crypto::SHA512>(fs::path(existing_file));
  } else {
    return name == crypto::HashFile<crypto::SHA512>(ChunkNameToFilePath(name));
  }
}

std::uintmax_t FileChunkStore::Size(const std::string &name) const {
  if (!IsChunkStoreInitialised())
    return 0;

  if (name.empty())
    return 0;

  boost::system::error_code ec;

  fs::path chunk_file(ChunkNameToFilePath(name));
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

  if (kReferenceCounting) {
    fs::path chunk_file(ChunkNameToFilePath(name, false));

    //  retrieve the ref count based on extension
    return GetChunkReferenceCount(chunk_file);
  }

  return Count();
}

bool FileChunkStore::Empty() const {
  if (!IsChunkStoreInitialised() || chunk_count_ == 0)
    return true;

  return false;
}

void FileChunkStore::Clear() {
  ResetChunkCount();
  fs::remove_all(storage_location_);
  ChunkStore::Clear();
}

fs::path FileChunkStore::ChunkNameToFilePath(const std::string &chunk_name,
                                             bool generate_dirs) const {
  //  std::string encoded_file_name = EncodeToHex(chunk_name);
  std::string encoded_file_name = EncodeToBase32(chunk_name);

  std::string dir_names;
  unsigned int dir_depth_for_chunk = dir_depth_;

  if (encoded_file_name.length() < dir_depth_for_chunk) {
    dir_depth_for_chunk = encoded_file_name.length() - 1;
  }

  for (unsigned int i = 0; i < dir_depth_for_chunk; ++i) {
    dir_names.push_back('/');
    dir_names.push_back(encoded_file_name[i]);
  }

  dir_names.push_back('/');

  encoded_file_name = encoded_file_name.erase(0, dir_depth_for_chunk);

  boost::system::error_code ec;
  if (generate_dirs) {
    fs::create_directories(fs::path(storage_location_.string() + dir_names),
                           ec);
  }

  //  create the absolute file path
  dir_names += encoded_file_name;

  return fs::path(storage_location_.string() + dir_names);
}

RestoredChunkStoreInfo FileChunkStore::RetrieveChunkInfo(
    const fs::path &location) const {
  boost::uintmax_t count(0);
  RestoredChunkStoreInfo chunk_store_info;
  chunk_store_info.first = 0;
  chunk_store_info.second = 0;

  try {
    for (fs::directory_iterator it(location);
         it != boost::filesystem::directory_iterator(); ++it) {
      if (boost::filesystem::is_regular_file(it->status())) {
        bool include_size(false);
        std::uintmax_t ref_count(GetChunkReferenceCount(it->path()));

        if ((kReferenceCounting && ref_count > 0) ||
            (!kReferenceCounting && ref_count == 0)) {
          ++chunk_store_info.first;
          chunk_store_info.second += fs::file_size(*it);
        }
      } else if (fs::is_directory(it->path())) {
        RestoredChunkStoreInfo info = RetrieveChunkInfo(it->path());
        chunk_store_info.first += info.first;
        chunk_store_info.second += info.second;
      }
    }
  } catch(...) {}
  return chunk_store_info;
}

void FileChunkStore::ChunkAdded(const std::uintmax_t &delta) {
  IncreaseSize(delta);
  IncreaseChunkCount();
}

void FileChunkStore::ChunkRemoved(const std::uintmax_t &delta) {
  DecreaseSize(delta);
  DecreaseChunkCount();
}

/**
 * Directory Iteration is required
 * The function receives a chunk name without extension
 * To get the file's ref count (extension), each file in the
 * dir needs to be checked for match after removing its extension
 */
std::uintmax_t FileChunkStore::GetChunkReferenceCount(
    const fs::path &chunk_path) const {
  fs::path location(chunk_path.parent_path());

  boost::system::error_code ec;
  fs::file_status file_stat = fs::status(location, ec);
  if (file_stat == fs::file_status(fs::file_not_found))
    return 0;

  std::string chunk_name(chunk_path.filename().string());

  for (fs::directory_iterator it(location);
       it != fs::directory_iterator(); ++it) {
    ec.clear();
    if (fs::is_regular_file(it->status())) {
      std::string ext(it->path().extension().string());
      //  check if file was stored without ref count, ignore it
      if (ext.length() == 0)
        continue;

      //  the chunk_name should be the file name minus the ext
      if (it->path().filename().string().find(chunk_name) !=
          std::string::npos) {
        //  remove the dot from the extension
        ext.erase(0, 1);
        return GetNumFromString(ext);
      }
    }
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

std::string FileChunkStore::GetStringFromNum(
    const std::uintmax_t &count) const {
  try {
    return boost::lexical_cast<std::string>(count);
  } catch(const boost::bad_lexical_cast&) {
    return "";
  }
}

std::string FileChunkStore::GetExtensionWithReferenceCount(
    const std::uintmax_t &ref_count) const {
  return std::string("." + GetStringFromNum(ref_count));
}

}  // namespace maidsafe
