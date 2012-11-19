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

#ifndef MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_
#define MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_

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

class KeyValueBuffer {
 public:
  typedef std::function<std::pair<Identity, NonEmptyString>()> PopFunctor;
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
  // can't be written to disk (e.g. value is not initialised).
  void Store(const Identity& key, const NonEmptyString& value);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value can't be read from disk.
  NonEmptyString Get(const Identity& key);
  // Throws if the background worker has thrown (e.g. the disk has become inaccessible).  Throws if
  // the value was written to disk and can't be removed.
  void Delete(const Identity& key);
  // Throws if max_memory_usage > max_disk_usage_.
  void SetMaxMemoryUsage(MemoryUsage max_memory_usage);
  // Throws if max_memory_usage_ > max_disk_usage.
  void SetMaxDiskUsage(DiskUsage max_disk_usage);

 private:
  KeyValueBuffer(const KeyValueBuffer&);
  KeyValueBuffer& operator=(const KeyValueBuffer&);
  template<typename T>
  struct Storage {
    explicit Storage(T max_in) : max(max_in), current(0), mutex() {}  // NOLINT (Fraser)
    T max, current;
    std::mutex mutex;
    std::condition_variable cond_var;
  };
  struct KeyValueInfo {
    KeyValueInfo(const Identity& key_in, const NonEmptyString& value_in)
        : key(key_in), value(value_in), on_disk(false) {}
    Identity key;
    NonEmptyString value;
    bool on_disk;
  };

  void Init();
  bool StoreInMemory(const Identity& key, const NonEmptyString& value);
  void StoreOnDisk(const Identity& key, const NonEmptyString& value);
  void DeleteFromMemory(const Identity& key, bool& also_on_disk);
  void DeleteFromDisk(const Identity& key);
  void CopyQueueToDisk();
  void CheckWorkerIsStillRunning();
  boost::filesystem::path GetFilename(const Identity& key) const;

  Storage<MemoryUsage> memory_store_;
  Storage<DiskUsage> disk_store_;
  const PopFunctor kPopFunctor_;
  const boost::filesystem::path kDiskBuffer_;
  const bool kShouldRemoveRoot_;
  bool running_;
  std::deque<KeyValueInfo> values_;
  std::condition_variable worker_cond_var_;
  std::future<void> worker_;
};

}  // namespace maidsafe


#endif  // MAIDSAFE_COMMON_KEY_VALUE_BUFFER_H_
