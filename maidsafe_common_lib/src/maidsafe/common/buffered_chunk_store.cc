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
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

std::string BufferedChunkStore::Get(const std::string &name) const {
  UpgradeLock upgrade_lock(shared_mutex_);
  if (memory_chunk_store_->Has(name)) {
    auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                        name);
    if (it != cached_chunk_names_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      cached_chunk_names_.erase(it);
      cached_chunk_names_.push_front(name);
    }
    return memory_chunk_store_->Get(name);
  } else {
    upgrade_lock.unlock();
    return file_chunk_store_->Get(name);
  }
}

bool BufferedChunkStore::Get(const std::string &name,
                             const fs::path &sink_file_name) const {
  UpgradeLock upgrade_lock(shared_mutex_);
  if (memory_chunk_store_->Has(name)) {
    auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                        name);
    if (it != cached_chunk_names_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      cached_chunk_names_.erase(it);
      cached_chunk_names_.push_front(name);
    }
    return memory_chunk_store_->Get(name, sink_file_name);
  } else {
    upgrade_lock.unlock();
    return file_chunk_store_->Get(name, sink_file_name);
  }
}

bool BufferedChunkStore::Store(const std::string &name,
                               const std::string &content) {
  {
    //  Check whether chunk already exist or not
    bool exist_already(false);
    SharedLock shared_lock(shared_mutex_);
    if (memory_chunk_store_->Has(name)) {
      exist_already = true;
    } else {
      shared_lock.unlock();
      exist_already = file_chunk_store_->Has(name);
    }
    if (exist_already) {
      shared_lock.unlock();
      return file_chunk_store_->Store(name, content);
    }

    //  Check whether cache has capacity to store chunk
    shared_lock.lock();
    if ((content.size() > memory_chunk_store_->Capacity()) &&
          (memory_chunk_store_->Capacity() > 0))
        return false;
  }

  //  Check whether File Chunk Store has capacity to hold chunk
  if ((content.size() > file_chunk_store_->Capacity() &&
      file_chunk_store_->Capacity() > 0) || (content.empty()) || (name.empty()))
    return false;

  //  Try to make space in File Chunk Store to hold new chunk
  std::string chunk_to_delete;
  bool no_space(false);
  while (!file_chunk_store_->Vacant(content.size())) {
    {
      UniqueLock unique_lock(shared_mutex_);
      if (removable_chunk_names_.empty()) {
        no_space = true;
        break;
      }
      chunk_to_delete = *(removable_chunk_names_.begin());
      removable_chunk_names_.pop_front();
    }
    Delete(chunk_to_delete);
  }
  if (no_space)
    return false;

  //  Now First store new chunk in Cache
  {
    UpgradeLock upgrade_lock(shared_mutex_);
    auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                        name);
    if (it == cached_chunk_names_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      while (!memory_chunk_store_->Vacant(content.size()) &&
          !cached_chunk_names_.empty()) {
        memory_chunk_store_->Delete(cached_chunk_names_.back());
        cached_chunk_names_.pop_back();
      }
      memory_chunk_store_->Store(name, content);
    }
  }

  asio_service_.post(std::bind(
      static_cast<void(BufferedChunkStore::*)                 // NOLINT (Fraser)
                  (const std::string&)>(
          &BufferedChunkStore::CopyingChunkInFile), this, name));
  return true;
}
bool BufferedChunkStore::StoreCached(const std::string &name,
                                     const std::string &content) {
  UpgradeLock upgrade_lock(shared_mutex_);
  if ((content.size() > memory_chunk_store_->Capacity()) &&
      (memory_chunk_store_->Capacity() > 0))
    return false;
  auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                      name);
  if (it == cached_chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    while (!memory_chunk_store_->Vacant(content.size())
        && !cached_chunk_names_.empty()) {
      if (!memory_chunk_store_->Delete(cached_chunk_names_.back()))
        return false;
      cached_chunk_names_.pop_back();
    }
    if (memory_chunk_store_->Store(name, content))
      cached_chunk_names_.push_front(name);
    else
      return false;
  }
  return true;
}
bool BufferedChunkStore::StoreCached(const std::string &name,
                                     const fs::path &source_file_name,
                                     bool delete_source_file) {
  UpgradeLock upgrade_lock(shared_mutex_);
  boost::system::error_code ec;
  uintmax_t chunk_size(fs::file_size(source_file_name, ec));
  if ((chunk_size > memory_chunk_store_->Capacity()) &&
      (memory_chunk_store_->Capacity() > 0))
    return false;
  auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                      name);
  if (it == cached_chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    while (!memory_chunk_store_->Vacant(chunk_size)
        && !cached_chunk_names_.empty()) {
      if (!memory_chunk_store_->Delete(cached_chunk_names_.back()))
        return false;
      cached_chunk_names_.pop_back();
    }
    if (memory_chunk_store_->Store(name, source_file_name,
                                      delete_source_file)) {
      cached_chunk_names_.push_front(name);
      return true;
    } else {
      return false;
    }
  } else {
    if (delete_source_file)
      fs::remove(source_file_name, ec);
    return true;
  }
}

