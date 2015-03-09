/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#include "maidsafe/common/data_buffer.h"

#include <chrono>

#include "boost/filesystem/convenience.hpp"

#include "maidsafe/common/convert.h"
#include "maidsafe/common/encode.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

DataBuffer::DataBuffer(MemoryUsage max_memory_usage, DiskUsage max_disk_usage,
                       PopFunctor pop_functor)
    : memory_store_(max_memory_usage),
      disk_store_(max_disk_usage),
      kPopFunctor_(std::move(pop_functor)),
      kDiskBuffer_(fs::unique_path(fs::temp_directory_path() / "DB-%%%%-%%%%-%%%%-%%%%")),
      kShouldRemoveRoot_(true) {
  Init();
}

DataBuffer::DataBuffer(MemoryUsage max_memory_usage, DiskUsage max_disk_usage,
                       PopFunctor pop_functor, const fs::path& disk_buffer, bool should_remove_root)
    : memory_store_(max_memory_usage),
      disk_store_(max_disk_usage),
      kPopFunctor_(std::move(pop_functor)),
      kDiskBuffer_(disk_buffer),
      kShouldRemoveRoot_(should_remove_root) {
  Init();
}

void DataBuffer::Init() {
  if (memory_store_.max > disk_store_.max) {
    LOG(kError) << "Max memory usage must be < max disk usage.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
  boost::system::error_code error_code;
  if (!fs::exists(kDiskBuffer_, error_code)) {
    if (!fs::create_directories(kDiskBuffer_, error_code)) {
      LOG(kError) << "Can't create disk root at " << kDiskBuffer_ << ": " << error_code.message();
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
      return;
    }
  }
  // Check kDiskBuffer_ is writable
  auto test_file(kDiskBuffer_ / "TestFile");
  if (!WriteFile(test_file, convert::ToByteVector("Test"))) {
    LOG(kError) << "Can't write file " << test_file;
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    return;
  }
  fs::remove(test_file);
  worker_ = std::async(std::launch::async, &DataBuffer::CopyQueueToDisk, this);
}

DataBuffer::~DataBuffer() {
  {
    std::lock(memory_store_.mutex, disk_store_.mutex);
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex, std::adopt_lock);
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex, std::adopt_lock);
    running_ = false;
    memory_store_.cond_var.notify_all();
    disk_store_.cond_var.notify_all();
  }
  {
    std::unique_lock<std::mutex> worker_lock(worker_mutex_);
    while (worker_.valid() &&
           worker_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
      worker_lock.unlock();
      memory_store_.cond_var.notify_all();
      disk_store_.cond_var.notify_all();
      std::this_thread::yield();
      worker_lock.lock();
    }
    if (worker_.valid()) {
      try {
        worker_.get();
      } catch (const std::exception& e) {
        LOG(kError) << boost::diagnostic_information(e);
      }
    }
  }

  if (kShouldRemoveRoot_) {
    boost::system::error_code error_code;
    fs::remove_all(kDiskBuffer_, error_code);
    if (error_code)
      LOG(kWarning) << "Failed to remove " << kDiskBuffer_ << ": " << error_code.message();
  }
}

void DataBuffer::Store(const KeyType& key, const NonEmptyString& value) {
  try {
    Delete(key);
  } catch (const std::exception&) {
    LOG(kVerbose) << "Storing " << DebugKeyName(key) << " with value " << value;
  }

  CheckWorkerIsStillRunning();
  auto disk_store_lock(StoreInMemory(key, value));
  if (disk_store_lock)
    StoreOnDisk(key, value, std::move(disk_store_lock));
}

std::unique_lock<std::mutex> DataBuffer::StoreInMemory(const KeyType& key,
                                                       const NonEmptyString& value) {
  {
    uint64_t required_space(value.string().size());
    std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
    if (required_space > memory_store_.max)
      return std::move(std::unique_lock<std::mutex>(disk_store_.mutex));

    WaitForSpaceInMemory(required_space, memory_store_lock);

    if (!running_) {
      {
        std::lock_guard<std::mutex> worker_lock(worker_mutex_);
        if (worker_.valid())
          worker_.get();
      }
      return std::move(std::unique_lock<std::mutex>());
    }

    memory_store_.current.data += required_space;
    memory_store_.index.emplace_back(key, value);
  }
  memory_store_.cond_var.notify_all();
  return std::move(std::unique_lock<std::mutex>());
}

void DataBuffer::WaitForSpaceInMemory(uint64_t required_space,
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

void DataBuffer::StoreOnDisk(const KeyType& key, const NonEmptyString& value,
                             std::unique_lock<std::mutex>&& disk_store_lock) {
  assert(disk_store_lock);
  if (value.string().size() > disk_store_.max) {
    LOG(kError) << "Cannot store " << DebugKeyName(key) << " since its " << value.string().size()
                << " bytes exceeds max of " << disk_store_.max << " bytes.";
    StopRunning();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::cannot_exceed_limit));
  }
  disk_store_.index.emplace_back(key);

  bool cancelled(false);
  WaitForSpaceOnDisk(key, &value, disk_store_lock, cancelled);
  if (!running_)
    return;

  if (!cancelled) {
    if (!WriteFile(GetFilename(key), value.string())) {
      LOG(kError) << "Failed to move " << DebugKeyName(key) << " to disk.";
      StopRunning();
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::filesystem_io_error));
    }
    auto itr(FindStartedToStoreOnDisk(key));
    if (itr != disk_store_.index.end())
      (*itr).state = StoringState::kCompleted;

    disk_store_.current.data += value.string().size();
  }
  disk_store_lock.unlock();
  disk_store_.cond_var.notify_all();
}

