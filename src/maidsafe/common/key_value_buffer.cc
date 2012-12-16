/* Copyright (c) 2012 maidsafe.net limited
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

#include "maidsafe/common/key_value_buffer.h"

#include "boost/filesystem/convenience.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace {

void InitialiseDiskRoot(const fs::path& disk_root) {
  boost::system::error_code error_code;
  if (!fs::exists(disk_root, error_code)) {
    if (!fs::create_directories(disk_root, error_code)) {
      LOG(kError) << "Can't create disk root at " << disk_root << ": " << error_code.message();
      ThrowError(CommonErrors::uninitialised);
      return;
    }
  }
  // Check kDiskBuffer_ is writable
  fs::path test_file(disk_root / "TestFile");
  if (!WriteFile(test_file, "Test")) {
    LOG(kError) << "Can't write file " << test_file;
    ThrowError(CommonErrors::uninitialised);
    return;
  }
  fs::remove(test_file);
}

}  // unnamed namespace

KeyValueBuffer::KeyValueBuffer(MemoryUsage max_memory_usage,
                               DiskUsage max_disk_usage,
                               PopFunctor pop_functor)
    : memory_store_(max_memory_usage),
      disk_store_(max_disk_usage),
      kPopFunctor_(pop_functor),
      kDiskBuffer_(fs::unique_path(fs::temp_directory_path() / "KVB-%%%%-%%%%-%%%%-%%%%")),
      kShouldRemoveRoot_(true),
      running_(true),
      worker_() {
  Init();
}

KeyValueBuffer::KeyValueBuffer(MemoryUsage max_memory_usage,
                               DiskUsage max_disk_usage,
                               PopFunctor pop_functor,
                               const boost::filesystem::path& disk_buffer)
    : memory_store_(max_memory_usage),
      disk_store_(max_disk_usage),
      kPopFunctor_(pop_functor),
      kDiskBuffer_(disk_buffer),
      kShouldRemoveRoot_(false),
      running_(true),
      worker_() {
  Init();
}

void KeyValueBuffer::Init() {
  if (memory_store_.max > disk_store_.max) {
    LOG(kError) << "Max memory usage must be < max disk usage.";
    ThrowError(CommonErrors::invalid_parameter);
  }
  InitialiseDiskRoot(kDiskBuffer_);
  worker_ = std::async(std::launch::async, &KeyValueBuffer::CopyQueueToDisk, this);
}

KeyValueBuffer::~KeyValueBuffer() {
  {
    std::lock(memory_store_.mutex, disk_store_.mutex);
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex, std::adopt_lock);
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex, std::adopt_lock);
    running_ = false;
  }
  memory_store_.cond_var.notify_all();
  disk_store_.cond_var.notify_all();
  if (worker_.valid()) {
    try {
      worker_.get();
    }
    catch(const std::exception& e) {
      LOG(kError) << e.what();
    }
  }

  if (kShouldRemoveRoot_) {
    boost::system::error_code error_code;
    fs::remove_all(kDiskBuffer_, error_code);
    if (error_code)
      LOG(kWarning) << "Failed to remove " << kDiskBuffer_ << ": " << error_code.message();
  }
}

void KeyValueBuffer::Store(const Identity& key, const NonEmptyString& value) {
  try {
    Delete(key);
    LOG(kInfo) << "Re-storing value " << EncodeToBase32(value) << " with key "
               << EncodeToBase32(key);
  }
  catch(const std::exception&) {
    LOG(kInfo) << "Storing value " << EncodeToBase32(value) << " with key " << EncodeToBase32(key);
  }
  CheckWorkerIsStillRunning();
  if (!StoreInMemory(key, value))
    StoreOnDisk(key, value);
}

bool KeyValueBuffer::StoreInMemory(const Identity& key, const NonEmptyString& value) {
  {
    uint64_t required_space(value.string().size());
    std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
    if (required_space > memory_store_.max)
      return false;

    WaitForSpaceInMemory(required_space, memory_store_lock);

    if (!running_) {
      worker_.get();
      return true;
    }

    memory_store_.current.data += required_space;
    memory_store_.index.emplace_back(key, value);
  }
  memory_store_.cond_var.notify_all();
  return true;
}

void KeyValueBuffer::WaitForSpaceInMemory(const uint64_t& required_space,
                                          std::unique_lock<std::mutex>& memory_store_lock) {
  while (!HasSpace(memory_store_, required_space)) {
    auto itr(FindMemoryRemovalCandidate(required_space, memory_store_lock));
    if (!running_)
      return;

    if (itr != memory_store_.index.end()) {
      memory_store_.current.data -= (*itr).value.string().size();
      memory_store_.index.erase(itr);
    }
  }
}

void KeyValueBuffer::StoreOnDisk(const Identity& key, const NonEmptyString& value) {
  {
    std::unique_lock<std::mutex> disk_store_lock(disk_store_.mutex);
    if (value.string().size() > disk_store_.max) {
      LOG(kError) << "Cannot store " << HexSubstr(key) << " since its " << value.string().size()
                  << " bytes exceeds max of " << disk_store_.max << " bytes.";
      StopRunning();
      ThrowError(CommonErrors::cannot_exceed_max_disk_usage);
    }
    disk_store_.index.emplace_back(key);

    bool cancelled(false);
    WaitForSpaceOnDisk(key, value.string().size(), disk_store_lock, cancelled);
    if (!running_)
      return;

    if (!cancelled) {
      if (!WriteFile(GetFilename(key), value.string())) {
        LOG(kError) << "Failed to move " << HexSubstr(key) << " to disk.";
        StopRunning();
        ThrowError(CommonErrors::filesystem_io_error);
      }
      auto itr(FindStartedToStoreOnDisk(key));
      if (itr != disk_store_.index.end())
        (*itr).state = StoringState::kCompleted;

      disk_store_.current.data += value.string().size();
    }
  }
  disk_store_.cond_var.notify_all();
}

void KeyValueBuffer::WaitForSpaceOnDisk(const Identity& key,
                                        const uint64_t& required_space,
                                        std::unique_lock<std::mutex>& disk_store_lock,
                                        bool& cancelled) {
  while (!HasSpace(disk_store_, required_space) && running_) {
    auto itr(Find(disk_store_, key));
    if (itr == disk_store_.index.end()) {
      cancelled = true;
      return;
    }

    if ((*itr).state == StoringState::kCancelled) {
      disk_store_.index.erase(itr);
      cancelled = true;
      return;
    }

    if (kPopFunctor_) {
      itr = FindOldestOnDisk();
      assert((*itr).state != StoringState::kStarted);
      if ((*itr).state == StoringState::kCompleted) {
        Identity oldest_key((*itr).key);
        NonEmptyString oldest_value;
        RemoveFile((*itr).key, &oldest_value);
        disk_store_.index.erase(itr);
        kPopFunctor_(oldest_key, oldest_value);
      }
    } else {
      // Rely on client of this class to call Delete until enough space becomes available
      if (running_)
        disk_store_.cond_var.wait(disk_store_lock);
    }
  }
}

NonEmptyString KeyValueBuffer::Get(const Identity& key) {
  CheckWorkerIsStillRunning();
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(Find(memory_store_, key));
    if (itr != memory_store_.index.end())
      return (*itr).value;
  }
  std::unique_lock<std::mutex> disk_store_lock(disk_store_.mutex);
  auto itr(FindAndThrowIfCancelled(key));
  if ((*itr).state == StoringState::kStarted) {
    disk_store_.cond_var.wait(disk_store_lock, [this, &key]()->bool {
        auto itr(Find(disk_store_, key));
        return (itr == disk_store_.index.end() || (*itr).state != StoringState::kStarted);
    });
    itr = FindAndThrowIfCancelled(key);
  }
  return ReadFile(GetFilename(key));
  // TODO(Fraser#5#): 2012-11-23 - There should maybe be another background task moving the item
  //                               from wherever it's found to the back of the memory index.
}

void KeyValueBuffer::Delete(const Identity& key) {
  CheckWorkerIsStillRunning();
  StoringState also_on_disk(StoringState::kNotStarted);
  DeleteFromMemory(key, also_on_disk);
  if (also_on_disk != StoringState::kNotStarted)
    DeleteFromDisk(key);
}

void KeyValueBuffer::DeleteFromMemory(const Identity& key, StoringState& also_on_disk) {
  bool changed(false);
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(Find(memory_store_, key));
    if (itr != memory_store_.index.end()) {
      also_on_disk = (*itr).also_on_disk;
      memory_store_.current.data -= (*itr).value.string().size();
      memory_store_.index.erase(itr);
      changed = true;
    } else {
      // Assume it's on disk so as to invoke a DeleteFromDisk
      also_on_disk = StoringState::kCompleted;
    }
  }
  if (changed)
    memory_store_.cond_var.notify_all();
}

void KeyValueBuffer::DeleteFromDisk(const Identity& key) {
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    auto itr(Find(disk_store_, key));
    if (itr == disk_store_.index.end()) {
      LOG(kError) << HexSubstr(key) << " is not in the disk index.";
      ThrowError(CommonErrors::no_such_element);
    }

    if ((*itr).state == StoringState::kStarted) {
      (*itr).state = StoringState::kCancelled;
    } else if ((*itr).state == StoringState::kCompleted) {
      RemoveFile((*itr).key, nullptr);
      disk_store_.index.erase(itr);
    }
  }
  disk_store_.cond_var.notify_all();
}

void KeyValueBuffer::RemoveFile(const Identity& key, NonEmptyString* value) {
  fs::path path(GetFilename(key));
  boost::system::error_code error_code;
  uint64_t size(fs::file_size(path, error_code));
  if (error_code) {
    LOG(kError) << "Error getting file size of " << path << ": " << error_code.message();
    ThrowError(CommonErrors::filesystem_io_error);
  }
  if (value)
    *value = ReadFile(path);
  if (!fs::remove(path, error_code) || error_code) {
    LOG(kError) << "Error removing " << path << ": " << error_code.message();
    ThrowError(CommonErrors::filesystem_io_error);
  }
  disk_store_.current.data -= size;
}

void KeyValueBuffer::CopyQueueToDisk() {
  Identity key;
  NonEmptyString value;
  for (;;) {
    {
      // Get oldest value not yet stored to disk
      std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
      auto itr(memory_store_.index.end());
      memory_store_.cond_var.wait(memory_store_lock, [this, &itr]()->bool {
          itr = FindOldestInMemoryOnly();
          return itr != memory_store_.index.end() || !running_;
      });
      if (!running_)
        return;

      key = (*itr).key;
      value = (*itr).value;
      (*itr).also_on_disk = StoringState::kStarted;
    }
    StoreOnDisk(key, value);
    {
      std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
      auto itr(Find(memory_store_, key));
      if (itr != memory_store_.index.end())
        (*itr).also_on_disk = StoringState::kCompleted;
    }
    memory_store_.cond_var.notify_all();
  }
}

void KeyValueBuffer::CheckWorkerIsStillRunning() {
  if (worker_.has_exception())
    worker_.get();
  if (!running_) {
    LOG(kError) << "Worker is no longer running.";
    ThrowError(CommonErrors::filesystem_io_error);
  }
}

void KeyValueBuffer::StopRunning() {
  running_ = false;
  memory_store_.cond_var.notify_all();
  disk_store_.cond_var.notify_all();
}

void KeyValueBuffer::SetMaxMemoryUsage(MemoryUsage max_memory_usage) {
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    if (max_memory_usage > disk_store_.max) {
      LOG(kError) << "Max memory usage must be <= max disk usage.";
      ThrowError(CommonErrors::invalid_parameter);
    }
    memory_store_.max = max_memory_usage;
  }
  memory_store_.cond_var.notify_all();
}

void KeyValueBuffer::SetMaxDiskUsage(DiskUsage max_disk_usage) {
  bool increased(false);
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    if (memory_store_.max > max_disk_usage) {
      LOG(kError) << "Max memory usage must be <= max disk usage.";
      ThrowError(CommonErrors::invalid_parameter);
    }
    increased = (max_disk_usage > disk_store_.max);
    disk_store_.max = max_disk_usage;
  }
  if (increased)
    disk_store_.cond_var.notify_all();
}

fs::path KeyValueBuffer::GetFilename(const Identity& key) const {
  return kDiskBuffer_ / EncodeToBase32(key);
}

template<typename T>
bool KeyValueBuffer::HasSpace(const T& store, const uint64_t& required_space) {
  assert(store.max >= required_space);
  return store.current <= store.max - required_space;
}

template<typename T>
typename T::index_type::iterator KeyValueBuffer::Find(T& store, const Identity& key) {
  return std::find_if(store.index.begin(),
                      store.index.end(),
                      [&key](const typename T::index_type::value_type& key_value) {
                          return key_value.key == key;
                      });
}

KeyValueBuffer::MemoryIndex::iterator KeyValueBuffer::FindOldestInMemoryOnly() {
  return std::find_if(memory_store_.index.begin(),
                      memory_store_.index.end(),
                      [](const MemoryElement& key_value) {
                          return key_value.also_on_disk == StoringState::kNotStarted;
                      });
}

KeyValueBuffer::MemoryIndex::iterator KeyValueBuffer::FindMemoryRemovalCandidate(
    const uint64_t& required_space,
    std::unique_lock<std::mutex>& memory_store_lock) {
  auto itr(memory_store_.index.end());
  memory_store_.cond_var.wait(memory_store_lock, [this, &itr, &required_space]()->bool {
      itr = std::find_if(memory_store_.index.begin(),
                         memory_store_.index.end(),
                         [](const MemoryElement& key_value) {
                             return key_value.also_on_disk == StoringState::kCompleted;
                         });
      return itr != memory_store_.index.end() ||
             HasSpace(memory_store_, required_space) ||
             !running_;
  });
  return itr;
}

KeyValueBuffer::DiskIndex::iterator KeyValueBuffer::FindStartedToStoreOnDisk(const Identity& key) {
  return std::find_if(disk_store_.index.begin(),
                      disk_store_.index.end(),
                      [&key](const DiskElement& entry) {
                          return entry.state == StoringState::kStarted && entry.key == key;
                      });
}

KeyValueBuffer::DiskIndex::iterator KeyValueBuffer::FindOldestOnDisk() {
  return disk_store_.index.begin();
}

KeyValueBuffer::DiskIndex::iterator KeyValueBuffer::FindAndThrowIfCancelled(const Identity& key) {
  auto itr(Find(disk_store_, key));
  if (itr == disk_store_.index.end() || (*itr).state == StoringState::kCancelled) {
    LOG(kError) << HexSubstr(key) << " is not in the disk index or is cancelled.";
    ThrowError(CommonErrors::no_such_element);
  }
  return itr;
}

}  // namespace maidsafe