bool BufferedChunkStore::Store(const std::string &name,
                               const fs::path &source_file_name,
                               bool delete_source_file) {
  boost::system::error_code ec;
  uintmax_t chunk_size(fs::file_size(source_file_name, ec));
  {
    //  Check whether chunk already exist or not
    bool exist_already(false);
    SharedLock shared_lock(shared_mutex_);
    if (memory_chunk_store_->Has(name)) {
      exist_already = true;
    } else {
      shared_lock.unlock();
      exist_already = file_chunk_store_->Has(name);
    }
    if (exist_already) {
      shared_lock.unlock();
      return file_chunk_store_->Store(name, source_file_name,
                                      delete_source_file);
    }

    //  Check whether cache has capacity to store chunk
    if ((chunk_size > memory_chunk_store_->Capacity()) &&
          (memory_chunk_store_->Capacity() > 0))
        return false;
  }

  //  Check whether File Chunk Store has capacity to hold chunk
  if ((chunk_size > file_chunk_store_->Capacity() &&
      file_chunk_store_->Capacity() > 0) || (ec) || (name.empty()))
    return false;

  //  Try to make space in File Chunk Store to hold new chunk
  std::string chunk_to_delete;
  bool no_space(false);
  while (!file_chunk_store_->Vacant(chunk_size)) {
    {
      UniqueLock unique_lock(shared_mutex_);
      if (removable_chunk_names_.empty()) {
        no_space = true;
        break;
      }
      chunk_to_delete = *(removable_chunk_names_.begin());
      removable_chunk_names_.pop_front();
    }
    Delete(chunk_to_delete);
  }
  if (no_space)
    return false;

  //  Now First store new chunk in Cache
  {
    UpgradeLock upgrade_lock(shared_mutex_);
    auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                        name);
    if (it == cached_chunk_names_.end()) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      while (!memory_chunk_store_->Vacant(chunk_size) &&
          !cached_chunk_names_.empty()) {
        memory_chunk_store_->Delete(cached_chunk_names_.back());
        cached_chunk_names_.pop_back();
      }
      memory_chunk_store_->Store(name, source_file_name, delete_source_file);
    }
  }

  asio_service_.post(std::bind(
      static_cast<void(BufferedChunkStore::*)                 // NOLINT (Fraser)
                  (const std::string&)>(
          &BufferedChunkStore::CopyingChunkInFile), this, name));
  return true;
}

bool BufferedChunkStore::Delete(const std::string &name) {
  /*{
    UniqueLock unique_lock(shared_mutex_);
    while (std::find(cached_chunk_names_.begin(),
                     cached_chunk_names_.end(), name) ==
        cached_chunk_names_.end())
      cond_var_any_.wait(unique_lock);
  }*/
  bool file_delete_result = file_chunk_store_->Delete(name);
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                      name);
  if (it != cached_chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    cached_chunk_names_.erase(it);
    memory_chunk_store_->Delete(name);
  }
  return file_delete_result;
}

