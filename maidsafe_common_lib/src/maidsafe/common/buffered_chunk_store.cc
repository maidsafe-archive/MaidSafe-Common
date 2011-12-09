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

/**
 * If the cache is full and there are no more chunks left to delete, this is the
 * number of chunk transfers to wait for (in Store) before the next check.
 */
const int kWaitTransfersForCacheVacantCheck(10);

BufferedChunkStore::~BufferedChunkStore() {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (!pending_xfers_.empty() && !asio_service_.stopped())
    xfer_cond_var_.wait(lock);
}

std::string BufferedChunkStore::Get(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "Get - Empty name passed.";
    return "";
  }

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
  if (name.empty()) {
    DLOG(ERROR) << "Get - Empty name passed.";
    return false;
  }

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

bool BufferedChunkStore::PermanentStore(const std::string &name) {
  if (name.empty()) {
    DLOG(ERROR) << "PermanentStore - Empty name passed.";
    return false;
  }

  std::string content;
  {
    SharedLock shared_lock(cache_mutex_);
    content = cache_chunk_store_.Get(name);
  }

  {
    boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
    RemoveDeletionMarks(name);
    while (pending_xfers_.count(name) > 0)
      xfer_cond_var_.wait(xfer_lock);
    if (perm_chunk_store_.Has(name))
      return true;
    if (content.empty() || !perm_chunk_store_.Store(name, content)) {
      DLOG(ERROR) << "PermanentStore - Could not transfer "
                  << Base32Substr(name);
      return false;
    }
    perm_size_ = perm_chunk_store_.Size();
  }

  return true;
}

bool BufferedChunkStore::Delete(const std::string &name) {
  if (name.empty()) {
    DLOG(ERROR) << "Delete - Empty name passed.";
    return false;
  }

  bool file_delete_result(false);
  {
    boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
    while (pending_xfers_.count(name) > 0)
      xfer_cond_var_.wait(xfer_lock);
    file_delete_result = perm_chunk_store_.Delete(name);
    perm_size_ = perm_chunk_store_.Size();
  }

  if (!file_delete_result)
    DLOG(ERROR) << "Delete - Could not delete " << Base32Substr(name);

  UpgradeLock upgrade_lock(cache_mutex_);
  auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
  if (it != cached_chunks_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cached_chunks_.erase(it);
    cache_chunk_store_.Delete(name);
  }

  return file_delete_result;
}

bool BufferedChunkStore::Modify(const std::string &name,
                                const std::string &content) {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name)) {
    DLOG(ERROR) << "Modify - Invalid name passed: " << Base32Substr(name);
    return false;
  }

  if (chunk_validation_->Hashable(name)) {
    DLOG(ERROR) << "Modify - Hashable chunk passed: " << Base32Substr(name);
    return false;
  }

  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  RemoveDeletionMarks(name);

  while (pending_xfers_.count(name) > 0)
    xfer_cond_var_.wait(lock);

  if (perm_chunk_store_.Has(name)) {
    std::string current_perm_content(perm_chunk_store_.Get(name));
    uintmax_t content_size_difference(0);
    bool increase_size(false);
    if (content.size() > current_perm_content.size()) {
      content_size_difference = content.size() - current_perm_content.size();
      increase_size = true;
      if (perm_capacity_ > 0) {  // Check if Perm Chunk Store Size is Infinite
        // Wait For Space in Perm Store
        while (perm_size_ + content_size_difference > perm_capacity_) {
          if (removable_chunks_.empty()) {
            DLOG(ERROR) << "Modify - Can't make space for changes to "
                        << Base32Substr(name);
            return false;
          }
          if (perm_chunk_store_.Delete(removable_chunks_.front()))
            perm_size_ = perm_chunk_store_.Size();
          removable_chunks_.pop_front();
        }
      }
    } else {
      content_size_difference = current_perm_content.size() - content.size();
      increase_size = false;
    }
    if (perm_chunk_store_.Modify(name, content)) {
      if (increase_size)
        perm_size_ += content_size_difference;
      else
        perm_size_ -= content_size_difference;
      {
        UpgradeLock upgrade_lock(cache_mutex_);
        auto it = std::find(cached_chunks_.begin(), cached_chunks_.end(), name);
        if (it != cached_chunks_.end()) {
          UpgradeToUniqueLock unique_lock(upgrade_lock);
          cached_chunks_.erase(it);
          cache_chunk_store_.Delete(name);
        }
      }
      return true;
    } else {
      DLOG(ERROR) << "Modify - Couldn't modify " << Base32Substr(name);
      return false;
    }
  } else {
    std::string current_cache_content;
    {
      UpgradeLock upgrade_lock(cache_mutex_);
      if (!cache_chunk_store_.Has(name)) {
        DLOG(ERROR) << "Modify - Don't have chunk " << Base32Substr(name);
        return false;
      }

      current_cache_content = cache_chunk_store_.Get(name);
      uintmax_t content_size_difference(0);
      if (content.size() > current_cache_content.size()) {
        content_size_difference = content.size() - current_cache_content.size();
        // Make space in Cache if Needed
        while (!cache_chunk_store_.Vacant(content_size_difference)) {
          while (cached_chunks_.empty()) {
            upgrade_lock.unlock();
            {
              boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
              if (pending_xfers_.empty()) {
                DLOG(ERROR) << "Modify - Can't make space for changes to "
                            << Base32Substr(name);
                return false;
              }

              int limit(kWaitTransfersForCacheVacantCheck);
              while (!pending_xfers_.empty() && limit > 0) {
                xfer_cond_var_.wait(xfer_lock);
                --limit;
              }
            }
            upgrade_lock.lock();
          }
          UpgradeToUniqueLock unique_lock(upgrade_lock);
          cache_chunk_store_.Delete(cached_chunks_.back());
          cached_chunks_.pop_back();
        }
      }
      return cache_chunk_store_.Modify(name, content);
    }
  }
}

