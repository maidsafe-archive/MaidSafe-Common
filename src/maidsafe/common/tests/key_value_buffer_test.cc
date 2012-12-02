/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/common/key_value_buffer.h"

#include <memory>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace test {

typedef std::pair<uint64_t, uint64_t> MaxMemoryDiskUsage;
const uint64_t OneKB(1024);
const uint64_t kDefaultMaxMemoryUsage(1000);
const uint64_t kDefaultMaxDiskUsage(2000);

class KeyValueBufferTest : public testing::Test {
 public:
  void PopFunction(const Identity& key_popped, const NonEmptyString& value_popped,
                   std::vector<std::pair<Identity, NonEmptyString>> *key_value_pairs,
                   size_t *cur_popped_index, boost::mutex *pop_mutex,
                   boost::condition_variable *pop_cond_var) {
    Identity to_be_popped_key((*key_value_pairs)[*cur_popped_index].first);
    NonEmptyString to_be_popped_value((*key_value_pairs)[*cur_popped_index].second);
    EXPECT_EQ(to_be_popped_key, key_popped);
    EXPECT_EQ(to_be_popped_value, value_popped);

    boost::mutex::scoped_lock lock(*pop_mutex);
    (*cur_popped_index)++;
    pop_cond_var->notify_one();
  }

 protected:
  KeyValueBufferTest()
      : max_memory_usage_(kDefaultMaxMemoryUsage),
        max_disk_usage_(kDefaultMaxDiskUsage),
        kv_buffer_path_(),
        pop_functor_(),
        key_value_buffer_(new KeyValueBuffer(max_memory_usage_, max_disk_usage_, pop_functor_)) {}

  bool DeleteDirectory(const fs::path& directory) {
    boost::system::error_code error_code;
    fs::directory_iterator end;
    try {
    fs::directory_iterator it(directory);
    for (; it != end; ++it)
      fs::remove_all((*it).path(), error_code);
      if (error_code)
        return false;
    }
    catch(const std::exception &e) {
      LOG(kError) << e.what();
      return false;
    }
    return true;
  }

  std::vector<std::pair<Identity, NonEmptyString>> PopulateKVB(
      size_t num_entries, size_t num_memory_entries, size_t num_disk_entries,
      TestPath test_path, const KeyValueBuffer::PopFunctor &pop_functor) {
    boost::system::error_code error_code;
    kv_buffer_path_ = fs::path(*test_path / "kv_buffer");
    std::vector<std::pair<Identity, NonEmptyString>> key_value_pairs;
    NonEmptyString value, recovered;
    Identity key;

    EXPECT_TRUE(fs::create_directories(kv_buffer_path_, error_code)) << kv_buffer_path_ << ": "
                                                                    << error_code.message();
    EXPECT_EQ(0, error_code.value()) << kv_buffer_path_ << ": " << error_code.message();
    EXPECT_TRUE(fs::exists(kv_buffer_path_, error_code)) << kv_buffer_path_ << ": "
                                                        << error_code.message();
    EXPECT_EQ(0, error_code.value());

    for (size_t i = 0; i < num_entries; ++i) {
      value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
      key = Identity(crypto::Hash<crypto::SHA512>(value));
      key_value_pairs.push_back(std::make_pair(key, value));
    }
    key_value_buffer_.reset(new KeyValueBuffer(MemoryUsage(num_memory_entries * OneKB),
                                               DiskUsage(num_disk_entries * OneKB),
                                               pop_functor, kv_buffer_path_));
    for (auto key_value : key_value_pairs) {
      EXPECT_NO_THROW(key_value_buffer_->Store(key_value.first, key_value.second));
      EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key_value.first));
      EXPECT_EQ(key_value.second, recovered);
    }
    return key_value_pairs;
  }

  boost::filesystem::path GetkDiskBuffer(const KeyValueBuffer &kvb) {
    return kvb.kDiskBuffer_;
  }

  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  fs::path kv_buffer_path_;
  KeyValueBuffer::PopFunctor pop_functor_;
  std::unique_ptr<KeyValueBuffer> key_value_buffer_;
};

