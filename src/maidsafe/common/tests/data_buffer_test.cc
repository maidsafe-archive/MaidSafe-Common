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

#include <memory>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/data_name_variant.h"

namespace fs = boost::filesystem;
namespace mpl = boost::mpl;

namespace maidsafe {

namespace test {

namespace {

typedef std::pair<uint64_t, uint64_t> MaxMemoryDiskUsage;
const uint64_t OneKB(1024);
const uint64_t kDefaultMaxMemoryUsage(1000);
const uint64_t kDefaultMaxDiskUsage(2000);
const uint32_t kMaxDataTagIndex(mpl::size<typename DataNameVariant::types>::value - 1);

template <typename KeyType>
KeyType GenerateRandomKey() {
  return KeyType(RandomString(crypto::SHA512::DIGESTSIZE));
}

template <>
DataNameVariant GenerateRandomKey<DataNameVariant>() {
  Identity id(RandomString(crypto::SHA512::DIGESTSIZE));
  // Check the value of 'kMaxDataTagIndex' is not too high
  GetDataNameVariant(static_cast<DataTagValue>(kMaxDataTagIndex), id);

  DataTagValue tag_value(static_cast<DataTagValue>(RandomUint32() % kMaxDataTagIndex));
  return GetDataNameVariant(tag_value, id);
}

template <typename KeyType>
KeyType GenerateKeyFromValue(const NonEmptyString& value) {
  return KeyType(crypto::Hash<crypto::SHA512>(value).string());
}

template <>
DataNameVariant GenerateKeyFromValue<DataNameVariant>(const NonEmptyString& value) {
  Identity id(crypto::Hash<crypto::SHA512>(value));
  // Check the value of 'kMaxDataTagIndex' is not too high
  GetDataNameVariant(static_cast<DataTagValue>(kMaxDataTagIndex), id);
  DataTagValue tag_value(static_cast<DataTagValue>(RandomUint32() % kMaxDataTagIndex));
  return GetDataNameVariant(tag_value, id);
}

}  // unnamed namespace

class DataBufferTest : public testing::Test {
 protected:
  typedef DataNameVariant KeyType;
  typedef DataBuffer<KeyType> DataBufferType;
  typedef std::unique_ptr<DataBufferType> DataBufferPtr;
  typedef DataBufferType::PopFunctor PopFunctor;
  typedef std::vector<std::pair<KeyType, NonEmptyString>> KeyValueVector;

  DataBufferTest()
      : max_memory_usage_(kDefaultMaxMemoryUsage),
        max_disk_usage_(kDefaultMaxDiskUsage),
        data_buffer_path_(),
        pop_functor_(),
        data_buffer_(new DataBufferType(max_memory_usage_, max_disk_usage_, pop_functor_)) {}

  void PopFunction(const KeyType& key, const NonEmptyString& value,
                   const std::vector<std::pair<KeyType, NonEmptyString>>& key_value_pairs,
                   size_t& index, std::mutex& mutex, std::condition_variable& cond_var) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      KeyType popped_key(key_value_pairs[index].first);
      NonEmptyString popped_value(key_value_pairs[index].second);
      GetIdentityVisitor get_identity;
      EXPECT_TRUE(boost::apply_visitor(get_identity, popped_key) ==
            boost::apply_visitor(get_identity, key));
      EXPECT_EQ(popped_value, value);
      ++index;
    }
    cond_var.notify_one();
  }

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
    catch (const std::exception& e) {
      LOG(kError) << e.what();
      return false;
    }
    return true;
  }

  KeyValueVector PopulateDataBuffer(size_t num_entries, size_t num_memory_entries,
                                    size_t num_disk_entries, maidsafe::test::TestPath test_path,
                                    const PopFunctor& pop_functor) {
    boost::system::error_code error_code;
    data_buffer_path_ = fs::path(*test_path / "data_buffer");
    KeyValueVector key_value_pairs;
    NonEmptyString value, recovered;
    KeyType key;

    EXPECT_TRUE(fs::create_directories(data_buffer_path_, error_code));
    EXPECT_EQ(0, error_code.value());
    EXPECT_TRUE(fs::exists(data_buffer_path_, error_code));
    EXPECT_EQ(0, error_code.value());

    for (size_t i = 0; i < num_entries; ++i) {
      value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
      key = GenerateKeyFromValue<KeyType>(value);
      key_value_pairs.push_back(std::make_pair(key, value));
    }
    data_buffer_.reset(new DataBufferType(MemoryUsage(num_memory_entries * OneKB),
                                          DiskUsage(num_disk_entries * OneKB), pop_functor,
                                          data_buffer_path_));
    for (auto key_value : key_value_pairs) {
      EXPECT_NO_THROW(data_buffer_->Store(key_value.first, key_value.second));
      EXPECT_NO_THROW(recovered = data_buffer_->Get(key_value.first));
      EXPECT_EQ(key_value.second, recovered);
    }
    return key_value_pairs;
  }

  boost::filesystem::path GetkDiskBuffer(const DataBufferType& data_buffer) {
    return data_buffer.kDiskBuffer_;
  }

  std::string DebugKeyName(const KeyType& key) { return data_buffer_->DebugKeyName(key); }

  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  fs::path data_buffer_path_;
  PopFunctor pop_functor_;
  DataBufferPtr data_buffer_;
};

