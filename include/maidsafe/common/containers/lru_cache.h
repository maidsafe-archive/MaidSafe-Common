/*  Copyright 2014 MaidSafe.net limited

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

/*
  A least recently used cache that has a capacity and time to live setting. Passing a void ValueType
  allows this object to be used as a firewall / filter type device that can hold and check
  for keys already seen. Users can set the capacity, time_to_live or both allowing a cache that will
  not hold data too long or stay full if it's not being accessed frequently. This should allow the
  cache to not hold stale information at the cost of a check every time we add that looks
  at the timestamp of the last entry in the list and compares this to the maps timestamp.

  Research links
  http://en.wikipedia.org/wiki/Cache_algorithms
  http://stackoverflow.com/questions/1935777/c-design-how-to-cache-most-recent-used
  http://timday.bitbucket.org/lru.html#x1-8007r1 (main inspiration for this implementation)
  http://www.ncbi.nlm.nih.gov/IEB/ToolBox/CPP_DOC/lxr/source/include/util/cache/icache.hpp
*/

#ifndef MAIDSAFE_COMMON_CONTAINERS_LRU_CACHE_H_
#define MAIDSAFE_COMMON_CONTAINERS_LRU_CACHE_H_

#include <cassert>
#include <chrono>
#include <limits>
#include <list>
#include <map>
#include <tuple>
#include <utility>

#include "boost/expected/expected.hpp"

#include "maidsafe/common/types.h"

namespace maidsafe {

namespace detail {

// Helper classes
template <typename KeyType>
using KeyOrder = std::list<KeyType>;

template <typename T>
struct TypeHelper {
  using type = T;
};

template <typename KeyType, typename T>
struct StorageType
    : TypeHelper<std::map<KeyType, std::tuple<typename KeyOrder<KeyType>::iterator,
                                              std::chrono::steady_clock::time_point, T>>> {};

template <typename KeyType>
struct StorageType<KeyType, void>
    : TypeHelper<std::map<KeyType, std::tuple<typename KeyOrder<KeyType>::iterator,
                                              std::chrono::steady_clock::time_point>>> {};

// Base class providing fixed-size (by number of records) and / or time_to_live LRU-replacement
// cache
template <typename KeyType, typename ValueType>
class LruCacheBase {
 public:
  explicit LruCacheBase(size_t capacity)
      : capacity_(capacity), time_to_live_(std::chrono::steady_clock::duration::zero()) {}

  explicit LruCacheBase(std::chrono::steady_clock::duration time_to_live)
      : capacity_(std::numeric_limits<size_t>::max()), time_to_live_(time_to_live) {}

  LruCacheBase(size_t capacity, std::chrono::steady_clock::duration time_to_live)
      : capacity_(capacity), time_to_live_(time_to_live) {}

  virtual ~LruCacheBase() = default;
  LruCacheBase(const LruCacheBase&) = delete;
  LruCacheBase(LruCacheBase&&) = delete;
  LruCacheBase& operator=(const LruCacheBase&) = delete;
  LruCacheBase& operator=(LruCacheBase&&) = delete;

  bool Check(const KeyType& key) const { return storage_.find(key) != storage_.end(); }

  size_t size() const { return storage_.size(); }

 protected:
  template <typename T>
  using Storage = typename StorageType<KeyType, T>::type;

  typename KeyOrder<KeyType>::iterator PrepareToAdd(const KeyType& key) {
    if (storage_.find(key) != storage_.end())
      return std::end(key_order_);
    // Check if we should evict any entries because of size
    if (storage_.size() == capacity_)
      RemoveOldestElement();
    // Check if we have entries with time expired
    while (CheckTimeExpired())  // Any old entries at beginning of the list
      RemoveOldestElement();

    // Record key as most-recently-used key
    return key_order_.insert(std::end(key_order_), key);
  }

  void RemoveOldestElement() {
    assert(!key_order_.empty());
    // Identify least recently used key
    const auto it = storage_.find(key_order_.front());
    assert(it != storage_.end());
    // Erase both elements in both containers
    storage_.erase(it);
    key_order_.pop_front();
  }