void DataBuffer::WaitForSpaceOnDisk(const KeyType& key, const NonEmptyString* const value,
                                    std::unique_lock<std::mutex>& disk_store_lock,
                                    bool& cancelled) {
  while (!HasSpace(disk_store_, value->string().size()) && running_) {
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
        KeyType oldest_key(itr->key);
        NonEmptyString oldest_value;
        RemoveFile(oldest_key, &oldest_value);
        disk_store_.index.erase(itr);
        kPopFunctor_(oldest_key, oldest_value);
      }
    } else {
      // Rely on client of this class to call Delete until enough space becomes available.  Make the
      // current value available for 'Get' (avoid 'Get' permanent blocking) by adding to
      // 'elements_being_moved_to_disk_'.
      if (running_) {
        elements_being_moved_to_disk_[key] = value;
        disk_store_.cond_var.wait(disk_store_lock);
        elements_being_moved_to_disk_.erase(key);
      }
    }
  }
}

NonEmptyString DataBuffer::Get(const KeyType& key) {
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
    auto temp_itr(elements_being_moved_to_disk_.find(key));
    if (temp_itr != std::end(elements_being_moved_to_disk_))
      return *temp_itr->second;
    disk_store_.cond_var.wait(disk_store_lock, [this, &key]() -> bool {
      auto itr(Find(disk_store_, key));
      return (itr == disk_store_.index.end() || (*itr).state != StoringState::kStarted);
    });
    itr = FindAndThrowIfCancelled(key);
  }
  auto result(ReadFile(GetFilename(key)));
  if (result)
    return NonEmptyString(*result);
  else
    BOOST_THROW_EXCEPTION(result.error());
  // TODO(Fraser#5#): 2012-11-23 - There should maybe be another background task moving the item
  //                               from wherever it's found to the back of the memory index.
}

void DataBuffer::Delete(const KeyType& key) {
  CheckWorkerIsStillRunning();
  StoringState also_on_disk(StoringState::kNotStarted);
  DeleteFromMemory(key, also_on_disk);
  if (also_on_disk != StoringState::kNotStarted)
    DeleteFromDisk(key);
}

void DataBuffer::Delete(std::function<bool(const KeyType&)> predicate) {
  CheckWorkerIsStillRunning();
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto before_size(memory_store_.index.size());
    memory_store_.index.erase(std::remove_if(std::begin(memory_store_.index),
                                             std::end(memory_store_.index),
                                             [&](const MemoryElement& item) {
                                if (predicate(item.key)) {
                                  memory_store_.current.data -= item.value.string().size();
                                  return true;
                                } else {
                                  return false;
                                }
                              }),
                              std::end(memory_store_.index));
    if (memory_store_.index.size() != before_size)
      memory_store_.cond_var.notify_all();
  }
  std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
  auto before_size(disk_store_.index.size());
  disk_store_.index.erase(
      std::remove_if(std::begin(disk_store_.index), std::end(disk_store_.index),
                     [&](const DiskElement& item) { return predicate(item.key); }),
      std::end(disk_store_.index));
  if (disk_store_.index.size() != before_size)
    disk_store_.cond_var.notify_all();
}

void DataBuffer::DeleteFromMemory(const KeyType& key, StoringState& also_on_disk) {
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

void DataBuffer::DeleteFromDisk(const KeyType& key) {
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    auto itr(Find(disk_store_, key));
    if (itr == disk_store_.index.end()) {
      LOG(kWarning) << DebugKeyName(key) << " is not in the disk index.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::no_such_element));
    }

    if ((*itr).state == StoringState::kStarted) {
      (*itr).state = StoringState::kCancelled;
    } else if ((*itr).state == StoringState::kCompleted) {
      RemoveFile(itr->key, nullptr);
      disk_store_.index.erase(itr);
    }
  }
  disk_store_.cond_var.notify_all();
}

void DataBuffer::RemoveFile(const KeyType& key, NonEmptyString* value) {
  auto path(GetFilename(key));
  boost::system::error_code error_code;
  uint64_t size(fs::file_size(path, error_code));
  if (error_code) {
    LOG(kError) << "Error getting file size of " << path << ": " << error_code.message();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::filesystem_io_error));
  }
  if (value) {
    auto contents(ReadFile(path));
    if (contents)
      *value = NonEmptyString(std::move(*contents));
    else
      BOOST_THROW_EXCEPTION(contents.error());
  }
  if (!fs::remove(path, error_code) || error_code) {
    LOG(kError) << "Error removing " << path << ": " << error_code.message();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::filesystem_io_error));
  }
  disk_store_.current.data -= size;
}

