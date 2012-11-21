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
      values_(),
      worker_cond_var_(),
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
      values_(),
      worker_cond_var_(),
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
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    running_ = false;
  }
  disk_store_.cond_var.notify_all();
  worker_cond_var_.notify_all();
  if (worker_.valid()) {
    try {
      worker_.get();
    }
    catch(const std::exception& e) {
      LOG(kError) << e.what();
    }
  }
  memory_store_.cond_var.notify_all();

  if (kShouldRemoveRoot_) {
    boost::system::error_code error_code;
    fs::remove_all(kDiskBuffer_, error_code);
    if (error_code)
      LOG(kWarning) << "Failed to remove " << kDiskBuffer_ << ": " << error_code.message();
  }
}

void KeyValueBuffer::Store(const Identity& key, const NonEmptyString& value) {
  CheckWorkerIsStillRunning();
  if (!StoreInMemory(key, value))
    StoreOnDisk(key, value);
}

bool KeyValueBuffer::StoreInMemory(const Identity& key, const NonEmptyString& value) {
  {
    std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
    if (value.string().size() > memory_store_.max)
      return false;

    bool running(false);
    while (memory_store_.current > memory_store_.max - value.string().size()) {
      auto itr(values_.end());
      memory_store_.cond_var.wait(memory_store_lock, [this, &itr, &running] {
          {
            std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
            running = running_;
          }
          itr = std::find_if(values_.begin(),
                             values_.end(),
                             [](const KeyValueInfo& key_value) {
                                 return key_value.on_disk != OnDisk::kNotStarted &&
                                        key_value.value.IsInitialised();
                             });
          return itr != values_.end() || !running;
      });
      if (!running)
        worker_.get();

      memory_store_.current.data -= (*itr).value.string().size();
      if (kPopFunctor_)
        (*itr).value = NonEmptyString();
      else
        values_.pop_front();
    }

    memory_store_.current.data += value.string().size();
    values_.push_back(KeyValueInfo(key, value));
  }
  worker_cond_var_.notify_all();
  return true;
}

void KeyValueBuffer::StoreOnDisk(const Identity& key, const NonEmptyString& value) {
  {
    std::unique_lock<std::mutex> disk_store_lock(disk_store_.mutex);
    if (value.string().size() > disk_store_.max) {
      LOG(kError) << "Cannot store " << HexSubstr(key) << " since its " << value.string().size()
                  << " bytes exceeds max of " << disk_store_.max << " bytes.";
      running_ = false;
      worker_cond_var_.notify_all();
      ThrowError(CommonErrors::cannot_exceed_max_disk_usage);
    }

    WaitForSpaceOnDisk(value.string().size(), disk_store_lock);
    if (!running_)
      return;

    if (!WriteFile(GetFilename(key), value.string())) {
      LOG(kError) << "Failed to move " << HexSubstr(key) << " to disk.";
      running_ = false;
      worker_cond_var_.notify_all();
      ThrowError(CommonErrors::filesystem_io_error);
    }
                                                              LOG(kSuccess) << "Stored " << GetFilename(key);
    disk_store_.current.data += value.string().size();
  }
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(std::find_if(values_.begin(),
                          values_.end(),
                          [](const KeyValueInfo& key_value) {
                              return key_value.on_disk == OnDisk::kStarted;
                          }));
    if (itr != values_.end()) {
      assert((*itr).key == key);
      (*itr).on_disk = OnDisk::kCompleted;
    }
  }
}

void KeyValueBuffer::WaitForSpaceOnDisk(const uint64_t& space_required,
                                        std::unique_lock<std::mutex>& disk_store_lock) {
  if (kPopFunctor_) {
    while (disk_store_.current > disk_store_.max - space_required) {
      std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
      auto itr(values_.end());
      memory_store_.cond_var.wait(memory_store_lock, [this, &itr] {
          itr = std::find_if(values_.begin(),
                             values_.end(),
                             [](const KeyValueInfo& key_value) {
                                 return key_value.on_disk == OnDisk::kCompleted;
                             });
          return itr != values_.end();
      });
      Identity key((*itr).key);
      NonEmptyString value;
      RemoveFile(key, &value);
      values_.pop_front();
      kPopFunctor_(key, value);
    }
  } else {
    // Rely on client of this class to call Delete until enough space becomes available
    disk_store_.cond_var.wait(disk_store_lock, [this, space_required] {
        return (disk_store_.current <= disk_store_.max - space_required) || !running_;
    });
  }
}

