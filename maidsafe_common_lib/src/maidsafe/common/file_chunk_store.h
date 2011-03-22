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
#include <string>
#include <utility>
#include "boost/filesystem.hpp"

#include "maidsafe/common/chunk_store.h"

namespace fs = boost::filesystem;

namespace maidsafe {

//  pair of chunk count and total of all chunk sizes in a dir
typedef std::pair <std::uintmax_t, std::uintmax_t> RestoredChunkStoreInfo;

/**
 * Manages storage and retrieval of chunks using the file system.
 */
class FileChunkStore: public ChunkStore {
 public:
  explicit FileChunkStore(bool reference_counting)
      : ChunkStore(reference_counting),
        initialised_(false),
        storage_location_(),
        chunk_count_(0) {}
  ~FileChunkStore() {}

  /**
   * Initialises the chunk storage directory.
   *
   * If the given directory does not exist, it will be created.
   * @param storage_location Path to storage directory
   * @param directory depth
   * @return True if directory exists or could be created
   */
  bool Init(const fs::path &storage_location, int dir_depth = 5);

  /**
   * Retrieves a chunk's content as a string.
   * @param name Chunk name
   * @return Chunk content, or empty if non-existant
   */
  std::string Get(const std::string &name);

  /**
   * Retrieves a chunk's content as a file, potentially overwriting an existing
   * file of the same name.
   * @param name Chunk name
   * @param sink_file_name Path to output file
   * @return True if chunk exists and could be written to file.
   */
  bool Get(const std::string &name, const fs::path &sink_file_name);

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
   * Efficiently adds a locally existing chunk to another ChunkStore and
   * removes it from this one.
   * @param name Chunk name
   * @param sink_chunk_store The receiving ChunkStore
   * @return True if operation successful
   */
  bool MoveTo(const std::string &name,
                      ChunkStore *sink_chunk_store);

  /**
   * Checks if a chunk exists.
   * @param name Chunk name
   * @return True if chunk exists
   */
  bool Has(const std::string &name);

  /**
   * Validates a chunk, i.e. confirms if the name matches the content's hash.
   *
   * In case a chunk turns out to be invalid, it's advisable to delete it.
   * @param name Chunk name
   * @return True if chunk valid
   */
  bool Validate(const std::string &name);

  /**
   * Retrieves the size of a chunk.
   * @param name Chunk name
   * @return Size in bytes
   */
  std::uintmax_t Size(const std::string &name);

  /**
   * Retrieves the number of references to a chunk.
   *
   * If reference counting is enabled, this returns the number of (virtual)
   * copies of a chunk in the ChunkStore. Otherwise it would return 1 if the
   * chunks exists, or 0 if it doesn't.
   * @param name Chunk name
   * @return Reference count
   */
  std::uintmax_t Count(const std::string &name);

  /**
   * Retrieves the number of chunks held by this ChunkStore.
   * @return Chunk count
   */
  std::uintmax_t Count();

  /**
   * Checks if any chunks are held by this ChunkStore.
   * @return True if no chunks stored
   */
  bool Empty();

  /**
   * Deletes all stored chunks.
   */
  void Clear();

 private:
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
                               bool generate_dirs = false);

  void IncreaseChunkCount() { ++chunk_count_; }
  void DecreaseChunkCount() { --chunk_count_; }

  void ChunkAdded(const std::uintmax_t &delta);
  void ChunkRemoved(const std::uintmax_t &delta);

  void ResetChunkCount(std::uintmax_t chunk_count = 0) {
    chunk_count_ = chunk_count; }

  /**
   * Tries to read the dir specified and gets
   * total number of chunks and their collective
   * size
   */
  RestoredChunkStoreInfo RetrieveChunkInfo(const fs::path &location);

  bool IsChunkStoreInitialised() { return initialised_; }

  /**
   * Utility function
   * Returns reference count of a chunk
   * @param the absolute path of the chunk
   * @return the reference count for the chunk
   */
  std::uintmax_t GetChunkReferenceCount(const fs::path &);

  std::uintmax_t GetNumFromString(const std::string &);
  std::string GetStringFromNum(const std::uintmax_t &);

  std::string GetExtensionWithReferenceCount(const std::uintmax_t &);

  bool initialised_;
  fs::path storage_location_;
  std::uintmax_t chunk_count_;
  unsigned int dir_depth_;
};

}  //  namespace maidsafe

#endif  // MAIDSAFE_COMMON_FILE_CHUNK_STORE_H_