bool BufferedChunkStore::MoveTo(const std::string &name,
                                ChunkStore *sink_chunk_store) {
  bool chunk_moved(false);
  UniqueLock unique_lock(shared_mutex_);
  auto it = std::find(cached_chunk_names_.begin(), cached_chunk_names_.end(),
                      name);
  if (it != cached_chunk_names_.end()) {
    cached_chunk_names_.erase(it);
    memory_chunk_store_->Delete(name);
    unique_lock.unlock();
    chunk_moved = file_chunk_store_->MoveTo(name, sink_chunk_store);
  } else {
    unique_lock.unlock();
    if (file_chunk_store_->Has(name))
      chunk_moved = file_chunk_store_->MoveTo(name, sink_chunk_store);
  }
  return chunk_moved;
}

bool BufferedChunkStore::Has(const std::string &name) const {
  return file_chunk_store_->Has(name);
}

bool BufferedChunkStore::CacheHas(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Has(name);
}

bool BufferedChunkStore::HasCached(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Has(name);
}

bool BufferedChunkStore::Validate(const std::string &name) const {
  {
    SharedLock shared_lock(shared_mutex_);
    if (memory_chunk_store_->Has(name))
      return memory_chunk_store_->Validate(name);
  }
  return file_chunk_store_->Validate(name);
}

std::uintmax_t BufferedChunkStore::Size(const std::string &name) const {
  {
    SharedLock shared_lock(shared_mutex_);
    if (memory_chunk_store_->Has(name))
      return memory_chunk_store_->Size(name);
  }
  return file_chunk_store_->Size(name);
}

std::uintmax_t BufferedChunkStore::Size() const {
  return file_chunk_store_->Size();
}

std::uintmax_t BufferedChunkStore::CacheSize(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Size(name);
}

std::uintmax_t BufferedChunkStore::CacheSize() const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Size();
}

std::uintmax_t BufferedChunkStore::Capacity() const {
  return file_chunk_store_->Capacity();
}

std::uintmax_t BufferedChunkStore::CacheCapacity() const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Capacity();
}

void BufferedChunkStore::SetCapacity(const std::uintmax_t &capacity) {
  file_chunk_store_->SetCapacity(capacity);
}

void BufferedChunkStore::SetCacheCapacity(const std::uintmax_t &capacity) {
  UniqueLock unique_lock(shared_mutex_);
  memory_chunk_store_->SetCapacity(capacity);
}

bool BufferedChunkStore::Vacant(const std::uintmax_t &required_size) const {
  return file_chunk_store_->Vacant(required_size);
}

bool BufferedChunkStore::VacantCache(
    const std::uintmax_t &required_size) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Vacant(required_size);
}

std::uintmax_t BufferedChunkStore::Count(const std::string &name) const {
  return file_chunk_store_->Count(name);
}

std::uintmax_t BufferedChunkStore::CacheCount(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Count(name);
}

std::uintmax_t BufferedChunkStore::Count() const {
  return file_chunk_store_->Count();
}

std::uintmax_t BufferedChunkStore::CacheCount() const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Count();
}

bool BufferedChunkStore::Empty() const {
  return file_chunk_store_->Empty();
}

bool BufferedChunkStore::CacheEmpty() const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Empty();
}

void BufferedChunkStore::Clear() {
  file_chunk_store_->Clear();
}

void BufferedChunkStore::ClearCache() {
  UniqueLock unique_lock(shared_mutex_);
  cached_chunk_names_.clear();
  memory_chunk_store_->Clear();
}

void BufferedChunkStore::MarkForDeletion(const std::string &name) {
  UniqueLock unique_lock(shared_mutex_);
  removable_chunk_names_.push_back(name);
}

void BufferedChunkStore::CopyingChunkInFile(const std::string &name) {
  UpgradeLock upgrade_lock(shared_mutex_);
  std::string content = memory_chunk_store_->Get(name);
  upgrade_lock.unlock();
  file_chunk_store_->Store(name, content);
  upgrade_lock.lock();
  UpgradeToUniqueLock unique_lock(upgrade_lock);
  cached_chunk_names_.push_front(name);
}

}  // namespace maidsafe
