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

namespace maidsafe {

// Class providing fixed-size (by number of records)
// and / or time_to_live LRU-replacement cache
template <typename KeyType, typename ValueType>
class LruCache {
 public:
  explicit LruCache(size_t capacity)
      : capacity_(capacity), time_to_live_(std::chrono::steady_clock::duration::zero()) {}

  explicit LruCache(std::chrono::steady_clock::duration time_to_live)
      : capacity_(std::numeric_limits<size_t>::max()), time_to_live_(time_to_live) {}

  LruCache(size_t capacity, std::chrono::steady_clock::duration time_to_live)
      : time_to_live_(time_to_live), capacity_(capacity) {}

  ~LruCache() = default;
  LruCache(const LruCache&) = delete;
  LruCache(LruCache&&) = delete;
  LruCache& operator=(const LruCache&) = delete;
  LruCache& operator=(LruCache&&) = delete;

  // We do not return an iterator here and use a pair instead as we are keeping two containers in
  // sync and cannot allow access to these containers from the public interface
  std::pair<bool, ValueType> Get(const KeyType& key) {
    const auto it = storage_.find(key);

    if (it == storage_.end()) {
      return std::make_pair(false, ValueType());
    } else {
      // Update access record by moving accessed key to back of list
      key_order_.splice(key_order_.end(), key_order_, std::get<1>(it->second));
      return std::make_pair(true, std::get<0>(it->second));
    }
  }

  bool Check(const KeyType& key) const { return storage_.find(key) != storage_.end(); }

  void Add(KeyType key, ValueType value) {
    if (storage_.find(key) != storage_.end())
      return;

    // Check if we should evict any entries because of size
    if (storage_.size() == capacity_)
      RemoveOldestElement();
    // Check if we have entries with time expired
    while (CheckTimeExpired())  // Any old entries at beginning of the list
      RemoveOldestElement();

    // Record key as most-recently-used key
    auto it = key_order_.insert(std::end(key_order_), key);

    // Create the key-value entry, linked to the usage record.
    storage_.insert(std::make_pair(
        std::move(key), std::make_tuple(std::move(value), it, std::chrono::steady_clock::now())));
  }

  size_t size() const { return storage_.size(); }

 private:
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
    return (std::get<2>(key->second) + time_to_live_) < std::chrono::steady_clock::now();
  }

  const size_t capacity_;
  const std::chrono::steady_clock::duration time_to_live_;
  std::list<KeyType> key_order_;
  std::map<KeyType, std::tuple<ValueType, typename std::list<KeyType>::iterator,
                               std::chrono::steady_clock::time_point>> storage_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CONTAINERS_LRU_CACHE_H_
