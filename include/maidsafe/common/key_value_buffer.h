/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_
#define MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <utility>
#include <deque>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"


namespace maidsafe {

namespace test { class KeyValueBufferTest; }

class KeyValueBuffer {
 public:
  typedef std::function<void(const Identity&, const NonEmptyString&)> PopFunctor;
  // Throws if max_memory_usage >= max_disk_usage.  Throws if a writable folder can't be created in
  // temp_directory_path().  Starts a background worker thread which copies values from memory to
  // disk.  If pop_functor is valid, the disk cache will pop excess items when it is full,
  // otherwise Store will block until there is space made via Delete calls.
  KeyValueBuffer(MemoryUsage max_memory_usage, DiskUsage max_disk_usage, PopFunctor pop_functor);
  // Throws if max_memory_usage >= max_disk_usage.  Throws if a writable folder can't be created in
  // "disk_buffer".  Starts a background worker thread which copies values from memory to disk.  If
  // pop_functor is valid, the disk cache will pop excess items when it is full, otherwise Store
  // will block until there is space made via Delete calls.
  KeyValueBuffer(MemoryUsage max_memory_usage,
                 DiskUsage max_disk_usage,
                 PopFunctor pop_functor,
                 const boost::filesystem::path& disk_buffer);
  ~KeyValueBuffer();
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the size of value is greater than the current specified maximum disk usage, or if the value
  // can't be written to disk (e.g. value is not initialised).  If there is not enough space to
  // store to memory, blocks until there is enough space to store to disk.  Space will be made
  // available via external calls to Delete, and also automatically if pop_functor_ is not NULL.
  void Store(const Identity& key, const NonEmptyString& value);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value can't be read from disk.  If the value isn't in memory and has started to be stored
  // to disk, blocks while waiting for the storing to complete.
  NonEmptyString Get(const Identity& key);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value was written to disk and can't be removed.
  void Delete(const Identity& key);
  // Throws if max_memory_usage > max_disk_usage_.
  void SetMaxMemoryUsage(MemoryUsage max_memory_usage);
  // Throws if max_memory_usage_ > max_disk_usage.
  void SetMaxDiskUsage(DiskUsage max_disk_usage);

  friend class test::KeyValueBufferTest;

 private:
  KeyValueBuffer(const KeyValueBuffer&);
  KeyValueBuffer& operator=(const KeyValueBuffer&);

  template<typename UsageType, typename IndexType>
  struct Storage {
    typedef IndexType index_type;
    explicit Storage(UsageType max_in) : max(max_in), current(0), index(), mutex(), cond_var() {}  // NOLINT (Fraser)
    UsageType max, current;
    IndexType index;
    std::mutex mutex;
    std::condition_variable cond_var;
  };

  enum class StoringState { kNotStarted, kStarted, kCancelled, kCompleted };

  struct MemoryElement {
    MemoryElement(const Identity& key_in, const NonEmptyString& value_in)
        : key(key_in), value(value_in), also_on_disk(StoringState::kNotStarted) {}
    Identity key;
    NonEmptyString value;
    StoringState also_on_disk;
  };
  typedef std::deque<MemoryElement> MemoryIndex;

  struct DiskElement {
    explicit DiskElement(const Identity& key_in) : key(key_in), state(StoringState::kStarted) {}
    Identity key;
    StoringState state;
  };
  typedef std::deque<DiskElement> DiskIndex;

  void Init();
  std::unique_lock<std::mutex> StoreInMemory(const Identity& key, const NonEmptyString& value);
  void WaitForSpaceInMemory(const uint64_t& required_space,
                            std::unique_lock<std::mutex>& memory_store_lock);
  void StoreOnDisk(const Identity& key,
                   const NonEmptyString& value,
                   std::unique_lock<std::mutex>&& disk_store_lock);
  void WaitForSpaceOnDisk(const Identity& key,
                          const uint64_t& required_space,
                          std::unique_lock<std::mutex>& disk_store_lock,
                          bool& cancelled);
  void DeleteFromMemory(const Identity& key, StoringState& also_on_disk);
  void DeleteFromDisk(const Identity& key);
  void RemoveFile(const Identity& key, NonEmptyString* value);
  void CopyQueueToDisk();
  void CheckWorkerIsStillRunning();
  void StopRunning();
  boost::filesystem::path GetFilename(const Identity& key) const;
  template<typename T>
  bool HasSpace(const T& store, const uint64_t& required_space);
  template<typename T>
  typename T::index_type::iterator Find(T& store, const Identity& key);
  MemoryIndex::iterator FindOldestInMemoryOnly();
  MemoryIndex::iterator FindMemoryRemovalCandidate(const uint64_t& required_space,
                                                   std::unique_lock<std::mutex>& memory_store_lock);
  DiskIndex::iterator FindStartedToStoreOnDisk(const Identity& key);
  DiskIndex::iterator FindOldestOnDisk();
  DiskIndex::iterator FindAndThrowIfCancelled(const Identity& key);

  Storage<MemoryUsage, MemoryIndex> memory_store_;
  Storage<DiskUsage, DiskIndex> disk_store_;
  const PopFunctor kPopFunctor_;
  const boost::filesystem::path kDiskBuffer_;
  const bool kShouldRemoveRoot_;
  std::atomic<bool> running_;
  std::mutex worker_mutex_;
  std::future<void> worker_;
};

}  // namespace maidsafe


#endif  // MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_
