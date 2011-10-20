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

#include "maidsafe/common/buffered_chunk_store.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

BufferedChunkStore::~BufferedChunkStore() {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (pending_xfers_ > 0 && !asio_service_.stopped())
    xfer_cond_var_.wait(lock);
}

std::string BufferedChunkStore::Get(const std::string &name) const {
  if (name.empty())
    return "";

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name)) {
    auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
    if (it != cached_chunks_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      cached_chunks_.erase(it);
      cached_chunks_.push_front(name);
    }
    return cache_chunk_store_.Get(name);
  } else {
    upgrade_lock.unlock();
    std::string content(perm_chunk_store_.Get(name));
    if (DoCacheStore(name, content))
      AddCachedChunksEntry(name);
    return content;
  }
}

bool BufferedChunkStore::Get(const std::string &name,
                             const fs::path &sink_file_name) const {
  if (name.empty())
    return false;

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name)) {
    auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
    if (it != cached_chunks_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      cached_chunks_.erase(it);
      cached_chunks_.push_front(name);
    }
    return cache_chunk_store_.Get(name, sink_file_name);
  } else {
    upgrade_lock.unlock();
    std::string content(perm_chunk_store_.Get(name));
    if (DoCacheStore(name, content))
      AddCachedChunksEntry(name);
    return !content.empty() && WriteFile(sink_file_name, content);
  }
}

bool BufferedChunkStore::Store(const std::string &name,
                               const std::string &content) {
  if (!DoCacheStore(name, content))
    return false;

  if (!MakeChunkPermanent(name, content.size())) {
    // AddCachedChunksEntry(name);
    UniqueLock lock(cache_mutex_);
    cache_chunk_store_.Delete(name);
    return false;
  }

  return true;
}

bool BufferedChunkStore::Store(const std::string &name,
                               const fs::path &source_file_name,
                               bool delete_source_file) {
  boost::system::error_code ec;
  uintmax_t size(fs::file_size(source_file_name, ec));

  if (!DoCacheStore(name, size, source_file_name, false))
    return false;

  if (!MakeChunkPermanent(name, size)) {
    // AddCachedChunksEntry(name);
    UniqueLock lock(cache_mutex_);
    cache_chunk_store_.Delete(name);
    return false;
  }

  if (delete_source_file)
    fs::remove(source_file_name, ec);

  return true;
}

bool BufferedChunkStore::CacheStore(const std::string &name,
                                    const std::string &content) {
  if (!DoCacheStore(name, content))
    return false;

  AddCachedChunksEntry(name);
  return true;
}

bool BufferedChunkStore::CacheStore(const std::string &name,
                                    const fs::path &source_file_name,
                                    bool delete_source_file) {
  boost::system::error_code ec;
  uintmax_t size(fs::file_size(source_file_name, ec));

  if (!DoCacheStore(name, size, source_file_name, false))
    return false;

  AddCachedChunksEntry(name);
  if (delete_source_file)
    fs::remove(source_file_name, ec);

  return true;
}

bool BufferedChunkStore::Delete(const std::string &name) {
  if (name.empty())
    return false;

  bool file_delete_result(false);
  {
    boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
    while (pending_xfers_ > 0)
      xfer_cond_var_.wait(xfer_lock);
    file_delete_result = perm_chunk_store_.Delete(name);
    perm_size_ = perm_chunk_store_.Size();
  }

  UpgradeLock upgrade_lock(cache_mutex_);
  auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
  if (it != cached_chunks_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cached_chunks_.erase(it);
    cache_chunk_store_.Delete(name);
  }

  return file_delete_result;
}

bool BufferedChunkStore::MoveTo(const std::string &name,
                                ChunkStore *sink_chunk_store) {
  if (name.empty())
    return false;

  bool chunk_moved(false);
  {
    boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
    while (pending_xfers_ > 0)
      xfer_cond_var_.wait(xfer_lock);
    chunk_moved = perm_chunk_store_.MoveTo(name, sink_chunk_store);
    perm_size_ = perm_chunk_store_.Size();
  }

  if (!chunk_moved)
    return false;

  UpgradeLock upgrade_lock(cache_mutex_);
  auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
  if (it != cached_chunks_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cached_chunks_.erase(it);
    cache_chunk_store_.Delete(name);
  }

  return true;
}

bool BufferedChunkStore::Has(const std::string &name) const {
  return !name.empty() && (CacheHas(name) || perm_chunk_store_.Has(name));
}

bool BufferedChunkStore::CacheHas(const std::string &name) const {
  if (name.empty())
    return false;

  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Has(name);
}

bool BufferedChunkStore::Validate(const std::string &name) const {
  if (name.empty())
    return false;

  {
    SharedLock shared_lock(cache_mutex_);
    if (cache_chunk_store_.Has(name))
      return cache_chunk_store_.Validate(name);
  }
  return perm_chunk_store_.Validate(name);
}

std::uintmax_t BufferedChunkStore::Size(const std::string &name) const {
  if (name.empty())
    return 0;

  {
    SharedLock shared_lock(cache_mutex_);
    if (cache_chunk_store_.Has(name))
      return cache_chunk_store_.Size(name);
  }
  return perm_chunk_store_.Size(name);
}

std::uintmax_t BufferedChunkStore::Size() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_size_;
}

std::uintmax_t BufferedChunkStore::CacheSize() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Size();
}

std::uintmax_t BufferedChunkStore::Capacity() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_capacity_;
}

std::uintmax_t BufferedChunkStore::CacheCapacity() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Capacity();
}

void BufferedChunkStore::SetCapacity(const std::uintmax_t &capacity) {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (pending_xfers_ > 0)
    xfer_cond_var_.wait(lock);
  perm_chunk_store_.SetCapacity(capacity);
  perm_capacity_ = perm_chunk_store_.Capacity();
}

