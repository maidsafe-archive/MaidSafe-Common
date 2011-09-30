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
  std::string contents;
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it != chunk_names_.end()) {
    contents = memory_chunk_store_->Get(name);
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_names_.erase(it);
    chunk_names_.push_front(name);
  } else {
    upgrade_lock.unlock();
    contents = file_chunk_store_->Get(name);
    if (contents != "") {
      upgrade_lock.lock();
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      chunk_names_.push_front(name);
      memory_chunk_store_->Store(name, contents);
    }
  }
  return contents;
}

bool BufferedChunkStore::Get(const std::string &name,
                             const fs::path &sink_file_name) const {
  bool get_result(false);
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it != chunk_names_.end()) {
    get_result = memory_chunk_store_->Get(name, sink_file_name);
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_names_.erase(it);
    chunk_names_.push_front(name);
  } else {
    upgrade_lock.unlock();
    get_result = file_chunk_store_->Get(name, sink_file_name);
    if (get_result) {
      upgrade_lock.lock();
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      chunk_names_.push_front(name);
      memory_chunk_store_->Store(name, sink_file_name, false);
    }
  }
  return get_result;
}

bool BufferedChunkStore::Store(const std::string &name,
                               const std::string &content) {
  if ((content.size() > file_chunk_store_->Capacity() &&
      file_chunk_store_->Capacity() > 0) || (content.empty()) || (name.empty()))
    return false;
  {
    UpgradeLock upgrade_lock(shared_mutex_);
    auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
    if (it == chunk_names_.end()) {
      if ((content.size() > memory_chunk_store_->Capacity()) &&
          (memory_chunk_store_->Capacity() > 0)) {
        DLOG(ERROR) << "Store -"
                    << "not able to store chunk in memory for chunk -"
                    << name;
      } else {
        UpgradeToUniqueLock unique_lock(upgrade_lock);
        while (!memory_chunk_store_->Vacant(content.size()) &&
            !chunk_names_.empty()) {
          memory_chunk_store_->Delete(chunk_names_.back());
          chunk_names_.pop_back();
        }
        if (memory_chunk_store_->Store(name, content)) {
          chunk_names_.push_front(name);
          //  Add info for chunk still being copied to file_chunk_store
          transient_chunk_names_.push_back(name);
        }
      }
    }
  }
  asio_service_.post(std::bind(
      static_cast<void(BufferedChunkStore::*)                 // NOLINT (Fraser)
                  (const std::string&, const std::string&)>(
          &BufferedChunkStore::StoreInFile), this, name, content));
  return true;
}
bool BufferedChunkStore::StoreCached(const std::string &name,
                                     const std::string &content) {
  UpgradeLock upgrade_lock(shared_mutex_);
  if ((content.size() > memory_chunk_store_->Capacity()) &&
      (memory_chunk_store_->Capacity() > 0))
    return false;
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it == chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    while (!memory_chunk_store_->Vacant(content.size())
        && !chunk_names_.empty()) {
      if (!memory_chunk_store_->Delete(chunk_names_.back()))
        return false;
      chunk_names_.pop_back();
    }
    chunk_names_.push_front(name);
    return memory_chunk_store_->Store(name, content);
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
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it == chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    while (!memory_chunk_store_->Vacant(chunk_size)
        && !chunk_names_.empty()) {
      if (!memory_chunk_store_->Delete(chunk_names_.back()))
        return false;
      chunk_names_.pop_back();
    }
    chunk_names_.push_front(name);
    return memory_chunk_store_->Store(name, source_file_name,
                                      delete_source_file);
  }
  return true;
}

bool BufferedChunkStore::Store(const std::string &name,
                               const fs::path &source_file_name,
                               bool delete_source_file) {
  boost::system::error_code ec;
  uintmax_t chunk_size(fs::file_size(source_file_name, ec));
  if ((chunk_size > file_chunk_store_->Capacity() &&
      file_chunk_store_->Capacity() > 0) || (ec) || (name.empty()))
    return false;
  {
    UpgradeLock upgrade_lock(shared_mutex_);
    auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
    if (it == chunk_names_.end()) {
      if ((chunk_size > memory_chunk_store_->Capacity()) &&
          (memory_chunk_store_->Capacity() > 0)) {
        DLOG(ERROR) << "Store -"
                    << "not able to store chunk in memory for chunk -"
                    << name;
      } else {
        UpgradeToUniqueLock unique_lock(upgrade_lock);
        while (!memory_chunk_store_->Vacant(chunk_size) &&
            !chunk_names_.empty()) {
          memory_chunk_store_->Delete(chunk_names_.back());
          chunk_names_.pop_back();
        }
        if (memory_chunk_store_->Store(name, source_file_name, false)) {
          chunk_names_.push_front(name);
          //  Add info for chunk still being copied to file_chunk_store
          transient_chunk_names_.push_back(name);
        }
      }
    }
  }
  asio_service_.post(std::bind(
      static_cast<void(BufferedChunkStore::*)                 // NOLINT (Fraser)
                  (const std::string&, const fs::path &, bool)>(
          &BufferedChunkStore::StoreInFile), this, name, source_file_name,
          delete_source_file));
  return true;
}