TEST_F(DataBufferTest, BEH_Constructor) {
  EXPECT_NO_THROW(DataBufferType(MemoryUsage(0), DiskUsage(0), pop_functor_));
  EXPECT_NO_THROW(DataBufferType(MemoryUsage(1), DiskUsage(1), pop_functor_));
  EXPECT_THROW(DataBufferType(MemoryUsage(1), DiskUsage(0), pop_functor_), common_error);
  EXPECT_THROW(DataBufferType(MemoryUsage(2), DiskUsage(1), pop_functor_), common_error);
  EXPECT_THROW(DataBufferType(MemoryUsage(200001), DiskUsage(200000), pop_functor_),
                  common_error);
  EXPECT_NO_THROW(DataBufferType(MemoryUsage(199999), DiskUsage(200000), pop_functor_));

  // Create a path to a file, and check that it can't be used as the disk buffer path.
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  EXPECT_TRUE(!test_path->empty());
  boost::filesystem::path file_path(*test_path / "File");
  EXPECT_TRUE(WriteFile(file_path, " "));
  EXPECT_THROW(DataBufferType(MemoryUsage(199999), DiskUsage(200000), pop_functor_, file_path),
                  common_error);
  EXPECT_THROW(DataBufferType(MemoryUsage(199999), DiskUsage(200000), pop_functor_,
                                 file_path / "Directory"), common_error);

  // Create a path to a directory, and check that it can be used as the disk buffer path.
  boost::filesystem::path directory_path(*test_path / "Directory");
  EXPECT_NO_THROW(DataBufferType(MemoryUsage(1), DiskUsage(1), pop_functor_, directory_path));
  EXPECT_TRUE(fs::exists(directory_path));
}

TEST_F(DataBufferTest, BEH_Destructor) {
  boost::filesystem::path data_buffer_path;
  {
    DataBufferType data_buffer(MemoryUsage(1), DiskUsage(1), pop_functor_);
    data_buffer_path = GetkDiskBuffer(data_buffer);
    EXPECT_TRUE(fs::exists(data_buffer_path));
  }
  EXPECT_TRUE(!fs::exists(data_buffer_path));

  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  EXPECT_TRUE(!test_path->empty());
  data_buffer_path = *test_path / "Directory";
  {
    DataBufferType data_buffer(MemoryUsage(1), DiskUsage(1), pop_functor_, data_buffer_path);
    EXPECT_TRUE(fs::exists(data_buffer_path));
  }
  EXPECT_TRUE(fs::exists(data_buffer_path));
  EXPECT_TRUE(DeleteDirectory(*test_path));
  EXPECT_TRUE(!fs::exists(data_buffer_path));
}