TEST_F(KeyValueBufferTest, BEH_Constructor) {
  EXPECT_NO_THROW(KeyValueBuffer(MemoryUsage(0), DiskUsage(0), pop_functor_));
  EXPECT_NO_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(1), pop_functor_));
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(0), pop_functor_), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(2), DiskUsage(1), pop_functor_), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(200001), DiskUsage(200000), pop_functor_),
               std::exception);
  EXPECT_NO_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000), pop_functor_));
  // Create a path to a file, and check that this can't be used as the disk buffer path.
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  ASSERT_FALSE(test_path->empty());
  boost::filesystem::path file_path(*test_path / "File");
  ASSERT_TRUE(WriteFile(file_path, " "));
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000), pop_functor_, file_path),
               std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000), pop_functor_,
               file_path / "base"), std::exception);

  file_path = boost::filesystem::path(*test_path / "File1");
  EXPECT_NO_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(1), pop_functor_, file_path));
  ASSERT_FALSE(test_path->empty());

  boost::filesystem::path key_path;
  {
    KeyValueBuffer kvb(MemoryUsage(1), DiskUsage(1), pop_functor_);
    key_path = GetkDiskBuffer(kvb);
    ASSERT_FALSE(key_path.empty());
    key_path = key_path / "FILE";
    ASSERT_TRUE(WriteFile(key_path, " "));
  }
  ASSERT_FALSE(WriteFile(key_path, " "));
}

TEST_F(KeyValueBufferTest, BEH_SetMaxDiskMemoryUsage) {
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ - 1)));
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_)));
  EXPECT_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ + 1)),
               std::exception);
  EXPECT_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ - 1)),
               std::exception);
  EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_)));
  EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ + 1)));
  EXPECT_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(-1))),
               std::exception);
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(1))));
  EXPECT_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))),
               std::exception);
  EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(1))));
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(0))));
  EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))));
  EXPECT_NO_THROW(
     key_value_buffer_->SetMaxDiskUsage(DiskUsage(std::numeric_limits<uint64_t>().max())));
  EXPECT_NO_THROW(
     key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(std::numeric_limits<uint64_t>().max())));
  EXPECT_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)),
               std::exception);
  EXPECT_NO_THROW(
     key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(kDefaultMaxMemoryUsage)));
  EXPECT_NO_THROW(
     key_value_buffer_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)));
}

TEST_F(KeyValueBufferTest, BEH_RemoveDiskBuffer) {
  boost::system::error_code error_code;
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  fs::path kv_buffer_path(*test_path / "kv_buffer");
  const uintmax_t kMemorySize(1), kDiskSize(2);
  key_value_buffer_.reset(new KeyValueBuffer(MemoryUsage(kMemorySize), DiskUsage(kDiskSize),
                                             pop_functor_, kv_buffer_path));
  Identity key(RandomAlphaNumericString(crypto::SHA512::DIGESTSIZE));
  NonEmptyString small_value(std::string(kMemorySize, 'a'));
  EXPECT_NO_THROW(key_value_buffer_->Store(key, small_value));
  EXPECT_NO_THROW(key_value_buffer_->Delete(key));
  ASSERT_EQ(1, fs::remove_all(kv_buffer_path, error_code));
  ASSERT_FALSE(fs::exists(kv_buffer_path, error_code));
  // Fits into memory buffer successfully.  Background thread in future should throw, causing other
  // API functions to throw on next execution.
  EXPECT_NO_THROW(key_value_buffer_->Store(key, small_value));
  Sleep(boost::posix_time::seconds(1));
  EXPECT_THROW(key_value_buffer_->Store(key, small_value), std::exception);
  EXPECT_THROW(key_value_buffer_->Get(key), std::exception);
  EXPECT_THROW(key_value_buffer_->Delete(key), std::exception);

  key_value_buffer_.reset(new KeyValueBuffer(MemoryUsage(kMemorySize), DiskUsage(kDiskSize),
                                             pop_functor_, kv_buffer_path));
  NonEmptyString large_value(std::string(kDiskSize, 'a'));
  EXPECT_NO_THROW(key_value_buffer_->Store(key, large_value));
  EXPECT_NO_THROW(key_value_buffer_->Delete(key));
  ASSERT_EQ(1, fs::remove_all(kv_buffer_path, error_code));
  ASSERT_FALSE(fs::exists(kv_buffer_path, error_code));
  // Skips memory buffer and goes straight to disk, causing exception.  Background thread in future
  // should finish, causing other API functions to throw on next execution.
  EXPECT_THROW(key_value_buffer_->Store(key, large_value), std::exception);
  EXPECT_THROW(key_value_buffer_->Get(key), std::exception);
  EXPECT_THROW(key_value_buffer_->Delete(key), std::exception);
}

