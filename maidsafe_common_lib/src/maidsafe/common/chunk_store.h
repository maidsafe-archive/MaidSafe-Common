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
 * @file chunk_store.h
 * @brief Declaration of ChunkStore interface.
 */

#ifndef MAIDSAFE_COMMON_CHUNK_STORE_H_
#define MAIDSAFE_COMMON_CHUNK_STORE_H_

#include <cstdint>
#include <string>

#include "boost/filesystem.hpp"
#include "maidsafe/common/alternative_store.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

/**
 * Abstract class to manage storage and retrieval of named data items (chunks).
 *
 * A chunk is a small, content-adressable piece of data that can be validated
 * using an implementation-specific mechanism.
 *
 * The storage capacity can be limited by setting Capacity to a value greater
 * than zero. If that limit is reached, further Store operations will fail. A
 * value of zero (the default) equals infinite storage capacity.
 */
class ChunkStore : public AlternativeStore {
 public:
  explicit ChunkStore(bool reference_counting)
      : AlternativeStore(),
        kReferenceCounting(reference_counting),
        capacity_(0),
        size_(0) {}
  virtual ~ChunkStore() {}

  /**
   * Retrieves a chunk's content as a string.
   * @param name Chunk name
   * @return Chunk content, or empty if non-existant
   */
  virtual std::string Get(const std::string &name) const = 0;

  /**
   * Retrieves a chunk's content as a file, potentially overwriting an existing
   * file of the same name.
   * @param name Chunk name
   * @param sink_file_name Path to output file
   * @return True if chunk exists and could be written to file.
   */
  virtual bool Get(const std::string &name,
                   const fs::path &sink_file_name) const = 0;

  /**
   * Stores chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's content
   * @return True if chunk could be stored or already existed
   */
  virtual bool Store(const std::string &name, const std::string &content) = 0;

  /**
   * Stores chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param source_file_name Path to input file
   * @param delete_source_file True if file can be deleted after storing
   * @return True if chunk could be stored or already existed
   */
  virtual bool Store(const std::string &name,
                     const fs::path &source_file_name,
                     bool delete_source_file) = 0;

  /**
   * Deletes a stored chunk.
   * @param name Chunk name
   * @return True if chunk deleted or non-existant
   */
  virtual bool Delete(const std::string &name) = 0;

  /**
   * Modifies chunk content under the given name.
   * @param name Chunk name, i.e. hash of the chunk content
   * @param content The chunk's modified content
   * @return True if chunk has been modified.
   */
  virtual bool Modify(const std::string &name, const std::string &content) = 0;

  /**
   * Modifies a chunk's content as a file, potentially overwriting an existing
   * file of the same name.
   * @param name Chunk name
   * @param source_file_name Path to modified content file
   * @return True if chunk has been modified.
   */
  virtual bool Modify(const std::string &name,
                      const fs::path &source_file_name,
                      bool delete_source_file) = 0;

  /**
   * Efficiently adds a locally existing chunk to another ChunkStore and
   * removes it from this one.
   * @param name Chunk name
   * @param sink_chunk_store The receiving ChunkStore
   * @return True if operation successful
   */
  virtual bool MoveTo(const std::string &name,
                      ChunkStore *sink_chunk_store) = 0;

  /**
   * Checks if a chunk exists.
   * @param name Chunk name
   * @return True if chunk exists
   */
  virtual bool Has(const std::string &name) const = 0;

  /**
   * Validates a chunk.
   *
   * In case a chunk turns out to be invalid, it's advisable to delete it.
   * @param name Chunk name
   * @return True if chunk valid
   */
  virtual bool Validate(const std::string &name) const = 0;

  /**
   * Retrieves the chunk's content version.
   * @param name Chunk name
   * @return The chunk version
   */
  virtual std::string Version(const std::string &name) const = 0;

