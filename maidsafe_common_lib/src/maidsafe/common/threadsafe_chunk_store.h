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
 * @file threadsafe_chunk_store.h
 * @brief Declaration of ThreadsafeChunkStore interface.
 */

#ifndef MAIDSAFE_COMMON_THREADSAFE_CHUNK_STORE_H_
#define MAIDSAFE_COMMON_THREADSAFE_CHUNK_STORE_H_

#include <cstdint>
#include <string>
#include <utility>

#include "boost/filesystem.hpp"
#include "boost/thread/shared_mutex.hpp"
#include "boost/thread/locks.hpp"

#include "maidsafe/common/chunk_store.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

/**
 * Concrete threadsafe class to manage storage and retrieval of chunks.  The
 * class implements shared mutex locking around another concrete ChunkStore.
 */
class ThreadsafeChunkStore : public ChunkStore {
 public:
  explicit ThreadsafeChunkStore(std::shared_ptr<ChunkStore> chunk_store)
      : ChunkStore(false),
        chunk_store_(chunk_store),
        shared_mutex_() {}

  ~ThreadsafeChunkStore() {}

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
  bool Modify(const std::string &name,
              const fs::path &source_file_name,
              bool delete_source_file);

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
   * Validates a chunk using the ChunkValidation object.
   *
   * In case a chunk turns out to be invalid, it's advisable to delete it.
   * @param name Chunk name
   * @return True if chunk valid
   */
  bool Validate(const std::string &name) const;

  /**
   * Retrieves the chunk's content version using the ChunkValidation object.
   * @param name Chunk name
   * @return The chunk version
   */
  std::string Version(const std::string &name) const;

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
  std::uintmax_t Size() const;

  /**
   * Retrieves the maximum storage capacity available to this ChunkStore.
   *
   * A capacity of zero (0) equals infinite storage space.
   * @return Capacity in bytes
   */
  std::uintmax_t Capacity() const;

  /**
   * Sets the maximum storage capacity available to this ChunkStore.
   *
   * A capacity of zero (0) equals infinite storage space. The capacity must
   * always be at least as high as the total size of already stored chunks.
   * @param capacity Capacity in bytes
   */
  void SetCapacity(const std::uintmax_t &capacity);

  /**
   * Checks whether the ChunkStore has enough capacity to store a chunk of the
   * given size.
   * @return True if required size vacant
   */
  bool Vacant(const std::uintmax_t &required_size) const;

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

 private:
  ThreadsafeChunkStore(const ThreadsafeChunkStore&);
  ThreadsafeChunkStore& operator=(const ThreadsafeChunkStore&);

  typedef boost::shared_lock<boost::shared_mutex> SharedLock;
  typedef boost::unique_lock<boost::shared_mutex> UniqueLock;

  std::shared_ptr<ChunkStore> chunk_store_;
  mutable boost::shared_mutex shared_mutex_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_THREADSAFE_CHUNK_STORE_H_