TEST_F(KeyValueBufferTest, BEH_SuccessfulStore) {
  NonEmptyString value1(std::string(RandomAlphaNumericString(
                    static_cast<uint32_t>(max_memory_usage_))));
  Identity key1(crypto::Hash<crypto::SHA512>(value1));
  NonEmptyString value2(std::string(RandomAlphaNumericString(
                    static_cast<uint32_t>(max_memory_usage_))));
  Identity key2(crypto::Hash<crypto::SHA512>(value2));
  EXPECT_NO_THROW(key_value_buffer_->Store(key1, value1));
  EXPECT_NO_THROW(key_value_buffer_->Store(key2, value2));
  NonEmptyString recovered = key_value_buffer_->Get(key1);
  EXPECT_EQ(recovered, value1);
  recovered = key_value_buffer_->Get(key2);
  EXPECT_EQ(recovered, value2);
}

TEST_F(KeyValueBufferTest, BEH_UnsuccessfulStore) {
  NonEmptyString value(std::string(static_cast<uint32_t>(max_disk_usage_ + 1), 'a'));
  Identity key(crypto::Hash<crypto::SHA512>(value));
  EXPECT_THROW(key_value_buffer_->Store(key, value), std::exception);
}

TEST_F(KeyValueBufferTest, BEH_DeleteOnDiskBufferOverfill) {
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  std::vector<std::pair<Identity, NonEmptyString>> key_value_pairs(
      PopulateKVB(num_entries, num_memory_entries, num_disk_entries, test_path, pop_functor_));
  NonEmptyString value, recovered;
  Identity key;

  Identity first_key(key_value_pairs[0].first), second_key(key_value_pairs[1].first);
  value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(2 * OneKB))));
  key = Identity(crypto::Hash<crypto::SHA512>(value));
  auto async = std::async(std::launch::async, [this, key, value] {
                                                  key_value_buffer_->Store(key, value);
                                              });
  EXPECT_THROW(recovered = key_value_buffer_->Get(key), std::exception);
  EXPECT_NO_THROW(key_value_buffer_->Delete(first_key));
  EXPECT_NO_THROW(key_value_buffer_->Delete(second_key));
  async.wait();
  EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
  EXPECT_EQ(recovered, value);

  EXPECT_TRUE(DeleteDirectory(kv_buffer_path_));
}

