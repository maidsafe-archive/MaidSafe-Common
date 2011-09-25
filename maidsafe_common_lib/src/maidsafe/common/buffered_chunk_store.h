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
 * @file buffered_chunk_store.h
 * @brief Implementation of BufferedChunkStore.
 */

#ifndef MAIDSAFE_COMMON_BUFFERED_CHUNK_STORE_H_
#define MAIDSAFE_COMMON_BUFFERED_CHUNK_STORE_H_

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <string>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4127)
#endif

#include "boost/asio.hpp"
#include "boost/asio/basic_deadline_timer.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/token_functions.hpp"
#include "boost/thread/shared_mutex.hpp"
#include "boost/thread/locks.hpp"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/memory_chunk_store.h"
#include "maidsafe/common/file_chunk_store.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1002
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;
namespace arg = std::placeholders;
namespace maidsafe {

struct ChunkDetails {
  ChunkDetails(const std::string &chunk_name,
               const boost::posix_time::ptime &expire_time)
      : name(chunk_name),
        expiry_time(expire_time) {}
  std::string name;
  boost::posix_time::ptime expiry_time;
};
const boost::posix_time::minutes kCleanupTimer(30);
/**
 * Manages storage and retrieval of chunks using in-memory data structures.
 */
class BufferedChunkStore: public ChunkStore {
 public:
  typedef std::function<std::string(fs::path)> FileHashFunc;
  typedef std::function<std::string(std::string)> MemoryHashFunc;
  BufferedChunkStore(bool reference_counting, FileHashFunc file_hash_func,
                     MemoryHashFunc memory_hash_func,
                     boost::asio::io_service &asio_service)
      : ChunkStore(reference_counting),
        shared_mutex_(),
        asio_service_(asio_service),
        cleanup_timer_(asio_service_),
        file_hash_func_(file_hash_func),
        memory_hash_func_(memory_hash_func),
        memory_chunk_store_(new MemoryChunkStore(false, memory_hash_func_)),
        file_chunk_store_(new FileChunkStore(false, file_hash_func_)),
        chunk_details_() {
    cleanup_timer_.expires_from_now(kCleanupTimer);
      cleanup_timer_.async_wait(std::bind(
          &BufferedChunkStore::CleanExpiredChunks, this, arg::_1));
  }
  ~BufferedChunkStore() {}

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
   * Stores chunk content under the given name in FileChunkStore.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's content
   * @return True if chunk could be stored or already existed
   */
  bool Store(const std::string &name, const std::string &content);

  /**
   * Stores chunk content under the given name in FileChunkStore.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param source_file_name Path to input file
   * @param delete_source_file True if file can be deleted after storing
   * @return True if chunk could be stored or already existed
   */
  bool Store(const std::string &name,
             const fs::path &source_file_name,
             bool delete_source_file);

   /**
   * Stores chunk content under the given name in MemoryChunkStore.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's content
   * @return True if chunk could be stored or already existed
   */
  bool StoreCached(const std::string &name, const std::string &content);

  /**
   * Stores chunk content under the given name in MemoryChunkStore.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param source_file_name Path to input file
   * @param delete_source_file True if file can be deleted after storing
   * @return True if chunk could be stored or already existed
   */
  bool StoreCached(const std::string &name,
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
  BufferedChunkStore(const BufferedChunkStore&);
  BufferedChunkStore& operator=(const BufferedChunkStore&);

  void CleanExpiredChunks(const boost::system::error_code &ec);
  bool CompareChunkName(const ChunkDetails &chunk_detail) const;
  void StoreInFile(const std::string &name, const std::string &contents);
  void StoreInFile(const std::string &name, const fs::path &source_file_name,
                   bool delete_source_file);
  typedef boost::shared_lock<boost::shared_mutex> SharedLock;
  typedef boost::upgrade_lock<boost::shared_mutex> UpgradeLock;
  typedef boost::unique_lock<boost::shared_mutex> UniqueLock;
  typedef boost::upgrade_to_unique_lock<boost::shared_mutex>
      UpgradeToUniqueLock;
  mutable boost::shared_mutex shared_mutex_;
  boost::asio::deadline_timer cleanup_timer_;
  boost::asio::io_service &asio_service_;
  FileHashFunc file_hash_func_;
  MemoryHashFunc memory_hash_func_;
  std::shared_ptr<ChunkStore> memory_chunk_store_;
  std::shared_ptr<ChunkStore> file_chunk_store_;
  mutable std::list<ChunkDetails> chunk_details_;
};

}  //  namespace maidsafe

#endif  // MAIDSAFE_COMMON_BUFFERED_CHUNK_STORE_H_
