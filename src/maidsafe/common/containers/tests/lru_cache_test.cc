/*  Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/containers/lru_cache.h"

#include <chrono>
#include <thread>

#include "maidsafe/common/test.h"
#include "maidsafe/common/node_id.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace test {

TEST(LruCacheTest, BEH_SizeOnlyTest) {
  auto size(10);
  LruCache<int, int> cache(size);

  for (int i(0); i < 10; ++i) {
    EXPECT_EQ(cache.size(), i);
    cache.Add(i, i);
    EXPECT_EQ(cache.size(), i + 1);
  }

  for (int i(10); i < 1000; ++i) {
    EXPECT_EQ(cache.size(), size);
    cache.Add(i, i);
    EXPECT_EQ(cache.size(), size);
  }

  for (int i(1000); i > 0; --i) {
    EXPECT_TRUE(cache.Check(1000 - 1));
    EXPECT_TRUE(cache.Get(1000 - 1).valid());
    EXPECT_EQ(cache.Get(1000 - 1).value(), 1000 - 1);
  }
}

TEST(LruCacheTest, BEH_DeleteTest) {
  auto size(10);
  LruCache<int, int> cache(size);

  {
    for (int i(0); i < size; ++i) {
      EXPECT_EQ(cache.size(), i);
      cache.Add(i, i);
      EXPECT_EQ(cache.size(), i + 1);
    }

    int random_index(RandomUint32() % size);
    for (int i(0); i < 10; ++i) {
      auto index((i + random_index) % size);
      cache.Delete(index);
      EXPECT_FALSE(cache.Get(index).valid());
    }

    for (int i(0); i < 10; ++i)
      EXPECT_FALSE(cache.Get(i).valid());

    EXPECT_EQ(cache.size(), 0);
  }

  {
    for (int i(0); i < size * 2; ++i)
      cache.Add(i, i);

    EXPECT_EQ(cache.size(), size);

    int random_index(RandomUint32() % size);
    for (int i(size); i < size * 2; ++i) {
      auto index(((i + random_index) % size) + size);
      cache.Delete(index);
      EXPECT_FALSE(cache.Get(index).valid());
    }

    for (int i(size); i < size * 2; ++i)
      EXPECT_FALSE(cache.Get(i).valid());

    EXPECT_EQ(cache.size(), 0);
  }
}

TEST(LruCacheTest, BEH_TimeOnlyTest) {
  std::chrono::milliseconds time(100);
  LruCache<int, int> cache(time);

  for (int i(0); i < 10; ++i) {
    EXPECT_EQ(cache.size(), i);
    cache.Add(i, i);
    EXPECT_EQ(cache.size(), i + 1);
  }
  std::this_thread::sleep_for(time);
  cache.Add(11, 11);

  EXPECT_EQ(cache.size(), 1);

  for (int i(0); i < 8; ++i) {
    EXPECT_EQ(cache.size(), i + 1);
    cache.Add(i, i);
    EXPECT_EQ(cache.size(), i + 2);
  }
}

TEST(LruCacheTest, BEH_TimeAndSizeTest) {
  std::chrono::milliseconds time(100);
  auto size(10);
  LruCache<int, int> cache(size, time);

  for (int i(0); i < 1000; ++i) {
    if (i < size)
      EXPECT_EQ(cache.size(), i);
    cache.Add(i, i);
    // ensure we maintian max size regardless of time
    if (i < size)
      EXPECT_EQ(cache.size(), i + 1);
    else
      EXPECT_EQ(cache.size(), size);
  }
  std::this_thread::sleep_for(time);
  cache.Add(1, 1);
  // are we trimming old stale data even
  EXPECT_EQ(cache.size(), 1);
}

TEST(LruCacheTest, BEH_FilterTimeAndSizeTest) {
  std::chrono::milliseconds time(100);
  auto size(10);
  LruCache<int, void> filter(size, time);

  for (int i(0); i < 1000; ++i) {
    if (i < size)
      EXPECT_EQ(filter.size(), i);
    filter.Add(i);
    // ensure we maintian max size regardless of time
    if (i < size)
      EXPECT_EQ(filter.size(), i + 1);
    else
      EXPECT_EQ(filter.size(), size);
  }
  std::this_thread::sleep_for(time);
  filter.Add(1);
  // are we trimming old stale data even
  EXPECT_EQ(filter.size(), 1);
}

TEST(LruCacheTest, BEH_TimeAndSizeStructValueTest) {
  std::chrono::milliseconds time(100);
  auto size(100);
  struct temp {
    temp() : a{0}, b("a string"), id(NodeId(RandomString(NodeId::kSize))) {}
    int a;
    std::string b;
    NodeId id;
    bool operator<(const temp& other) const {
      return std::tie(a, b, id) < std::tie(other.a, other.b, other.id);
    }
  };
  LruCache<temp, int> cache(size, time);

  for (int i(0); i < 100; ++i) {
    if (i < size)
      EXPECT_EQ(cache.size(), i);
    cache.Add(temp(), 3);
    // ensure we maintian max size regardless of time
    if (i < size)
      EXPECT_EQ(cache.size(), i + 1);
    else
      EXPECT_EQ(cache.size(), size);
  }
}

}  // namespace test

}  // namespace maidsafe