void BufferedChunkStore::SetCacheCapacity(const std::uintmax_t &capacity) {
  UniqueLock unique_lock(cache_mutex_);
  cache_chunk_store_.SetCapacity(capacity);
}

bool BufferedChunkStore::Vacant(const std::uintmax_t &required_size) const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_capacity_ == 0 || perm_size_ + required_size <= perm_capacity_;
}

bool BufferedChunkStore::CacheVacant(
    const std::uintmax_t &required_size) const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Vacant(required_size);
}

std::uintmax_t BufferedChunkStore::Count(const std::string &name) const {
  if (name.empty())
    return 0;

  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (pending_xfers_ > 0)
    xfer_cond_var_.wait(lock);
  return perm_chunk_store_.Count(name);
}

std::uintmax_t BufferedChunkStore::Count() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (pending_xfers_ > 0)
    xfer_cond_var_.wait(lock);
  return perm_chunk_store_.Count();
}

std::uintmax_t BufferedChunkStore::CacheCount() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Count();
}

bool BufferedChunkStore::Empty() const {
  return CacheEmpty() && perm_chunk_store_.Empty();
}

bool BufferedChunkStore::CacheEmpty() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Empty();
}

void BufferedChunkStore::Clear() {
  UniqueLock unique_lock(cache_mutex_);
  boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
  while (pending_xfers_ > 0)
    xfer_cond_var_.wait(xfer_lock);
  cached_chunks_.clear();
  removable_chunks_.clear();
  cache_chunk_store_.Clear();
  perm_chunk_store_.Clear();
  perm_capacity_ = perm_chunk_store_.Capacity();
  perm_size_ = 0;
}

void BufferedChunkStore::CacheClear() {
  UniqueLock unique_lock(cache_mutex_);
  cached_chunks_.clear();
  cache_chunk_store_.Clear();
}

void BufferedChunkStore::MarkForDeletion(const std::string &name) {
  if (!name.empty()) {
    boost::unique_lock<boost::mutex> lock(xfer_mutex_);
    removable_chunks_.push_back(name);
  }
}

/// @note Ensure cache mutex is not locked.
void BufferedChunkStore::AddCachedChunksEntry(const std::string &name) const {
  if (!name.empty()) {
    UniqueLock unique_lock(cache_mutex_);
    auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
    if (it != cached_chunks_.end())
      cached_chunks_.erase(it);
    cached_chunks_.push_front(name);
  }
}

bool BufferedChunkStore::DoCacheStore(const std::string &name,
                                      const std::string &content) const {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name))
    return false;

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name))
    return true;

  // Check whether cache has capacity to store chunk
  if (content.size() > cache_chunk_store_.Capacity() &&
      cache_chunk_store_.Capacity() > 0)
    return false;

  // Make space in cache
  UpgradeToUniqueLock unique_lock(upgrade_lock);
  while (!cache_chunk_store_.Vacant(content.size()) &&
         !cached_chunks_.empty()) {
    cache_chunk_store_.Delete(cached_chunks_.back());
    cached_chunks_.pop_back();
  }

  return cache_chunk_store_.Store(name, content);
}

bool BufferedChunkStore::DoCacheStore(const std::string &name,
                                      const uintmax_t &size,
                                      const fs::path &source_file_name,
                                      bool delete_source_file) const {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name))
    return false;

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name))
    return true;

  // Check whether cache has capacity to store chunk
  if (size > cache_chunk_store_.Capacity() &&
      cache_chunk_store_.Capacity() > 0)
    return false;

  // Make space in cache
  UpgradeToUniqueLock unique_lock(upgrade_lock);
  while (!cache_chunk_store_.Vacant(size) && !cached_chunks_.empty()) {
    cache_chunk_store_.Delete(cached_chunks_.back());
    cached_chunks_.pop_back();
  }

  return cache_chunk_store_.Store(name, source_file_name, delete_source_file);
}

bool BufferedChunkStore::MakeChunkPermanent(const std::string& name,
                                            const uintmax_t &size) {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);

  // Check whether permanent store has capacity to store chunk
  if (perm_capacity_ > 0) {
    if (size > perm_capacity_)
      return false;

    bool is_new(true);
    if (perm_size_ + size > perm_capacity_) {
      while (pending_xfers_ > 0)
        xfer_cond_var_.wait(lock);
      if (perm_chunk_store_.Has(name)) {
        is_new = false;
      } else {
        // Make space in permanent store
        while (perm_size_ + size > perm_capacity_) {
          if (removable_chunks_.empty())
            return false;
          if (perm_chunk_store_.Delete(removable_chunks_.front()))
            perm_size_ = perm_chunk_store_.Size();
          removable_chunks_.pop_front();
        }
      }
    }

    if (is_new)
      perm_size_ += size;  // account for chunk in transfer
  }

  asio_service_.post(std::bind(
      static_cast<void(BufferedChunkStore::*)(const std::string&)>(    // NOLINT
          &BufferedChunkStore::DoMakeChunkPermanent), this, name));
  ++pending_xfers_;

  return true;
}

void BufferedChunkStore::DoMakeChunkPermanent(const std::string &name) {
  std::string content;
  {
    SharedLock shared_lock(cache_mutex_);
    content = cache_chunk_store_.Get(name);
  }

  if (!content.empty()) {
    perm_chunk_store_.Store(name, content);
    AddCachedChunksEntry(name);
  }

  {
    boost::unique_lock<boost::mutex> lock(xfer_mutex_);
    perm_size_ = perm_chunk_store_.Size();
    --pending_xfers_;
    xfer_cond_var_.notify_all();
  }
}

}  // namespace maidsafe