TEST_F(KeyValueBufferTest, BEH_PopOnDiskBufferOverfill) {
  size_t cur_idx(0);
  boost::mutex pop_mutex;
  boost::condition_variable pop_cond_var;
  std::vector<std::pair<Identity, NonEmptyString>> key_value_pairs;
  KeyValueBuffer::PopFunctor pop_functor(boost::bind(&KeyValueBufferTest::PopFunction,
                                                     this, _1, _2, &key_value_pairs, &cur_idx,
                                                     &pop_mutex, &pop_cond_var));
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  key_value_pairs = PopulateKVB(num_entries, num_memory_entries, num_disk_entries,
                                test_path, pop_functor);
  EXPECT_EQ(0, cur_idx);

  NonEmptyString value, recovered;
  Identity key;
  value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
  key = Identity(crypto::Hash<crypto::SHA512>(value));
  // Trigger pop...
  EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
  EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
  EXPECT_EQ(recovered, value);
  {
    boost::mutex::scoped_lock pop_lock(pop_mutex);
    EXPECT_TRUE(pop_cond_var.timed_wait(pop_lock, boost::posix_time::seconds(1), [&]()->bool {
        if (cur_idx == 1)
          return true;
        else
          return false;
    }));
  }
  EXPECT_EQ(1, cur_idx);

  value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(2 * OneKB))));
  key = Identity(crypto::Hash<crypto::SHA512>(value));
  // Trigger pop...
  EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
  {
    boost::mutex::scoped_lock pop_lock(pop_mutex);
    EXPECT_TRUE(pop_cond_var.timed_wait(pop_lock, boost::posix_time::seconds(2), [&]()->bool {
        if (cur_idx == 3)
          return true;
        else
          return false;
    }));
  }
  EXPECT_EQ(3, cur_idx);
  EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
  EXPECT_EQ(recovered, value);

  EXPECT_TRUE(DeleteDirectory(kv_buffer_path_));
}

TEST_F(KeyValueBufferTest, BEH_AsyncPopOnDiskBufferOverfill) {
  size_t cur_idx(0);
  boost::mutex pop_mutex;
  boost::condition_variable pop_cond_var;
  std::vector<std::pair<Identity, NonEmptyString>> old_key_value_pairs,
                                                   new_key_value_pairs;
  KeyValueBuffer::PopFunctor pop_functor(boost::bind(&KeyValueBufferTest::PopFunction,
                                                     this, _1, _2, &old_key_value_pairs, &cur_idx,
                                                     &pop_mutex, &pop_cond_var));
  const size_t num_entries(6), num_memory_entries(1), num_disk_entries(6);
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  old_key_value_pairs = PopulateKVB(num_entries, num_memory_entries, num_disk_entries,
                                    test_path, pop_functor);
  EXPECT_EQ(0, cur_idx);

  NonEmptyString value, recovered;
  Identity key;
  while (new_key_value_pairs.size() < num_entries) {
    value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
    key = Identity(crypto::Hash<crypto::SHA512>(value));
    new_key_value_pairs.push_back(std::make_pair(key, value));
  }

  std::vector<std::future<void> > async_operations;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_operations.push_back(std::async(std::launch::async, [this, key, value] {
                                                  key_value_buffer_->Store(key, value);
                                              }));
  }
  {
    boost::mutex::scoped_lock pop_lock(pop_mutex);
    EXPECT_TRUE(pop_cond_var.timed_wait(pop_lock, boost::posix_time::seconds(2), [&]()->bool {
        if (cur_idx == num_entries)
          return true;
        else
          return false;
    }));
  }
  for (auto key_value : new_key_value_pairs) {
    EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key_value.first));
    EXPECT_EQ(key_value.second, recovered);
  }
  EXPECT_EQ(num_entries, cur_idx);

  EXPECT_TRUE(DeleteDirectory(kv_buffer_path_));
}