bool BufferedChunkStore::Delete(const std::string &name) {
  {
    UniqueLock unique_lock(shared_mutex_);
    while (std::find(transient_chunk_names_.begin(),
                     transient_chunk_names_.end(), name) !=
        transient_chunk_names_.end())
      cond_var_any_.wait(unique_lock);
  }
  bool file_delete_result = file_chunk_store_->Delete(name);
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it != chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_names_.erase(it);
    memory_chunk_store_->Delete(name);
  }
  return file_delete_result;
}

bool BufferedChunkStore::MoveTo(const std::string &name,
                                ChunkStore *sink_chunk_store) {
  if (file_chunk_store_->Has(name))
      return file_chunk_store_->MoveTo(name, sink_chunk_store);
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it != chunk_names_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_names_.erase(it);
    return memory_chunk_store_->MoveTo(name, sink_chunk_store);
  }
  return false;
}

bool BufferedChunkStore::Has(const std::string &name) const {
  return file_chunk_store_->Has(name);
}

bool BufferedChunkStore::HasCached(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  auto it = std::find(chunk_names_.begin(), chunk_names_.end(), name);
  if (it != chunk_names_.end()) {
    return true;
  }
  return false;
}

bool BufferedChunkStore::Validate(const std::string &name) const {
  return file_chunk_store_->Validate(name);
}

std::uintmax_t BufferedChunkStore::Size(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  if (memory_chunk_store_->Has(name)) {
    return memory_chunk_store_->Size(name);
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Size(name);
  }
}

std::uintmax_t BufferedChunkStore::CacheSize(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Size(name);
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
  SharedLock shared_lock(shared_mutex_);
  if (memory_chunk_store_->Has(name)) {
    return memory_chunk_store_->Count(name);
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Count();
  }
}

std::uintmax_t BufferedChunkStore::CacheCount(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  return memory_chunk_store_->Count(name);
}

std::uintmax_t BufferedChunkStore::Count() const {
  SharedLock shared_lock(shared_mutex_);
  return file_chunk_store_->Count() + transient_chunk_names_.size();
}

std::uintmax_t BufferedChunkStore::CacheCount() const {
  SharedLock shared_lock(shared_mutex_);
  return chunk_names_.size();
}

bool BufferedChunkStore::Empty() const {
  return file_chunk_store_->Empty();
}

bool BufferedChunkStore::CacheEmpty() const {
  SharedLock shared_lock(shared_mutex_);
  return chunk_names_.empty();
}

void BufferedChunkStore::Clear() {
  file_chunk_store_->Clear();
}

void BufferedChunkStore::ClearCache() {
  UniqueLock unique_lock(shared_mutex_);
  chunk_names_.clear();
  memory_chunk_store_->Clear();
}

void BufferedChunkStore::StoreInFile(const std::string &name,
                                     const std::string &contents) {
  file_chunk_store_->Store(name, contents);
  UniqueLock unique_lock(shared_mutex_);
  auto it = std::find(transient_chunk_names_.begin(),
                      transient_chunk_names_.end(), name);
  if (it != transient_chunk_names_.end())
    transient_chunk_names_.erase(it);
  cond_var_any_.notify_one();
}
void BufferedChunkStore::StoreInFile(const std::string &name,
                                     const fs::path &source_file_name,
                                     bool delete_source_file) {
  file_chunk_store_->Store(name, source_file_name, delete_source_file);
  UniqueLock unique_lock(shared_mutex_);
  auto it = std::find(transient_chunk_names_.begin(),
                     transient_chunk_names_.end(), name);
  if (it != transient_chunk_names_.end())
    transient_chunk_names_.erase(it);
  cond_var_any_.notify_one();
}
}  // namespace maidsafe