void DataBuffer::CopyQueueToDisk() {
  KeyType key;
  NonEmptyString value;
  for (;;) {
    {
      // Get oldest value not yet stored to disk
      std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
      auto itr(memory_store_.index.end());

      memory_store_.cond_var.wait(memory_store_lock, [this, &itr]() -> bool {
        itr = FindOldestInMemoryOnly();
        return itr != memory_store_.index.end() || !running_;
      });
      if (!running_)
        return;

      assert(itr != memory_store_.index.end());
      key = (*itr).key;
      value = (*itr).value;
      (*itr).also_on_disk = StoringState::kStarted;
      std::unique_lock<std::mutex> disk_store_lock(disk_store_.mutex);
      memory_store_lock.unlock();
      StoreOnDisk(key, value, std::move(disk_store_lock));
      memory_store_lock.lock();
      itr = Find(memory_store_, key);
      if (itr != memory_store_.index.end())
        (*itr).also_on_disk = StoringState::kCompleted;
    }
    memory_store_.cond_var.notify_all();
  }
}

void DataBuffer::CheckWorkerIsStillRunning() {
  // if this goes ready then we have an exception so get that (throw basically)
  {
    std::lock_guard<std::mutex> worker_lock(worker_mutex_);
    if (worker_.valid() && worker_.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      worker_.get();
  }
  if (!running_) {
    LOG(kError) << "Worker is no longer running.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::filesystem_io_error));
  }
}

void DataBuffer::StopRunning() {
  running_ = false;
  memory_store_.cond_var.notify_all();
  disk_store_.cond_var.notify_all();
}

fs::path DataBuffer::GetFilename(const KeyType& key) const {
  return kDiskBuffer_ / detail::GetFileName(key);
}

void DataBuffer::SetMaxMemoryUsage(MemoryUsage max_memory_usage) {
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    if (max_memory_usage > disk_store_.max) {
      LOG(kError) << "Max memory usage must be <= max disk usage.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
    }
    memory_store_.max = max_memory_usage;
  }
  memory_store_.cond_var.notify_all();
}

void DataBuffer::SetMaxDiskUsage(DiskUsage max_disk_usage) {
  bool increased(false);
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    if (memory_store_.max > max_disk_usage) {
      LOG(kError) << "Max memory usage must be <= max disk usage.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
    }
    increased = (max_disk_usage > disk_store_.max);
    disk_store_.max = max_disk_usage;
  }
  if (increased)
    disk_store_.cond_var.notify_all();
}

template <typename T>
bool DataBuffer::HasSpace(const T& store, uint64_t required_space) const {
  assert(store.max >= required_space);
  return store.current <= store.max - required_space;
}

template <typename T>
typename T::index_type::iterator DataBuffer::Find(T& store, const KeyType& key) {
  return std::find_if(
      store.index.begin(), store.index.end(),
      [&key](const typename T::index_type::value_type& key_value) { return key_value.key == key; });
}

DataBuffer::MemoryIndex::iterator DataBuffer::FindOldestInMemoryOnly() {
  return std::find_if(memory_store_.index.begin(), memory_store_.index.end(),
                      [](const MemoryElement& key_value) {
    return key_value.also_on_disk == StoringState::kNotStarted;
  });
}

DataBuffer::MemoryIndex::iterator DataBuffer::FindMemoryRemovalCandidate(
    uint64_t required_space, std::unique_lock<std::mutex>& memory_store_lock) {
  auto itr(memory_store_.index.end());
  memory_store_.cond_var.wait(memory_store_lock, [this, &itr, &required_space]() -> bool {
    itr = std::find_if(memory_store_.index.begin(), memory_store_.index.end(),
                       [](const MemoryElement& key_value) {
      return key_value.also_on_disk == StoringState::kCompleted;
    });
    return itr != memory_store_.index.end() || HasSpace(memory_store_, required_space) || !running_;
  });
  return itr;
}

DataBuffer::DiskIndex::iterator DataBuffer::FindStartedToStoreOnDisk(const KeyType& key) {
  return std::find_if(disk_store_.index.begin(), disk_store_.index.end(),
                      [&key](const DiskElement& entry) {
    return entry.state == StoringState::kStarted && entry.key == key;
  });
}

DataBuffer::DiskIndex::iterator DataBuffer::FindOldestOnDisk() { return disk_store_.index.begin(); }

DataBuffer::DiskIndex::iterator DataBuffer::FindAndThrowIfCancelled(const KeyType& key) {
  auto itr(Find(disk_store_, key));
  if (itr == disk_store_.index.end() || (*itr).state == StoringState::kCancelled) {
    LOG(kWarning) << DebugKeyName(key) << " is not in the disk index or is cancelled.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::no_such_element));
  }
  return itr;
}

std::string DataBuffer::DebugKeyName(const KeyType& key) { return hex::Encode(key.name); }

}  // namespace maidsafe