NonEmptyString KeyValueBuffer::Get(const Identity& key) {
  CheckWorkerIsStillRunning();
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(std::find_if(values_.begin(),
                          values_.end(),
                          [&key](const KeyValueInfo& key_value) { return key_value.key == key; }));  // NOLINT (Fraser)
    if (itr != values_.end())
      return (*itr).value;
  }
  std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
  return ReadFile(GetFilename(key));
}

void KeyValueBuffer::Delete(const Identity& key) {
  CheckWorkerIsStillRunning();
  OnDisk also_on_disk(OnDisk::kNotStarted);
  DeleteFromMemory(key, also_on_disk);
  if (also_on_disk != OnDisk::kNotStarted)
    DeleteFromDisk(key, also_on_disk == OnDisk::kStarted);
}

void KeyValueBuffer::DeleteFromMemory(const Identity& key, OnDisk& also_on_disk) {
  {
    std::lock_guard<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(std::find_if(values_.begin(),
                          values_.end(),
                          [&key](const KeyValueInfo& key_value) { return key_value.key == key; }));  // NOLINT (Fraser)
    if (itr != values_.end()) {
      also_on_disk = (*itr).on_disk;
      memory_store_.current.data -= (*itr).value.string().size();
      // value gets erased in DeleteFromDisk if the storing process has already started
      if (also_on_disk != OnDisk::kStarted)
        values_.erase(itr);
    } else {
      DeleteFromDisk(key, false);
    }
  }
  memory_store_.cond_var.notify_all();
}

void KeyValueBuffer::DeleteFromDisk(const Identity& key, bool wait_for_storing_to_complete) {
  if (wait_for_storing_to_complete) {
    std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
    auto itr(std::find_if(values_.begin(),
                          values_.end(),
                          [&key](const KeyValueInfo& key_value) {
                              return key_value.on_disk != OnDisk::kNotStarted &&
                                     key_value.key == key;
                          }));
    assert(itr != values_.end());
    worker_cond_var_.wait(memory_store_lock,
                          [this, &itr] { return (*itr).on_disk == OnDisk::kCompleted; });  // NOLINT (Fraser)
    values_.erase(itr);
  }
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    if (!running_)
      return;
    RemoveFile(key, nullptr);
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
  bool running(true);
  for (;;) {
    {
      // Get oldest value not yet stored to disk
      std::unique_lock<std::mutex> memory_store_lock(memory_store_.mutex);
      auto itr(values_.end());
      worker_cond_var_.wait(memory_store_lock, [this, &itr, &running]()->bool {
          {
            std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
            running = running_;
          }
          itr = std::find_if(values_.begin(),
                             values_.end(),
                             [](const KeyValueInfo& key_value) {
                                 return key_value.on_disk == OnDisk::kNotStarted;
                             });
          return itr != values_.end() || !running;
      });
      if (!running)
        return;
      key = (*itr).key;
      value = (*itr).value;
      (*itr).on_disk = OnDisk::kStarted;
    }
    memory_store_.cond_var.notify_one();
    StoreOnDisk(key, value);
    worker_cond_var_.notify_one();
  }
}

void KeyValueBuffer::CheckWorkerIsStillRunning() {
  if (worker_.has_exception())
    worker_.get();
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    if (!running_) {
      LOG(kError) << "Worker is no longer running.";
      ThrowError(CommonErrors::filesystem_io_error);
    }
  }
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
  {
    std::lock_guard<std::mutex> disk_store_lock(disk_store_.mutex);
    if (memory_store_.max > max_disk_usage) {
      LOG(kError) << "Max memory usage must be <= max disk usage.";
      ThrowError(CommonErrors::invalid_parameter);
    }
    disk_store_.max = max_disk_usage;
  }
  disk_store_.cond_var.notify_all();
}

fs::path KeyValueBuffer::GetFilename(const Identity& key) const {
  return kDiskBuffer_ / EncodeToBase32(key);
}


}  // namespace maidsafe
