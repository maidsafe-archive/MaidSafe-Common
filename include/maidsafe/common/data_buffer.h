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

#ifndef MAIDSAFE_COMMON_DATA_BUFFER_H_
#define MAIDSAFE_COMMON_DATA_BUFFER_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <string>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/common/data_types/data.h"

namespace maidsafe {

namespace test {

class DataBufferTest;
class DataStoreTest;

}  // namespace test

class DataBuffer {
 public:
  using KeyType = Data::NameAndTypeId;
  using PopFunctor = std::function<void(const KeyType&, const NonEmptyString&)>;

  DataBuffer() = delete;
  DataBuffer(const DataBuffer&) = delete;
  DataBuffer(DataBuffer&&) = delete;
  DataBuffer& operator=(const DataBuffer&) = delete;
  DataBuffer& operator=(DataBuffer&&) = delete;

  // Throws if max_memory_usage >= max_disk_usage.  Throws if a writable folder can't be created in
  // temp_directory_path().  Starts a background worker thread which copies values from memory to
  // disk.  If pop_functor is valid, the disk cache will pop excess items when it is full,
  // otherwise Store will block until there is space made via Delete calls.
  DataBuffer(MemoryUsage max_memory_usage, DiskUsage max_disk_usage, PopFunctor pop_functor);
  // Throws if max_memory_usage >= max_disk_usage.  Throws if a writable folder can't be created in
  // "disk_buffer".  Starts a background worker thread which copies values from memory to disk.  If
  // pop_functor is valid, the disk cache will pop excess items when it is full, otherwise Store
  // will block until there is space made via Delete calls.
  DataBuffer(MemoryUsage max_memory_usage, DiskUsage max_disk_usage, PopFunctor pop_functor,
             const boost::filesystem::path& disk_buffer, bool should_remove_root = false);
  ~DataBuffer();
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the size of value is greater than the current specified maximum disk usage, or if the value
  // can't be written to disk (e.g. value is not initialised).  If there is not enough space to
  // store to memory, blocks until there is enough space to store to disk.  Space will be made
  // available via external calls to Delete, and also automatically if pop_functor_ is not NULL.
  void Store(const KeyType& key, const NonEmptyString& value);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value can't be read from disk.  If the value isn't in memory and has started to be stored
  // to disk, blocks briefly while waiting for the storing to complete.
  NonEmptyString Get(const KeyType& key);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value was written to disk and can't be removed.
  void Delete(const KeyType& key);
  // Delete based on a predicate, allows pairs etc. to be used as key
  void Delete(std::function<bool(const KeyType&)> predicate);
  // Throws if max_memory_usage > max_disk_usage_.
  void SetMaxMemoryUsage(MemoryUsage max_memory_usage);
  // Throws if max_memory_usage_ > max_disk_usage.
  void SetMaxDiskUsage(DiskUsage max_disk_usage);

  friend class test::DataBufferTest;
  friend class test::DataStoreTest;

 private:
  template <typename UsageType, typename IndexType>
  struct Storage {
    using index_type = IndexType;
    explicit Storage(UsageType max_in)
        : max(std::move(max_in)),  // NOLINT
          current(0),
          index(),
          mutex(),
          cond_var() {}
    UsageType max, current;
    IndexType index;
    std::mutex mutex;
    std::condition_variable cond_var;
  };

  enum class StoringState { kNotStarted, kStarted, kCancelled, kCompleted };

  struct MemoryElement {
    MemoryElement(KeyType key_in, NonEmptyString value_in)
        : key(std::move(key_in)),
          value(std::move(value_in)),
          also_on_disk(StoringState::kNotStarted) {}
    KeyType key;
    NonEmptyString value;
    StoringState also_on_disk;
  };

  using MemoryIndex = std::deque<MemoryElement>;

  struct DiskElement {
    explicit DiskElement(KeyType key_in) : key(std::move(key_in)), state(StoringState::kStarted) {}
    KeyType key;
    StoringState state;
  };
  using DiskIndex = std::deque<DiskElement>;

  void Init();

  std::unique_lock<std::mutex> StoreInMemory(const KeyType& key, const NonEmptyString& value);
  void WaitForSpaceInMemory(uint64_t required_space,
                            std::unique_lock<std::mutex>& memory_store_lock);
  void StoreOnDisk(const KeyType& key, const NonEmptyString& value,
                   std::unique_lock<std::mutex>&& disk_store_lock);
  void WaitForSpaceOnDisk(const KeyType& key, const NonEmptyString* const value,
                          std::unique_lock<std::mutex>& disk_store_lock, bool& cancelled);
  void DeleteFromMemory(const KeyType& key, StoringState& also_on_disk);
  void DeleteFromDisk(const KeyType& key);
  void RemoveFile(const KeyType& key, NonEmptyString* value);

  void CopyQueueToDisk();
  void CheckWorkerIsStillRunning();
  void StopRunning();
  boost::filesystem::path GetFilename(const KeyType& key) const;

  template <typename T>
  bool HasSpace(const T& store, uint64_t required_space) const;

  template <typename T>
  typename T::index_type::iterator Find(T& store, const KeyType& key);

  MemoryIndex::iterator FindOldestInMemoryOnly();
  MemoryIndex::iterator FindMemoryRemovalCandidate(
      uint64_t required_space, std::unique_lock<std::mutex>& memory_store_lock);

  DiskIndex::iterator FindStartedToStoreOnDisk(const KeyType& key);
  DiskIndex::iterator FindOldestOnDisk();

  DiskIndex::iterator FindAndThrowIfCancelled(const KeyType& key);

  std::string DebugKeyName(const KeyType& key);

  Storage<MemoryUsage, MemoryIndex> memory_store_;
  Storage<DiskUsage, DiskIndex> disk_store_;
  const PopFunctor kPopFunctor_;
  const boost::filesystem::path kDiskBuffer_;
  const bool kShouldRemoveRoot_;
  std::map<KeyType, const NonEmptyString*> elements_being_moved_to_disk_{};
  std::atomic<bool> running_{true};
  std::mutex worker_mutex_{};
  std::future<void> worker_{};
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_BUFFER_H_