  /**
   * Retrieves the size of a chunk.
   * @param name Chunk name
   * @return Size in bytes
   */
  virtual std::uintmax_t Size(const std::string &name) const = 0;

  /**
   * Retrieves the total size of the stored chunks.
   * @return Size in bytes
   */
  virtual std::uintmax_t Size() const {
    return size_;
  }

  /**
   * Retrieves the maximum storage capacity available to this ChunkStore.
   *
   * A capacity of zero (0) equals infinite storage space.
   * @return Capacity in bytes
   */
  virtual std::uintmax_t Capacity() const {
    return capacity_;
  }

  /**
   * Sets the maximum storage capacity available to this ChunkStore.
   *
   * A capacity of zero (0) equals infinite storage space. The capacity must
   * always be at least as high as the total size of already stored chunks.
   * @param capacity Capacity in bytes
   */
  virtual void SetCapacity(const std::uintmax_t &capacity) {
    capacity_ = capacity;
    if (capacity_ > 0 && capacity_ < size_)
      capacity_ = size_;
  }

  /**
   * Checks whether the ChunkStore has enough capacity to store a chunk of the
   * given size.
   * @return True if required size vacant
   */
  virtual bool Vacant(const std::uintmax_t &required_size) const {
    return capacity_ == 0 || size_ + required_size <= capacity_;
  }

  /**
   * Retrieves the number of references to a chunk.
   *
   * If reference counting is enabled, this returns the number of (virtual)
   * copies of a chunk in the ChunkStore. Otherwise it would return 1 if the
   * chunks exists, or 0 if it doesn't.
   * @param name Chunk name
   * @return Reference count
   */
  virtual std::uintmax_t Count(const std::string &name) const = 0;

  /**
   * Retrieves the number of chunks held by this ChunkStore.
   * @return Chunk count
   */
  virtual std::uintmax_t Count() const = 0;

  /**
   * Checks if any chunks are held by this ChunkStore.
   * @return True if no chunks stored
   */
  virtual bool Empty() const = 0;

  /**
   * Deletes all stored chunks.
   */
  virtual void Clear() { size_ = 0; }

 protected:
  // / Whether reference counting is enabled for this instance.
  const bool kReferenceCounting;

  /**
   * Increases the total size of the stored chunks.
   *
   * To be called by derived class when storing non-existant chunk.
   * @param delta Size to add to total
   */
  void IncreaseSize(const std::uintmax_t &delta) {
    size_ += delta;
    if (capacity_ > 0 && capacity_ < size_)
      capacity_ = size_;
  }

  /**
   * Decreases the total size of the stored chunks.
   *
   * To be called by derived class when deleting existant chunk.
   * @param delta Size to subtract from total
   */
  void DecreaseSize(const std::uintmax_t &delta) {
    if (delta <= size_)
      size_ -= delta;
    else
      size_ = 0;
  }

  /**
   * Assess Storage Capacity needed For a Modify Operation
   */
  bool AssessSpaceRequirement(const std::uintmax_t& current_size,
                              const std::uintmax_t& new_size,
                              bool* increase_size,
                              std::uintmax_t* adjusting_space) {
    if (current_size < new_size) {
      *increase_size = true;
      *adjusting_space = new_size - current_size;
      if (!Vacant(*adjusting_space))
        return false;
    } else {
      *increase_size = false;
      *adjusting_space = current_size - new_size;
    }
    return true;
  }

  /**
   * Updates Chunk Store Size After a Modify Operation
   */
  void AdjustChunkStoreStats(const std::uintmax_t& content_size_difference,
                             const bool& increase_size) {
    if (content_size_difference == 0)
      return;
    if (increase_size)
      IncreaseSize(content_size_difference);
    else
      DecreaseSize(content_size_difference);
  }

 private:
  ChunkStore(const ChunkStore&);
  ChunkStore& operator=(const ChunkStore&);
  std::uintmax_t capacity_, size_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CHUNK_STORE_H_
