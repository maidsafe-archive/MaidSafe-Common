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

#if MAIDSAFE_COMMON_VERSION != 1005
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

// Concrete threadsafe class to manage storage and retrieval of chunks.  The
// class implements shared mutex locking around another concrete ChunkStore.
class ThreadsafeChunkStore : public ChunkStore {
 public:
  explicit ThreadsafeChunkStore(std::shared_ptr<ChunkStore> chunk_store);
  ~ThreadsafeChunkStore();
  std::string Get(
      const std::string &name,
      const asymm::Identity &public_key_id = asymm::Identity()) const;
  bool Get(const std::string &name,
           const fs::path &sink_file_name,
           const asymm::Identity &public_key_id = asymm::Identity()) const;
  bool Store(const std::string &name,
             const std::string &content,
             const asymm::Identity &public_key_id = asymm::Identity());
  bool Store(const std::string &name,
             const fs::path &source_file_name,
             bool delete_source_file,
             const asymm::Identity &public_key_id = asymm::Identity());
  bool Delete(const std::string &name,
              const asymm::Identity &public_key_id = asymm::Identity());
  bool Modify(const std::string &name,
              const std::string &content,
              const asymm::Identity &public_key_id = asymm::Identity());
  bool Modify(const std::string &name,
              const fs::path &source_file_name,
              bool delete_source_file,
              const asymm::Identity &public_key_id = asymm::Identity());
  bool Has(const std::string &name,
           const asymm::Identity &public_key_id = asymm::Identity()) const;
  bool MoveTo(const std::string &name, ChunkStore *sink_chunk_store);
  uintmax_t Size(const std::string &name) const;
  uintmax_t Size() const;
  uintmax_t Capacity() const;
  void SetCapacity(const uintmax_t &capacity);
  bool Vacant(const uintmax_t &required_size) const;
  uintmax_t Count(const std::string &name) const;
  uintmax_t Count() const;
  bool Empty() const;
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
