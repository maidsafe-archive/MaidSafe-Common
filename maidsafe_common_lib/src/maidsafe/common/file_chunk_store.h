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

/**
 * @file file_chunk_store.h
 * @brief Implementation of FileChunkStore.
 */

#ifndef MAIDSAFE_COMMON_FILE_CHUNK_STORE_H_
#define MAIDSAFE_COMMON_FILE_CHUNK_STORE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4127)
#endif

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/token_functions.hpp"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/chunk_store.h"
#include "maidsafe/common/chunk_validation.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

namespace test {
class FileChunkStoreTest_BEH_Methods_Test;
}  // namespace test

/**
 * Manages storage and retrieval of chunks using the file system.
 */
class FileChunkStore: public ChunkStore {
 public:
  FileChunkStore(bool reference_counting,
                 std::shared_ptr<ChunkValidation> chunk_validation)
      : ChunkStore(reference_counting),
        chunk_validation_(chunk_validation),
        initialised_(false),
        storage_location_(),
        chunk_count_(0),
        dir_depth_(0),
        info_file_() {}
  ~FileChunkStore();

  /**
   * Initialises the chunk storage directory.
   *
   * If the given directory path does not exist, it will be created.
   * @param storage_location Path to storage directory
   * @param dir_depth directory depth
   * @return True if directory exists or could be created
   */
  bool Init(const fs::path &storage_location, unsigned int dir_depth = 5U);

  /**
   * Retrieves a chunk's content as a string.
   * @param name Chunk name
   * @return Chunk content, or empty if non-existant
   */
  std::string Get(const std::string &name) const;

  /**
   * Retrieves a chunk's content as a file, potentially overwriting an existing
   * file of the same name.
   * @param name Chunk name
   * @param sink_file_name Path to output file
   * @return True if chunk exists and could be written to file.
   */
  bool Get(const std::string &name, const fs::path &sink_file_name) const;

  /**
   * Stores chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's content
   * @return True if chunk could be stored or already existed
   */
  bool Store(const std::string &name, const std::string &content);

  /**
   * Stores chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param source_file_name Path to input file
   * @param delete_source_file True if file can be deleted after storing
   * @return True if chunk could be stored or already existed
   */
  bool Store(const std::string &name,
             const fs::path &source_file_name,
             bool delete_source_file);

  /**
   * Deletes a stored chunk.
   * @param name Chunk name
   * @return True if chunk deleted or non-existant
   */
  bool Delete(const std::string &name);

  /**
   * Modifies chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's modified content
   * @return True if chunk has been modified.
   */
  bool Modify(const std::string &name, const std::string &content);

  /**
   * Modifies a chunk's content as a file, potentially overwriting an existing
   * file of the same name.
   * @param name Chunk name
   * @param source_file_name Path to modified content file
   * @return True if chunk has been modified.
   */
  bool Modify(const std::string &name, const fs::path &source_file_name);
  /**
   * Efficiently adds a locally existing chunk to another ChunkStore and
   * removes it from this one.
   * @param name Chunk name
   * @param sink_chunk_store The receiving ChunkStore
   * @return True if operation successful
   */
  bool MoveTo(const std::string &name, ChunkStore *sink_chunk_store);

  /**
   * Checks if a chunk exists.
   * @param name Chunk name
   * @return True if chunk exists
   */
  bool Has(const std::string &name) const;

  /**
   * Validates a chunk, i.e. confirms if the name matches the content's hash.
   *
   * In case a chunk turns out to be invalid, it's advisable to delete it.
   * @param name Chunk name
   * @return True if chunk valid
   */
  bool Validate(const std::string &name) const;

  /**
   * Retrieves the size of a chunk.
   * @param name Chunk name
   * @return Size in bytes
   */
  std::uintmax_t Size(const std::string &name) const;

  /**
   * Retrieves the total size of the stored chunks.
   * @return Size in bytes
   */
  std::uintmax_t Size() const { return ChunkStore::Size(); }

  /**
   * Retrieves the number of references to a chunk.
   *
   * If reference counting is enabled, this returns the number of (virtual)
   * copies of a chunk in the ChunkStore. Otherwise it would return 1 if the
   * chunks exists, or 0 if it doesn't.
   * @param name Chunk name
   * @return Reference count
   */
  std::uintmax_t Count(const std::string &name) const;

  /**
   * Retrieves the number of chunks held by this ChunkStore.
   * @return Chunk count
   */
  std::uintmax_t Count() const;

  /**
   * Checks if any chunks are held by this ChunkStore.
   * @return True if no chunks stored
   */
  bool Empty() const;

  /**
   * Deletes all stored chunks.
   */
  void Clear();

  friend class test::FileChunkStoreTest_BEH_Methods_Test;

 private:
  typedef std::pair<std::uintmax_t, std::uintmax_t> RestoredChunkStoreInfo;

  FileChunkStore(const FileChunkStore&);
  FileChunkStore& operator=(const FileChunkStore&);

  /**
   * Utility function
   * Generates sub-dirs based on chunk-name and dir_depth_ specified
   * @param the chunk name in raw format
   * @param option used while storing a chunk - creates dir hierarchy
   * @return the absolute file path after encoding the chunk name as hex
   */
  fs::path ChunkNameToFilePath(const std::string &chunk_name,
                               bool generate_dirs = false) const;

  void IncreaseChunkCount() { ++chunk_count_; }
  void DecreaseChunkCount() { --chunk_count_; }

  void ChunkAdded(const std::uintmax_t &delta);
  void ChunkRemoved(const std::uintmax_t &delta);

  void ResetChunkCount(std::uintmax_t chunk_count = 0) {
    chunk_count_ = chunk_count;
  }

  /**
   * Tries to read the ChunkStore info file in
   * dir specified and gets total number of chunks
   * and their collective size
   */
  RestoredChunkStoreInfo RetrieveChunkInfo(const fs::path &location) const;

  /**
   * Saves the current state of the ChunkStore
   * (in terms of total number of chunks and their
   * collective size) to the info file
   */
  void SaveChunkStoreState();

  bool IsChunkStoreInitialised() const { return initialised_; }

  /**
   * Utility function
   * Returns reference count of a chunk
   * @param the absolute path of the chunk
   * @return the reference count for the chunk
   */
  std::uintmax_t GetChunkReferenceCount(const fs::path &) const;

  std::uintmax_t GetNumFromString(const std::string &) const;

  std::shared_ptr<ChunkValidation> chunk_validation_;
  bool initialised_;
  fs::path storage_location_;
  std::uintmax_t chunk_count_;
  unsigned int dir_depth_;
  fs::fstream info_file_;
};

}  //  namespace maidsafe

#endif  // MAIDSAFE_COMMON_FILE_CHUNK_STORE_H_