bool BufferedChunkStore::Modify(const std::string &name,
                                const fs::path &source_file_name,
                                bool delete_source_file) {
  if (source_file_name.empty()) {
    DLOG(ERROR) << "Modify - No source file passed for " << Base32Substr(name);
    return false;
  }

  // TODO(Steve) implement optimized Modify for changes from file

  std::string content;
  if (!ReadFile(source_file_name, &content)) {
    DLOG(ERROR) << "Modify - Couldn't read source file for "
                << Base32Substr(name);
    return false;
  }

  if (!Modify(name, content))
    return false;
  boost::system::error_code ec;
  if (delete_source_file)
    fs::remove(source_file_name, ec);
  return true;
}

bool BufferedChunkStore::MoveTo(const std::string &name,
                                ChunkStore *sink_chunk_store) {
  if (name.empty()) {
    DLOG(ERROR) << "MoveTo - Empty name passed.";
    return false;
  }

  bool chunk_moved(false);
  {
    boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
    while (pending_xfers_.count(name) > 0)
      xfer_cond_var_.wait(xfer_lock);
    chunk_moved = perm_chunk_store_.MoveTo(name, sink_chunk_store);
    perm_size_ = perm_chunk_store_.Size();
  }

  if (!chunk_moved) {
    DLOG(ERROR) << "MoveTo - Could not move " << Base32Substr(name);
    return false;
  }

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
  return CacheHas(name) || PermanentHas(name);
}

bool BufferedChunkStore::CacheHas(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "CacheHas - Empty name passed.";
    return false;
  }

  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Has(name);
}

bool BufferedChunkStore::PermanentHas(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "PermanentHas - Empty name passed.";
    return false;
  }

  boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
  while (pending_xfers_.count(name) > 0)
    xfer_cond_var_.wait(xfer_lock);
  uintmax_t rem_count(0);
  for (auto it = removable_chunks_.begin(); it != removable_chunks_.end(); ++it)
    if (*it == name)
      ++rem_count;
  return perm_chunk_store_.Count(name) > rem_count;
}

bool BufferedChunkStore::Validate(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "Validate - Empty name passed.";
    return false;
  }

  {
    SharedLock shared_lock(cache_mutex_);
    if (cache_chunk_store_.Has(name))
      return cache_chunk_store_.Validate(name);
  }
  return perm_chunk_store_.Validate(name);
}

std::string BufferedChunkStore::Version(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "Version - Empty name passed.";
    return "";
  }

  {
    SharedLock shared_lock(cache_mutex_);
    if (cache_chunk_store_.Has(name))
      return cache_chunk_store_.Version(name);
  }
  return perm_chunk_store_.Version(name);
}

uintmax_t BufferedChunkStore::Size(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "Size - Empty name passed.";
    return 0;
  }

  {
    SharedLock shared_lock(cache_mutex_);
    if (cache_chunk_store_.Has(name))
      return cache_chunk_store_.Size(name);
  }
  return perm_chunk_store_.Size(name);
}