TEST_F(DataBufferTest, BEH_SetMaxDiskMemoryUsage) {
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ - 1)));
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_)));
  ASSERT_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ + 1)),
                    common_error);
  ASSERT_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ - 1)),
                    common_error);
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_)));
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ + 1)));
  ASSERT_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(-1))),
                    common_error);
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(1))));
  ASSERT_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))),
                    common_error);
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(1))));
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(0))));
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))));
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(std::numeric_limits<uint64_t>().max())));
  MemoryUsage memory_usage(std::numeric_limits<uint64_t>().max());
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(memory_usage)));
  ASSERT_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)),
                    common_error);
  ASSERT_NO_THROW(data_buffer_->SetMaxMemoryUsage(MemoryUsage(kDefaultMaxMemoryUsage)));
  ASSERT_NO_THROW(data_buffer_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)));
}

TEST_F(DataBufferTest, BEH_RemoveDiskBuffer) {
  boost::system::error_code error_code;
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  fs::path data_buffer_path(*test_path / "data_buffer");
  const uintmax_t kMemorySize(1), kDiskSize(2);

  data_buffer_.reset(new DataBufferType(MemoryUsage(kMemorySize), DiskUsage(kDiskSize),
                                        pop_functor_, data_buffer_path));
  auto key(GenerateRandomKey<KeyType>());
  NonEmptyString small_value(std::string(kMemorySize, 'a'));
  ASSERT_NO_THROW(data_buffer_->Store(key, small_value));
  ASSERT_NO_THROW(data_buffer_->Delete(key));
  EXPECT_EQ(1, fs::remove_all(data_buffer_path, error_code));
  ASSERT_FALSE(fs::exists(data_buffer_path, error_code));
  // Fits into memory buffer successfully.  Background thread in future should throw, causing other
  // API functions to throw on next execution.
  ASSERT_NO_THROW(data_buffer_->Store(key, small_value));
  Sleep(std::chrono::seconds(1));
  ASSERT_THROW(data_buffer_->Store(key, small_value), common_error);
  ASSERT_THROW(data_buffer_->Get(key), common_error);
  ASSERT_THROW(data_buffer_->Delete(key), common_error);

  data_buffer_.reset(new DataBufferType(MemoryUsage(kMemorySize), DiskUsage(kDiskSize),
                                        pop_functor_, data_buffer_path));
  NonEmptyString large_value(std::string(kDiskSize, 'a'));
  ASSERT_NO_THROW(data_buffer_->Store(key, large_value));
  ASSERT_NO_THROW(data_buffer_->Delete(key));
  EXPECT_EQ(1, fs::remove_all(data_buffer_path, error_code));
  ASSERT_FALSE(fs::exists(data_buffer_path, error_code));
  // Skips memory buffer and goes straight to disk, causing exception.  Background thread in future
  // should finish, causing other API functions to throw on next execution.
  ASSERT_THROW(data_buffer_->Store(key, large_value), common_error);
  ASSERT_THROW(data_buffer_->Get(key), common_error);
  ASSERT_THROW(data_buffer_->Delete(key), common_error);
}

TEST_F(DataBufferTest, BEH_SuccessfulStore) {
  NonEmptyString value1(RandomAlphaNumericString(static_cast<uint32_t>(max_memory_usage_)));
  auto key1(GenerateKeyFromValue<KeyType>(value1));
  NonEmptyString value2(RandomAlphaNumericString(static_cast<uint32_t>(max_memory_usage_)));
  auto key2(GenerateKeyFromValue<KeyType>(value2));
  NonEmptyString recovered;

  ASSERT_NO_THROW(data_buffer_->Store(key1, value1));
  ASSERT_NO_THROW(data_buffer_->Store(key2, value2));
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key1));
  EXPECT_EQ(recovered, value1);
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key2));
  EXPECT_EQ(recovered, value2);
}

TEST_F(DataBufferTest, BEH_UnsuccessfulStore) {
  NonEmptyString value(std::string(static_cast<uint32_t>(max_disk_usage_ + 1), 'a'));
  auto key(GenerateKeyFromValue<KeyType>(value));
  ASSERT_THROW(data_buffer_->Store(key, value), common_error);
}