TEST_F(KeyValueBufferTest, BEH_AsyncNonPopOnDiskBufferOverfill) {
  std::vector<std::pair<Identity, NonEmptyString>> old_key_value_pairs,
                                                   new_key_value_pairs;
  const size_t num_entries(6), num_memory_entries(0), num_disk_entries(6);
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  old_key_value_pairs = PopulateKVB(num_entries, num_memory_entries, num_disk_entries,
                                    test_path, pop_functor_);

  NonEmptyString value, recovered;
  Identity key;
  while (new_key_value_pairs.size() < num_entries) {
    value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
    key = Identity(crypto::Hash<crypto::SHA512>(value));
    new_key_value_pairs.push_back(std::make_pair(key, value));
  }

  std::vector<std::future<void> > async_operations;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_operations.push_back(std::async(std::launch::async, [this, key, value] {
                                                  key_value_buffer_->Store(key, value);
                                              }));
  }
  Sleep(boost::posix_time::seconds(1));
  for (auto key_value : new_key_value_pairs) {
    EXPECT_THROW(recovered = key_value_buffer_->Get(key_value.first), std::exception);
    EXPECT_NE(key_value.second, recovered);
  }
  for (auto key_value : old_key_value_pairs) {
    EXPECT_NO_THROW(key_value_buffer_->Delete(key_value.first));
  }
  Sleep(boost::posix_time::seconds(1));
  for (auto key_value : new_key_value_pairs) {
    EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key_value.first));
    EXPECT_EQ(key_value.second, recovered);
  }

  EXPECT_TRUE(DeleteDirectory(kv_buffer_path_));
}

class KeyValueBufferTestDiskMemoryUsage : public testing::TestWithParam<MaxMemoryDiskUsage> {
 protected:
  KeyValueBufferTestDiskMemoryUsage()
      : max_memory_usage_(GetParam().first),
        max_disk_usage_(GetParam().second),
        pop_functor_(),
        key_value_buffer_(new KeyValueBuffer(max_memory_usage_, max_disk_usage_, pop_functor_)) {}
  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  KeyValueBuffer::PopFunctor pop_functor_;
  std::unique_ptr<KeyValueBuffer> key_value_buffer_;
};

TEST_P(KeyValueBufferTestDiskMemoryUsage, BEH_Store) {
  uint64_t disk_usage(max_disk_usage_), memory_usage(max_memory_usage_),
           total_usage(disk_usage + memory_usage);
  while (total_usage != 0) {
    NonEmptyString value(std::string(RandomAlphaNumericString(
                      static_cast<uint32_t>(max_memory_usage_))));
    Identity key(crypto::Hash<crypto::SHA512>(value));
    EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
    NonEmptyString recovered;
    EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
    EXPECT_EQ(value, recovered);
    if (disk_usage != 0) {
      disk_usage -= max_memory_usage_;
      total_usage -= max_memory_usage_;
    } else {
      total_usage -= max_memory_usage_;
    }
  }
}

TEST_P(KeyValueBufferTestDiskMemoryUsage, BEH_Delete) {
  uint64_t disk_usage(max_disk_usage_), memory_usage(max_memory_usage_),
           total_usage(disk_usage + memory_usage);
  std::map<Identity, NonEmptyString> key_value_pairs;
  while (total_usage != 0) {
    NonEmptyString value(std::string(RandomAlphaNumericString(
                      static_cast<uint32_t>(max_memory_usage_))));
    Identity key(crypto::Hash<crypto::SHA512>(value));
    key_value_pairs[key] = value;
    EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
    if (disk_usage != 0) {
      disk_usage -= max_memory_usage_;
      total_usage -= max_memory_usage_;
    } else {
      total_usage -= max_memory_usage_;
    }
  }
  NonEmptyString recovered;
  for (auto key_value : key_value_pairs) {
    Identity key(key_value.first);
    EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
    EXPECT_EQ(key_value.second, recovered);
    EXPECT_NO_THROW(key_value_buffer_->Delete(key));
    EXPECT_THROW(recovered = key_value_buffer_->Get(key), std::exception);
  }
}

INSTANTIATE_TEST_CASE_P(TestKeyValueBuffer,
                        KeyValueBufferTestDiskMemoryUsage,
                        testing::Values(std::make_pair(1, 2),
                                        std::make_pair(1, 1024),
                                        std::make_pair(8, 1024),
                                        std::make_pair(1024, 2048),
                                        std::make_pair(1024, 1024),
                                        std::make_pair(16, 16 * 1024),
                                        std::make_pair(32, 32),
                                        std::make_pair(1000, 10000),
                                        std::make_pair(10000, 1000000)));

}  // namespace test

}  // namespace maidsafe