uintmax_t BufferedChunkStore::Size() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_size_;
}

uintmax_t BufferedChunkStore::CacheSize() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Size();
}

uintmax_t BufferedChunkStore::Capacity() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_capacity_;
}

uintmax_t BufferedChunkStore::CacheCapacity() const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Capacity();
}

void BufferedChunkStore::SetCapacity(const uintmax_t &capacity) {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (!pending_xfers_.empty())
    xfer_cond_var_.wait(lock);
  perm_chunk_store_.SetCapacity(capacity);
  perm_capacity_ = perm_chunk_store_.Capacity();
}

void BufferedChunkStore::SetCacheCapacity(const uintmax_t &capacity) {
  UniqueLock unique_lock(cache_mutex_);
  cache_chunk_store_.SetCapacity(capacity);
}

bool BufferedChunkStore::Vacant(const uintmax_t &required_size) const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  return perm_capacity_ == 0 || perm_size_ + required_size <= perm_capacity_;
}

bool BufferedChunkStore::CacheVacant(
    const uintmax_t &required_size) const {
  SharedLock shared_lock(cache_mutex_);
  return cache_chunk_store_.Vacant(required_size);
}

uintmax_t BufferedChunkStore::Count(const std::string &name) const {
  if (name.empty()) {
    DLOG(ERROR) << "Count - Empty name passed.";
    return 0;
  }

  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (pending_xfers_.count(name) > 0)
    xfer_cond_var_.wait(lock);
  return perm_chunk_store_.Count(name);
}

uintmax_t BufferedChunkStore::Count() const {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);
  while (!pending_xfers_.empty())
    xfer_cond_var_.wait(lock);
  return perm_chunk_store_.Count();
}

uintmax_t BufferedChunkStore::CacheCount() const {
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
  boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
  while (!pending_xfers_.empty())
    xfer_cond_var_.wait(xfer_lock);
  UniqueLock unique_lock(cache_mutex_);
  cached_chunks_.clear();
  removable_chunks_.clear();
  cache_chunk_store_.Clear();
  perm_chunk_store_.Clear();
  perm_capacity_ = perm_chunk_store_.Capacity();
  perm_size_ = 0;
}

void BufferedChunkStore::CacheClear() {
  boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
  while (!pending_xfers_.empty())
    xfer_cond_var_.wait(xfer_lock);
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
  if (!chunk_validation_ || !chunk_validation_->ValidName(name)) {
    DLOG(ERROR) << "DoCacheStore - Invalid name passed: " << Base32Substr(name);
    return false;
  }

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name))
    return chunk_validation_->Hashable(name);

  // Check whether cache has capacity to store chunk
  if (content.size() > cache_chunk_store_.Capacity() &&
      cache_chunk_store_.Capacity() > 0) {
    DLOG(ERROR) << "DoCacheStore - Chunk " << Base32Substr(name) << " too big ("
                << BytesToBinarySiUnits(content.size()) << " vs. "
                << BytesToBinarySiUnits(cache_chunk_store_.Capacity()) << ").";
    return false;
  }

  // Make space in cache
  while (!cache_chunk_store_.Vacant(content.size())) {
    while (cached_chunks_.empty()) {
      upgrade_lock.unlock();
      {
        boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
        if (pending_xfers_.empty()) {
          DLOG(ERROR) << "DoCacheStore - Can't make space for "
                      << Base32Substr(name);
          return false;
        }
        int limit(kWaitTransfersForCacheVacantCheck);
        while (!pending_xfers_.empty() && limit > 0) {
          xfer_cond_var_.wait(xfer_lock);
          --limit;
        }
      }
      upgrade_lock.lock();
    }
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cache_chunk_store_.Delete(cached_chunks_.back());
    cached_chunks_.pop_back();
  }

  return cache_chunk_store_.Store(name, content);
}