TEST_F(DataBufferTest, BEH_DeleteOnDiskBufferOverfill) {
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  KeyValueVector key_value_pairs(PopulateDataBuffer(num_entries, num_memory_entries,
                                                    num_disk_entries, test_path, pop_functor_));
  NonEmptyString value, recovered;

  KeyType first_key(key_value_pairs[0].first), second_key(key_value_pairs[1].first);
  value = NonEmptyString(RandomAlphaNumericString(static_cast<uint32_t>(2 * OneKB)));
  auto key(GenerateKeyFromValue<KeyType>(value));
  auto async = std::async(std::launch::async,
                          [this, key, value] {
                            Sleep(std::chrono::milliseconds(100));
                            data_buffer_->Store(key, value);
                          });
  ASSERT_THROW(recovered = data_buffer_->Get(key), common_error);
  Sleep(std::chrono::milliseconds(200));
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_EQ(recovered, value);
  ASSERT_NO_THROW(data_buffer_->Delete(first_key));
  ASSERT_NO_THROW(data_buffer_->Delete(second_key));
  ASSERT_NO_THROW(async.wait());
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_EQ(recovered, value);
  EXPECT_TRUE(DeleteDirectory(data_buffer_path_));
}

TEST_F(DataBufferTest, BEH_PopOnDiskBufferOverfill) {
  size_t index(0);
  std::mutex mutex;
  std::condition_variable condition_variable;
  KeyValueVector key_value_pairs;
  PopFunctor pop_functor([&](const KeyType& key, const NonEmptyString & value) {
    PopFunction(key, value, key_value_pairs, index, mutex, condition_variable);
  });
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  key_value_pairs = PopulateDataBuffer(num_entries, num_memory_entries, num_disk_entries,
                                       test_path, pop_functor);
  EXPECT_EQ(0, index);

  NonEmptyString value, recovered;
  value = NonEmptyString(RandomAlphaNumericString(static_cast<uint32_t>(OneKB)));
  auto key(GenerateKeyFromValue<KeyType>(value));
  // Trigger pop.
  ASSERT_NO_THROW(data_buffer_->Store(key, value));
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_EQ(recovered, value);
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto result(condition_variable.wait_for(lock, std::chrono::seconds(1),
                                            [&]()->bool { return index == 1; }));
    EXPECT_TRUE(result);
  }
  EXPECT_EQ(1, index);

  value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(2 * OneKB))));
  key = GenerateKeyFromValue<KeyType>(value);
  // Trigger pop.
  ASSERT_NO_THROW(data_buffer_->Store(key, value));
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto result(condition_variable.wait_for(lock, std::chrono::seconds(2),
                                            [&]()->bool { return index == 3; }));
    EXPECT_TRUE(result);
  }
  EXPECT_EQ(3, index);
  ASSERT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_EQ(recovered, value);
  EXPECT_TRUE(DeleteDirectory(data_buffer_path_));
}