  bool CheckTimeExpired() const {
    if (time_to_live_ == std::chrono::steady_clock::duration::zero() || storage_.empty())
      return false;
    auto key = storage_.find(key_order_.front());
    assert(key != std::end(storage_) && "cannot find element");
    return (std::get<1>(key->second) + time_to_live_) < std::chrono::steady_clock::now();
  }

  const size_t capacity_;
  const std::chrono::steady_clock::duration time_to_live_;
  KeyOrder<KeyType> key_order_;
  Storage<ValueType> storage_;
};

}  // namespace detail

// Class providing fixed-size (by number of records) and / or time_to_live LRU-replacement cache
template <typename KeyType, typename ValueType>
class LruCache : public detail::LruCacheBase<KeyType, ValueType> {
 public:
  explicit LruCache(size_t capacity) : detail::LruCacheBase<KeyType, ValueType>(capacity) {}

  explicit LruCache(std::chrono::steady_clock::duration time_to_live)
      : detail::LruCacheBase<KeyType, ValueType>(time_to_live) {}

  LruCache(size_t capacity, std::chrono::steady_clock::duration time_to_live)
      : detail::LruCacheBase<KeyType, ValueType>(capacity, time_to_live) {}

  virtual ~LruCache() = default;
  LruCache(const LruCache&) = delete;
  LruCache(LruCache&&) = delete;
  LruCache& operator=(const LruCache&) = delete;
  LruCache& operator=(LruCache&&) = delete;

  // We do not return an iterator here and use a pair instead as we are keeping two containers in
  // sync and cannot allow access to these containers from the public interface
  boost::expected<ValueType, maidsafe_error> Get(const KeyType& key) {
    const auto it = this->storage_.find(key);

    if (it == this->storage_.end())
      return boost::make_unexpected(MakeError(CommonErrors::no_such_element));

    // Update access record by moving accessed key to back of list
    this->key_order_.splice(this->key_order_.end(), this->key_order_, std::get<0>(it->second));
    return std::get<2>(it->second);
  }

  void Add(KeyType key, ValueType value) {
    auto it = this->PrepareToAdd(key);
    if (it == std::end(this->key_order_))
      return;
    // Create the key-value entry, linked to the usage record.
    this->storage_.insert(std::make_pair(
        std::move(key), std::make_tuple(it, std::chrono::steady_clock::now(), std::move(value))));
  }

  void Delete(const KeyType& key) {
    const auto it = this->storage_.find(key);
    if (it != this->storage_.end()) {
      std::get<1>(it->second) = std::chrono::steady_clock::now() - this->time_to_live_;
      this->key_order_.splice(this->key_order_.begin(), this->key_order_, std::get<0>(it->second));
      this->RemoveOldestElement();
    }
  }
};

// Class providing fixed-size (by number of records) and / or time_to_live LRU-replacement filter
template <typename KeyType>
class LruCache<KeyType, void> : public detail::LruCacheBase<KeyType, void> {
 public:
  explicit LruCache(size_t capacity) : detail::LruCacheBase<KeyType, void>(capacity) {}

  explicit LruCache(std::chrono::steady_clock::duration time_to_live)
      : detail::LruCacheBase<KeyType, void>(time_to_live) {}

  LruCache(size_t capacity, std::chrono::steady_clock::duration time_to_live)
      : detail::LruCacheBase<KeyType, void>(capacity, time_to_live) {}

  virtual ~LruCache() = default;
  LruCache(const LruCache&) = delete;
  LruCache(LruCache&&) = delete;
  LruCache& operator=(const LruCache&) = delete;
  LruCache& operator=(LruCache&&) = delete;

  void Add(KeyType key) {
    auto it = this->PrepareToAdd(key);
    if (it == std::end(this->key_order_))
      return;
    // Create the key entry, linked to the usage record.
    this->storage_.insert(
        std::make_pair(std::move(key), std::make_tuple(it, std::chrono::steady_clock::now())));
  }
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CONTAINERS_LRU_CACHE_H_
