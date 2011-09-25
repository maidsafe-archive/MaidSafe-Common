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
#include "maidsafe/common/utils.h"
const boost::posix_time::hours kChunkExpiryTime(10);
const uint16_t kChunkContainerCapacity(200);
namespace maidsafe {

std::string BufferedChunkStore::Get(const std::string &name) const {
  std::string contents;
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    contents = memory_chunk_store_->Get(name);
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_details_.pop_back();
    boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
  } else {
    upgrade_lock.unlock();
    contents = file_chunk_store_->Get(name);
    if (contents != "") {
      upgrade_lock.lock();
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
      ChunkDetails c_details(name, now + kChunkExpiryTime);
      chunk_details_.push_front(c_details);
      memory_chunk_store_->Store(name, contents);
    }
  }
  return contents;
}

bool BufferedChunkStore::Get(const std::string &name,
                             const fs::path &sink_file_name) const {
  bool get_result(false);
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    get_result = memory_chunk_store_->Get(name, sink_file_name);
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    chunk_details_.pop_back();
    boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
  } else {
    upgrade_lock.unlock();
    get_result = file_chunk_store_->Get(name, sink_file_name);
    if (get_result) {
      upgrade_lock.lock();
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
      ChunkDetails c_details(name, now + kChunkExpiryTime);
      chunk_details_.push_front(c_details);
      memory_chunk_store_->Store(name, sink_file_name, false);
    }
  }
  return get_result;
}

bool BufferedChunkStore::Store(const std::string &name,
                               const std::string &content) {
  asio_service_.post(std::bind(&BufferedChunkStore::StoreInFile, this, name,
                               content));
  return true;
}
bool BufferedChunkStore::StoreCached(const std::string &name,
                                     const std::string &content) {
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it == chunk_details_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    boost::posix_time::ptime now(
        boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
    return memory_chunk_store_->Store(name, content);
  }
  return true;
}
bool BufferedChunkStore::StoreCached(const std::string &name,
                                     const fs::path &source_file_name,
                                     bool delete_source_file) {
  UpgradeLock upgrade_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it == chunk_details_.end()) {
    UpgradeToUniqueLock unique_lock(upgrade_lock);
    boost::posix_time::ptime now(
        boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
    return memory_chunk_store_->Store(name, source_file_name,
                                      delete_source_file);
  }
  return true;
}
bool BufferedChunkStore::Store(const std::string &name,
                               const fs::path &source_file_name,
                               bool delete_source_file) {
  asio_service_.post(std::bind(&BufferedChunkStore::StoreInFile, this, name,
                               source_file_name, delete_source_file));
  return true;
}

bool BufferedChunkStore::Delete(const std::string &name) {
  UniqueLock unique_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    chunk_details_.erase(it);
    return memory_chunk_store_->Delete(name);
  } else {
    unique_lock.unlock();
    std::string contents = file_chunk_store_->Get(name);
    bool delete_result = file_chunk_store_->Delete(name);
    if (contents != "") {
      unique_lock.lock();
      boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
      ChunkDetails c_details(name, now + kChunkExpiryTime);
      chunk_details_.push_front(c_details);
      memory_chunk_store_->Store(name, contents);
    }
    return delete_result;
  }
}

bool BufferedChunkStore::MoveTo(const std::string &name,
                                ChunkStore *sink_chunk_store) {
  UniqueLock unique_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    chunk_details_.erase(it);
    return memory_chunk_store_->MoveTo(name, sink_chunk_store);
  } else {
    unique_lock.unlock();
    return file_chunk_store_->MoveTo(name, sink_chunk_store);
  }
}

bool BufferedChunkStore::Has(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    return true;
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Has(name);
  }
}

bool BufferedChunkStore::Validate(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    return memory_chunk_store_->Validate(name);
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Validate(name);
  }
}

std::uintmax_t BufferedChunkStore::Size(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    return memory_chunk_store_->Size(name);
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Size(name);
  }
}

std::uintmax_t BufferedChunkStore::Count(const std::string &name) const {
  SharedLock shared_lock(shared_mutex_);
  auto it = std::find_if(chunk_details_.begin(), chunk_details_.end(),
                         std::bind(&BufferedChunkStore::CompareChunkName, this,
                                   arg::_1));
  if (it != chunk_details_.end()) {
    return memory_chunk_store_->Count(name);
  } else {
    shared_lock.unlock();
    return file_chunk_store_->Size(name);
  }
}

std::uintmax_t BufferedChunkStore::Count() const {
  SharedLock shared_lock(shared_mutex_);
  return chunk_details_.size();
}

bool BufferedChunkStore::Empty() const {
  SharedLock shared_lock(shared_mutex_);
  return chunk_details_.empty();
}

bool BufferedChunkStore::CompareChunkName(
    const ChunkDetails &chunk_detail) const {
  for (auto it = chunk_details_.begin(); it != chunk_details_.end(); ++it) {
    if ((*it).name == chunk_detail.name)
      return true;
  }
  return false;
}
void BufferedChunkStore::StoreInFile(const std::string &name,
                                     const std::string &contents) {
  bool result = file_chunk_store_->Store(name, contents);

  if (result) {
    UniqueLock unique_lock(shared_mutex_);
    if (chunk_details_.size() == kChunkContainerCapacity)
      chunk_details_.pop_back();

    boost::posix_time::ptime now(
        boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
    memory_chunk_store_->Store(name, contents);
  }
}
void BufferedChunkStore::StoreInFile(const std::string &name,
                                     const fs::path &source_file_name,
                                     bool delete_source_file) {
  bool result = file_chunk_store_->Store(name, source_file_name, false);
  if (result) {
    UniqueLock unique_lock(shared_mutex_);
    if (chunk_details_.size() == kChunkContainerCapacity)
      chunk_details_.pop_back();

    boost::posix_time::ptime now(
        boost::posix_time::microsec_clock::universal_time());
    ChunkDetails c_details(name, now + kChunkExpiryTime);
    chunk_details_.push_front(c_details);
    memory_chunk_store_->Store(name, source_file_name, delete_source_file);
  }
}
void BufferedChunkStore::CleanExpiredChunks(
    const boost::system::error_code &ec) {
  if (ec)
    return;
  UpgradeLock upgrade_lock(shared_mutex_);
  boost::posix_time::ptime now(
          boost::posix_time::microsec_clock::universal_time());
  for (auto it = chunk_details_.begin(); it != chunk_details_.end(); ++it) {
    if ((*it).expiry_time >= now) {
      UpgradeToUniqueLock unique_lock(upgrade_lock);
      memory_chunk_store_->Delete((*it).name);
      chunk_details_.erase(it);
    }
  }
}
}  // namespace maidsafe