TEST_F(DataBufferTest, BEH_AsyncDeleteOnDiskBufferOverfill) {
  KeyValueVector old_key_value_pairs, new_key_value_pairs;
  const size_t num_entries(6), num_memory_entries(0), num_disk_entries(6);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  old_key_value_pairs = PopulateDataBuffer(num_entries, num_memory_entries, num_disk_entries,
                                           test_path, pop_functor_);

  NonEmptyString value, recovered;
  KeyType key;
  while (new_key_value_pairs.size() < num_entries) {
    value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
    key = GenerateKeyFromValue<KeyType>(value);
    new_key_value_pairs.push_back(std::make_pair(key, value));
  }

  std::vector<std::future<void>> async_stores;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_stores.push_back(std::async(
        std::launch::async, [this, key, value] { data_buffer_->Store(key, value); }));
  }

  // Check the new Store attempts all block pending some Deletes
  for (auto& async_store : async_stores) {
    auto status(async_store.wait_for(std::chrono::milliseconds(250)));
    EXPECT_EQ(std::future_status::timeout, status);
  }

  std::vector<std::future<NonEmptyString>> async_gets;
  for (auto key_value : new_key_value_pairs) {
    async_gets.push_back(std::async(std::launch::async, [this, key_value] {
      return data_buffer_->Get(key_value.first);
    }));
  }

  // Check Get attempts for the new Store values don't block pending the Store attempts completing
  for (auto& async_get : async_gets) {
    auto status(async_get.wait_for(std::chrono::milliseconds(100)));
    EXPECT_EQ(std::future_status::ready, status);
  }

  // Delete the last new Store attempt before it has completed
  EXPECT_NO_THROW(data_buffer_->Delete(new_key_value_pairs.back().first));
  // Delete the old values to allow the new Store attempts to complete
  for (auto key_value : old_key_value_pairs) {
    EXPECT_NO_THROW(data_buffer_->Delete(key_value.first));
  }

  for (size_t i(0); i != num_entries; ++i) {
    auto status(async_gets[i].wait_for(std::chrono::milliseconds(200)));
    EXPECT_EQ(std::future_status::ready, status);
    EXPECT_NO_THROW(recovered = async_gets[i].get());
    EXPECT_EQ(new_key_value_pairs[i].second, recovered);
  }

  // Check the last store value which was cancelled is now unavailable
  EXPECT_THROW(data_buffer_->Get(new_key_value_pairs.back().first), common_error);
  EXPECT_TRUE(DeleteDirectory(this->data_buffer_path_));
}

TEST_F(DataBufferTest, BEH_AsyncPopOnDiskBufferOverfill) {
  size_t index(0);
  std::mutex mutex;
  std::condition_variable condition_variable;
  KeyValueVector old_key_value_pairs, new_key_value_pairs;
  PopFunctor pop_functor([&](const KeyType& key, const NonEmptyString & value) {
    PopFunction(key, value, old_key_value_pairs, index, mutex, condition_variable);
  });
  const size_t num_entries(6), num_memory_entries(1), num_disk_entries(6);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  old_key_value_pairs = PopulateDataBuffer(num_entries, num_memory_entries, num_disk_entries,
                                           test_path, pop_functor);
  EXPECT_EQ(0, index);

  NonEmptyString value, recovered;
  KeyType key;
  while (new_key_value_pairs.size() < num_entries) {
    value = NonEmptyString(std::string(RandomAlphaNumericString(static_cast<uint32_t>(OneKB))));
    key = GenerateKeyFromValue<KeyType>(value);
    new_key_value_pairs.push_back(std::make_pair(key, value));
  }

  std::vector<std::future<void>> async_operations;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_operations.push_back(std::async(
        std::launch::async, [this, key, value] { this->data_buffer_->Store(key, value); }));
  }
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto result(condition_variable.wait_for(
        lock, std::chrono::seconds(2), [&]()->bool { return index == num_entries; }));
    EXPECT_TRUE(result);
  }
  for (auto key_value : new_key_value_pairs) {
    EXPECT_NO_THROW(recovered = this->data_buffer_->Get(key_value.first));
    EXPECT_EQ(key_value.second, recovered);
  }
  EXPECT_EQ(num_entries, index);
  EXPECT_TRUE(DeleteDirectory(data_buffer_path_));
}