bool BufferedChunkStore::DoCacheStore(const std::string &name,
                                      const uintmax_t &size,
                                      const fs::path &source_file_name,
                                      bool delete_source_file) const {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name)) {
    DLOG(ERROR) << "DoCacheStore - Invalid name passed: " << Base32Substr(name);
    return false;
  }

  UpgradeLock upgrade_lock(cache_mutex_);
  if (cache_chunk_store_.Has(name))
    return chunk_validation_->Hashable(name);

  // Check whether cache has capacity to store chunk
  if (size > cache_chunk_store_.Capacity() &&
      cache_chunk_store_.Capacity() > 0) {
    DLOG(ERROR) << "DoCacheStore - Chunk " << Base32Substr(name) << " too big ("
                << BytesToBinarySiUnits(size) << " vs. "
                << BytesToBinarySiUnits(cache_chunk_store_.Capacity()) << ").";
    return false;
  }

  // Make space in cache
  while (!cache_chunk_store_.Vacant(size)) {
    while (cached_chunks_.empty()) {
      upgrade_lock.unlock();
      {
        boost::unique_lock<boost::mutex> xfer_lock(xfer_mutex_);
        if (pending_xfers_.empty()) {
          DLOG(ERROR) << "DoCacheStore - Can't make space for "
                      << Base32Substr(name);
          return false;
        }
        int limit(kWaitTransfersForCacheVacantCheck);
        while (!pending_xfers_.empty() && limit > 0) {
          xfer_cond_var_.wait(xfer_lock);
          --limit;
        }
      }
      upgrade_lock.lock();
    }
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cache_chunk_store_.Delete(cached_chunks_.back());
    cached_chunks_.pop_back();
  }

  return cache_chunk_store_.Store(name, source_file_name, delete_source_file);
}

bool BufferedChunkStore::MakeChunkPermanent(const std::string& name,
                                            const uintmax_t &size) {
  boost::unique_lock<boost::mutex> lock(xfer_mutex_);

  if (!initialised_) {
    DLOG(ERROR) << "MakeChunkPermanent - Can't make " << Base32Substr(name)
                << " permanent, not initialised.";
    return false;
  }

  RemoveDeletionMarks(name);

  // Check whether permanent store has capacity to store chunk
  if (perm_capacity_ > 0) {
    if (size > perm_capacity_) {
      DLOG(ERROR) << "MakeChunkPermanent - Chunk " << Base32Substr(name)
                  << " too big (" << BytesToBinarySiUnits(size) << " vs. "
                  << BytesToBinarySiUnits(perm_capacity_) << ").";
      return false;
    }

    bool is_new(true);
    if (perm_size_ + size > perm_capacity_) {
      while (!pending_xfers_.empty())
        xfer_cond_var_.wait(lock);
      if (perm_chunk_store_.Has(name)) {
        is_new = false;
      } else {
        // Make space in permanent store
        while (perm_size_ + size > perm_capacity_) {
          if (removable_chunks_.empty()) {
            DLOG(ERROR) << "MakeChunkPermanent - Can't make space for "
                        << Base32Substr(name);
            return false;
          }
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
  pending_xfers_.insert(name);

  return true;
}

void BufferedChunkStore::DoMakeChunkPermanent(const std::string &name) {
  std::string content;
  {
    SharedLock shared_lock(cache_mutex_);
    content = cache_chunk_store_.Get(name);
  }

  if (content.empty()) {
    DLOG(ERROR) << "DoMakeChunkPermanent - Could not get " << Base32Substr(name)
                << " from cache.";
  } else if (perm_chunk_store_.Store(name, content)) {
    AddCachedChunksEntry(name);
  } else {
    DLOG(ERROR) << "DoMakeChunkPermanent - Could not store "
                << Base32Substr(name);
  }

  {
    boost::unique_lock<boost::mutex> lock(xfer_mutex_);
    perm_size_ = perm_chunk_store_.Size();
    pending_xfers_.erase(pending_xfers_.find(name));
    xfer_cond_var_.notify_all();
  }
}

void BufferedChunkStore::RemoveDeletionMarks(const std::string &name) {
  auto it = removable_chunks_.begin();
  while ((it = std::find(it, removable_chunks_.end(), name)) !=
         removable_chunks_.end()) {
      it = removable_chunks_.erase(it);
  }
}

bool BufferedChunkStore::DeleteAllMarked() {
  DLOG(INFO) << "DeleteAllMarked - Deleting all chunks listed "
             << "in removable_chunks_";
  while (!removable_chunks_.empty()) {
    auto it = removable_chunks_.begin();
    if (!Delete(*it)) {
      DLOG(ERROR) << "DeleteAllMarked - Could not delete "
                  << Base32Substr(*it);
      return false;
    }
    removable_chunks_.pop_front();
  }
  return true;
}

std::list<std::string> BufferedChunkStore::GetRemovableChunks() const {
  return removable_chunks_;
}

}  // namespace maidsafe