TEST_F(DataBufferTest, BEH_RepeatedlyStoreUsingSameKey) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  data_buffer_path_ = fs::path(*test_path / "data_buffer");
  PopFunctor pop_functor([this](const KeyType& key, const NonEmptyString & value) {
    LOG(kInfo) << "Pop called on " << DebugKeyName(key) << "with value "
               << HexSubstr(value.string());
  });
  data_buffer_.reset(new DataBufferType(MemoryUsage(kDefaultMaxMemoryUsage),
                                        DiskUsage(kDefaultMaxDiskUsage), pop_functor,
                                        data_buffer_path_));
  NonEmptyString value(RandomAlphaNumericString((RandomUint32() % 30) + 1)), recovered, last_value;
  auto key(GenerateKeyFromValue<KeyType>(value));
  auto async = std::async(std::launch::async,
                          [this, key, value] { data_buffer_->Store(key, value); });
  EXPECT_NO_THROW(async.wait());
  EXPECT_TRUE(async.valid());
  EXPECT_NO_THROW(async.get());
  EXPECT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_EQ(recovered, value);

  uint32_t events((RandomUint32() % 100) + 10);
  for (uint32_t i = 0; i != events; ++i) {
    last_value = value;
    while (last_value == value)
      last_value = NonEmptyString(RandomAlphaNumericString((RandomUint32() % 30) + 1));
    auto async =
        std::async(std::launch::async,
                   [this, key, last_value] { data_buffer_->Store(key, last_value); });
    EXPECT_NO_THROW(async.wait());
    EXPECT_TRUE(async.valid());
    EXPECT_NO_THROW(async.get());
  }
  Sleep(std::chrono::milliseconds(100));
  EXPECT_NO_THROW(recovered = data_buffer_->Get(key));
  EXPECT_NE(value, recovered);
  EXPECT_EQ(last_value, recovered);
  data_buffer_.reset();
  EXPECT_TRUE(DeleteDirectory(data_buffer_path_));
}

TEST_F(DataBufferTest, BEH_RandomAsync) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  data_buffer_path_ = fs::path(*test_path / "data_buffer");
  PopFunctor pop_functor([this](const KeyType& key, const NonEmptyString & value) {
    LOG(kInfo) << "Pop called on " << DebugKeyName(key) << "with value "
               << HexSubstr(value.string());
  });
  data_buffer_.reset(new DataBufferType(MemoryUsage(kDefaultMaxMemoryUsage),
                                        DiskUsage(kDefaultMaxDiskUsage), pop_functor,
                                        data_buffer_path_));

  KeyValueVector key_value_pairs;
  uint32_t events((RandomUint32() % 400) + 100);
  std::vector<std::future<void>> future_stores, future_deletes;
  std::vector<std::future<NonEmptyString>> future_gets;

  for (uint32_t i = 0; i != events; ++i) {
    NonEmptyString value(RandomAlphaNumericString((RandomUint32() % 300) + 1));
    auto key(GenerateKeyFromValue<KeyType>(value));
    key_value_pairs.push_back(std::make_pair(key, value));

    uint32_t event(RandomUint32() % 3);
    switch (event) {
      case 0: {
        if (!key_value_pairs.empty()) {
          KeyType event_key(key_value_pairs[RandomUint32() % key_value_pairs.size()].first);
          future_deletes.push_back(
              std::async([this, event_key] { data_buffer_->Delete(event_key); }));
        } else {
          future_deletes.push_back(std::async([this, key] { data_buffer_->Delete(key); }));
        }
        break;
      }
      case 1: {
        // uint32_t index(RandomUint32() % key_value_pairs.size());
        uint32_t index(i);
        KeyType event_key(key_value_pairs[index].first);
        NonEmptyString event_value(key_value_pairs[index].second);
        future_stores.push_back(std::async([this, event_key, event_value] {
            data_buffer_->Store(event_key, event_value);
        }));
        break;
      }
      case 2: {
        if (!key_value_pairs.empty()) {
          KeyType event_key(key_value_pairs[RandomUint32() % key_value_pairs.size()].first);
          future_gets.push_back(
              std::async([this, event_key] { return data_buffer_->Get(event_key); }));
        } else {
          future_gets.push_back(std::async([this, key] { return data_buffer_->Get(key); }));
        }
        break;
      }
    }
  }

  for (auto& future_store : future_stores) {
    EXPECT_NO_THROW(future_store.get());
  }

  for (auto& future_delete : future_deletes) {
    try {
      future_delete.get();
    }
    catch (const common_error&) {}
  }

  for (auto& future_get : future_gets) {
    try {
      NonEmptyString value(future_get.get());
      typedef KeyValueVector::value_type value_type;
      auto it = std::find_if(
          key_value_pairs.begin(), key_value_pairs.end(),
          [this, &value](const value_type & key_value) { return key_value.second == value; });
      EXPECT_NE(key_value_pairs.end(), it);
    }
    catch (const common_error& e) {
      LOG(kInfo) << boost::diagnostic_information(e);
    }
  }
  // Need to destroy data_buffer_ so that test_path can be deleted.
  data_buffer_.reset();
}


namespace {

struct DataBufferUsage {
  DataBufferUsage(int64_t memory_usage, int64_t disk_usage)
      : memory_usage(memory_usage), disk_usage(disk_usage) {}

  int64_t memory_usage;
  int64_t disk_usage;
};

}  // unnamed namespace


class DataBufferValueParameterisedTest : public testing::TestWithParam<DataBufferUsage> {
 protected:
  typedef DataNameVariant KeyType;
  typedef DataBuffer<KeyType> DataBufferType;
  typedef DataBufferType::PopFunctor PopFunctorType;
  typedef std::unique_ptr<DataBufferType> DataBufferPtr;

  DataBufferValueParameterisedTest() : pop_functor_(), data_buffer_() {}

  virtual void SetUp() override {
    auto buf = GetParam();
    memory_usage = buf.memory_usage;
    disk_usage = buf.disk_usage;
    total_usage = disk_usage + memory_usage;
  }

  uint64_t memory_usage;
  uint64_t disk_usage;
  uint64_t total_usage;
  PopFunctorType pop_functor_;
  DataBufferPtr data_buffer_;
};

TEST_P(DataBufferValueParameterisedTest, BEH_Store) {
  data_buffer_.reset(new DataBufferType(MemoryUsage(memory_usage), DiskUsage(disk_usage),
                                        pop_functor_));

  while (total_usage) {
    NonEmptyString value(std::string(RandomAlphaNumericString(
                      static_cast<uint32_t>(memory_usage))));
    KeyType key(GenerateKeyFromValue<KeyType>(value));
    EXPECT_NO_THROW(data_buffer_->Store(key, value));
    NonEmptyString recovered;
    EXPECT_NO_THROW(recovered = data_buffer_->Get(key));
    EXPECT_EQ(value, recovered);
    if (disk_usage) {
      disk_usage -= memory_usage;
      total_usage -= memory_usage;
    } else {
      total_usage -= memory_usage;
    }
  }
}

TEST_P(DataBufferValueParameterisedTest, BEH_Delete) {
  data_buffer_.reset(new DataBufferType(MemoryUsage(memory_usage), DiskUsage(disk_usage),
                                        pop_functor_));

  std::map<KeyType, NonEmptyString> key_value_pairs;
  while (total_usage) {
    NonEmptyString value(
        std::string(RandomAlphaNumericString(static_cast<uint32_t>(memory_usage))));
    KeyType key(GenerateKeyFromValue<KeyType>(value));
  #if defined(__GNUC__) && !defined(__clang__)
    auto ret_val = key_value_pairs.insert(std::make_pair(key, value));
    if (!ret_val.second)
      ret_val.first->second = value;
  #else
    key_value_pairs[key] = value;
  #endif

    EXPECT_NO_THROW(data_buffer_->Store(key, value));
    if (disk_usage) {
      disk_usage -= memory_usage;
      total_usage -= memory_usage;
    } else {
      total_usage -= memory_usage;
    }
  }
  NonEmptyString recovered;
  for (auto key_value : key_value_pairs) {
    KeyType key(key_value.first);
    EXPECT_NO_THROW(recovered = data_buffer_->Get(key));
    EXPECT_EQ(key_value.second, recovered);
    EXPECT_NO_THROW(data_buffer_->Delete(key));
    EXPECT_THROW(recovered = data_buffer_->Get(key), common_error);
  }
}

INSTANTIATE_TEST_CASE_P(BufferValueParam, DataBufferValueParameterisedTest, testing::Values(
    DataBufferUsage(1, 2),
    DataBufferUsage(1, 1024),
    DataBufferUsage(8, 1024),
    DataBufferUsage(1024, 2048),
    DataBufferUsage(1024, 1024),
    DataBufferUsage(16, 16 * 1024),
    DataBufferUsage(32, 32),
    DataBufferUsage(1000, 10000),
    DataBufferUsage(10000, 1000000)));

}  // namespace test

}  // namespace maidsafe
