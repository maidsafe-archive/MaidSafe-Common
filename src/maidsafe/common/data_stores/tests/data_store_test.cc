/*  Copyright 2012 MaidSafe.net limited

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

#include "maidsafe/common/data_stores/data_store.h"
#include "maidsafe/common/data_stores/data_buffer.h"

#include <memory>

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_stores/tests/test_utils.h"

namespace fs = boost::filesystem;
namespace args = std::placeholders;

namespace maidsafe {

namespace data_stores {

namespace test {

const uint64_t kDefaultMaxMemoryUsage(1000);
const uint64_t kDefaultMaxDiskUsage(2000);
const uint64_t OneKB(1024);

class DataStoreTest {
 public:
  typedef DataBuffer<DataNameVariant> StoragePolicy;
  typedef DataStore<StoragePolicy> DataStoreType;
  typedef std::unique_ptr<DataStoreType> DataStorePtr;
  typedef StoragePolicy::KeyType KeyType;
  typedef std::vector<std::pair<KeyType, NonEmptyString>> KeyValueContainer;
  typedef StoragePolicy::PopFunctor PopFunctor;

  struct GenerateKeyValuePair : public boost::static_visitor<NonEmptyString> {
    GenerateKeyValuePair() : size_(OneKB) {}
    explicit GenerateKeyValuePair(uint32_t size) : size_(size) {}

    template <typename T>
    NonEmptyString operator()(T& key) {
      NonEmptyString value = NonEmptyString(RandomAlphaNumericString(size_));
      key.value = Identity(crypto::Hash<crypto::SHA512>(value));
      return value;
    }

    uint32_t size_;
  };

  struct GetIdentity : public boost::static_visitor<Identity> {
    template <typename T>
    Identity operator()(T& key) {
      return key.value;
    }
  };

 protected:
  DataStoreTest()
      : max_memory_usage_(kDefaultMaxMemoryUsage),
        max_disk_usage_(kDefaultMaxDiskUsage),
        data_store_path_(),
        pop_functor_(),
        data_store_(new DataStoreType(max_memory_usage_, max_disk_usage_, pop_functor_)) {}

  void PopFunction(const KeyType& key, const NonEmptyString& value,
                   const std::vector<std::pair<KeyType, NonEmptyString>>& key_value_pairs,
                   size_t& index, std::mutex& mutex,
                   std::condition_variable& condition_variable) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      KeyType compare_key(key_value_pairs[index].first);
      NonEmptyString compare_value(key_value_pairs[index].second);
      GetIdentity get_identity;
      Identity compare_key_id(boost::apply_visitor(get_identity, compare_key)),
          key_id(boost::apply_visitor(get_identity, key));
      REQUIRE(compare_key_id == key_id);
      REQUIRE(compare_value == value);
      ++index;
    }
    condition_variable.notify_one();
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

  KeyValueContainer PopulateDataStore(size_t num_entries, size_t num_memory_entries,
                                      size_t num_disk_entries, maidsafe::test::TestPath test_path,
                                      const PopFunctor& pop_functor) {
    boost::system::error_code error_code;
    data_store_path_ = fs::path(*test_path / "data_store");
    KeyValueContainer key_value_pairs;
    NonEmptyString value, recovered;
    KeyType key;

    REQUIRE(fs::create_directories(data_store_path_, error_code));
    REQUIRE(0 == error_code.value());
    REQUIRE(fs::exists(data_store_path_, error_code));
    REQUIRE(0 == error_code.value());

    AddRandomKeyValuePairs(key_value_pairs, static_cast<uint32_t>(num_entries),
                           static_cast<uint32_t>(OneKB));

    data_store_.reset(new DataStoreType(MemoryUsage(num_memory_entries * OneKB),
                                        DiskUsage(num_disk_entries * OneKB), pop_functor,
                                        data_store_path_));
    for (auto key_value : key_value_pairs) {
      REQUIRE_NOTHROW(data_store_->Store(key_value.first, key_value.second));
      REQUIRE_NOTHROW(recovered = data_store_->Get(key_value.first));
      REQUIRE(key_value.second == recovered);
    }
    return key_value_pairs;
  }

  boost::filesystem::path GetDiskStore(const DataStoreType& data_store) {
    return data_store.kDiskBuffer_;
  }

  NonEmptyString GenerateKeyValueData(KeyType& key, uint32_t size) {
    GenerateKeyValuePair generate_key_value_pair_(size);
    return boost::apply_visitor(generate_key_value_pair_, key);
  }

  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  fs::path data_store_path_;
  PopFunctor pop_functor_;
  DataStorePtr data_store_;
};

TEST_CASE_METHOD(DataStoreTest, "Constructor", "[Behavioural]") {
  REQUIRE_NOTHROW(DataStoreType(MemoryUsage(0), DiskUsage(0), pop_functor_));
  REQUIRE_NOTHROW(DataStoreType(MemoryUsage(1), DiskUsage(1), pop_functor_));
  REQUIRE_THROWS_AS(DataStoreType(MemoryUsage(1), DiskUsage(0), pop_functor_), std::exception);
  REQUIRE_THROWS_AS(DataStoreType(MemoryUsage(2), DiskUsage(1), pop_functor_), std::exception);
  REQUIRE_THROWS_AS(DataStoreType(MemoryUsage(200001), DiskUsage(200000), pop_functor_),
                    std::exception);
  REQUIRE_NOTHROW(DataStoreType(MemoryUsage(199999), DiskUsage(200000), pop_functor_));
  // Create a path to a file, and check that this can't be used as the disk store path.
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataBuffer"));
  REQUIRE_FALSE(test_path->empty());
  boost::filesystem::path file_path(*test_path / "File");
  REQUIRE(WriteFile(file_path, " "));
  REQUIRE_THROWS_AS(DataStoreType(MemoryUsage(199999), DiskUsage(200000), pop_functor_, file_path),
                                  std::exception);
  REQUIRE_THROWS_AS(DataStoreType(MemoryUsage(199999), DiskUsage(200000), pop_functor_,
                                  file_path / "base"), std::exception);

  boost::filesystem::path dir_path(*test_path / "Dir");
  REQUIRE_NOTHROW(DataStoreType(MemoryUsage(1), DiskUsage(1), pop_functor_, dir_path));
  REQUIRE(fs::exists(dir_path));

  boost::filesystem::path data_store_path;
  {
    DataStoreType data_store(MemoryUsage(1), DiskUsage(1), pop_functor_);
    data_store_path = GetDiskStore(data_store);
    REQUIRE(fs::exists(data_store_path));
  }
  REQUIRE_FALSE(fs::exists(data_store_path));
}

TEST_CASE_METHOD(DataStoreTest, "SetMaxDiskMemoryUsage", "[Behavioural]") {
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ - 1)));
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_)));
  REQUIRE_THROWS_AS(data_store_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ + 1)),
                    std::exception);
  REQUIRE_THROWS_AS(data_store_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ - 1)),
                    std::exception);
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(max_disk_usage_)));
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(max_disk_usage_ + 1)));
  REQUIRE_THROWS_AS(data_store_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(-1))),
                    std::exception);
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(1))));
  REQUIRE_THROWS_AS(data_store_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))),
                    std::exception);
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(1))));
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(MemoryUsage(static_cast<uint64_t>(0))));
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(static_cast<uint64_t>(0))));
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(std::numeric_limits<uint64_t>().max())));
  MemoryUsage memory_usage(std::numeric_limits<uint64_t>().max());
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(memory_usage));
  REQUIRE_THROWS_AS(data_store_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)), std::exception);
  REQUIRE_NOTHROW(data_store_->SetMaxMemoryUsage(MemoryUsage(kDefaultMaxMemoryUsage)));
  REQUIRE_NOTHROW(data_store_->SetMaxDiskUsage(DiskUsage(kDefaultMaxDiskUsage)));
}

TEST_CASE_METHOD(DataStoreTest, "RemoveDiskStore", "[Behavioural]") {
  boost::system::error_code error_code;
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  fs::path data_store_path(*test_path / "data_store");
  const uintmax_t kMemorySize(1), kDiskSize(2);
  data_store_.reset(new DataStoreType(MemoryUsage(kMemorySize), DiskUsage(kDiskSize), pop_functor_,
                                      data_store_path));
  KeyType key(GetRandomDataNameType());
  NonEmptyString small_value = GenerateKeyValueData(key, kMemorySize);
  REQUIRE_NOTHROW(data_store_->Store(key, small_value));
  REQUIRE_NOTHROW(data_store_->Delete(key));
  REQUIRE(1 == fs::remove_all(data_store_path, error_code));
  REQUIRE_FALSE(fs::exists(data_store_path, error_code));
  // Fits into memory store successfully.  Background thread in future should throw, causing other
  // API functions to throw on next execution.
  REQUIRE_NOTHROW(data_store_->Store(key, small_value));
  Sleep(std::chrono::seconds(1));
  REQUIRE_THROWS_AS(data_store_->Store(key, small_value), std::exception);
  REQUIRE_THROWS_AS(data_store_->Get(key), std::exception);
  REQUIRE_THROWS_AS(data_store_->Delete(key), std::exception);

  data_store_.reset(new DataStoreType(MemoryUsage(kMemorySize), DiskUsage(kDiskSize),
                                      pop_functor_, data_store_path));
  NonEmptyString large_value = GenerateKeyValueData(key, kDiskSize);
  REQUIRE_NOTHROW(data_store_->Store(key, large_value));
  REQUIRE_NOTHROW(data_store_->Delete(key));
  REQUIRE(1 == fs::remove_all(data_store_path, error_code));
  REQUIRE_FALSE(fs::exists(data_store_path, error_code));
  // Skips memory store and goes straight to disk, causing exception.  Background thread in future
  // should finish, causing other API functions to throw on next execution.
  // - ADAPT TEST FOR MEMORY STORAGE ONLY!!!
  REQUIRE_THROWS_AS(data_store_->Store(key, large_value), std::exception);
  REQUIRE_THROWS_AS(data_store_->Get(key), std::exception);
  REQUIRE_THROWS_AS(data_store_->Delete(key), std::exception);
}

TEST_CASE_METHOD(DataStoreTest, "SuccessfulStore", "[Behavioural]") {
  KeyType key1(GetRandomDataNameType()), key2(GetRandomDataNameType());
  NonEmptyString value1 = GenerateKeyValueData(
                     key1, static_cast<uint32_t>(max_memory_usage_)),
                 value2 = GenerateKeyValueData(
                     key2, static_cast<uint32_t>(max_memory_usage_)),
                 recovered;
  REQUIRE_NOTHROW(data_store_->Store(key1, value1));
  REQUIRE_NOTHROW(data_store_->Store(key2, value2));
  REQUIRE_NOTHROW(recovered = data_store_->Get(key1));
  REQUIRE(recovered == value1);
  REQUIRE_NOTHROW(recovered = data_store_->Get(key2));
  REQUIRE(recovered == value2);
}

TEST_CASE_METHOD(DataStoreTest, "UnsuccessfulStore", "[Behavioural]") {
  KeyType key(GetRandomDataNameType());
  NonEmptyString value = GenerateKeyValueData(key, static_cast<uint32_t>(max_disk_usage_) + 1);
  REQUIRE_THROWS_AS(data_store_->Store(key, value), std::exception);
}

TEST_CASE_METHOD(DataStoreTest, "DeleteOnDiskStoreOverfill", "[Behavioural]") {
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  KeyValueContainer key_value_pairs(PopulateDataStore(
      num_entries, num_memory_entries, num_disk_entries, test_path, pop_functor_));
  KeyType key(GetRandomDataNameType());
  NonEmptyString value = GenerateKeyValueData(key, 2 * OneKB), recovered;
  KeyType first_key(key_value_pairs[0].first), second_key(key_value_pairs[1].first);
  auto async =
      std::async(std::launch::async, [this, key, value] { data_store_->Store(key, value); });
  REQUIRE_THROWS_AS(recovered = data_store_->Get(key), std::exception);
  REQUIRE_NOTHROW(data_store_->Delete(first_key));
  REQUIRE_NOTHROW(data_store_->Delete(second_key));
  REQUIRE_NOTHROW(async.wait());
  REQUIRE_NOTHROW(recovered = data_store_->Get(key));
  REQUIRE(recovered == value);
  REQUIRE(DeleteDirectory(data_store_path_));
}

TEST_CASE_METHOD(DataStoreTest, "PopOnDiskStoreOverfill", "[Behavioural]") {
  size_t index(0);
  std::mutex mutex;
  std::condition_variable condition_variable;
  KeyValueContainer key_value_pairs;
  PopFunctor pop_functor([this, &key_value_pairs, &index, &mutex, &condition_variable](
      const KeyType& key, const NonEmptyString& value) {
    PopFunction(key, value, key_value_pairs, index, mutex, condition_variable);
  });
  const size_t num_entries(4), num_memory_entries(1), num_disk_entries(4);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  key_value_pairs = PopulateDataStore(num_entries, num_memory_entries, num_disk_entries,
                                            test_path, pop_functor);
  REQUIRE(0 == index);

  KeyType key(GetRandomDataNameType());
  NonEmptyString value = GenerateKeyValueData(key, OneKB), recovered;
  // Trigger pop...
  REQUIRE_NOTHROW(data_store_->Store(key, value));
  REQUIRE_NOTHROW(recovered = data_store_->Get(key));
  REQUIRE(recovered == value);
  {
    std::unique_lock<std::mutex> lock(mutex);
    bool result(condition_variable.wait_for(lock, std::chrono::seconds(1),
                                            [&]()->bool { return index == 1; }));
    REQUIRE(result);
  }
  REQUIRE(1 == index);

  value = GenerateKeyValueData(key, 2 * OneKB);
  // Trigger pop.
  REQUIRE_NOTHROW(data_store_->Store(key, value));
  {
    std::unique_lock<std::mutex> lock(mutex);
    bool result(condition_variable.wait_for(lock, std::chrono::seconds(2),
                                            [&]()->bool { return index == 3; }));
    REQUIRE(result);
  }
  REQUIRE(3 == index);
  REQUIRE_NOTHROW(recovered = data_store_->Get(key));
  REQUIRE(recovered == value);

  REQUIRE(DeleteDirectory(data_store_path_));
}

TEST_CASE_METHOD(DataStoreTest, "AsyncDeleteOnDiskStoreOverfill", "[Behavioural]") {
  KeyValueContainer old_key_value_pairs, new_key_value_pairs;
  const size_t num_entries(6), num_memory_entries(0), num_disk_entries(6);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  old_key_value_pairs = PopulateDataStore(num_entries, num_memory_entries, num_disk_entries,
                                          test_path, pop_functor_);
  AddRandomKeyValuePairs(new_key_value_pairs, num_entries, OneKB);

  NonEmptyString value, recovered;
  KeyType key;
  std::vector<std::future<void>> async_stores;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_stores.push_back(std::async(
        std::launch::async, [this, key, value] { data_store_->Store(key, value); }));
  }
  // Check the new Store attempts all block pending some Deletes
  for (auto& async_store : async_stores) {
    auto status(async_store.wait_for(std::chrono::milliseconds(250)));
    REQUIRE(std::future_status::timeout == status);
  }

  std::vector<std::future<NonEmptyString>> async_gets;
  for (auto key_value : new_key_value_pairs) {
    async_gets.push_back(std::async(
        std::launch::async, [this, key_value] { return data_store_->Get(key_value.first); }));
  }
  // Check Get attempts for the new Store values all block pending the Store attempts completing
  for (auto& async_get : async_gets) {
    auto status(async_get.wait_for(std::chrono::milliseconds(100)));
    REQUIRE(std::future_status::timeout == status);
  }
  // Delete the last new Store attempt before it has completed
  REQUIRE_NOTHROW(data_store_->Delete(new_key_value_pairs.back().first));
  // Delete the old values to allow the new Store attempts to complete
  for (auto key_value : old_key_value_pairs) {
    REQUIRE_NOTHROW(data_store_->Delete(key_value.first));
  }

  for (size_t i(0); i != num_entries - 1; ++i) {
    auto status(async_gets[i].wait_for(std::chrono::milliseconds(500)));
    REQUIRE(std::future_status::ready == status);
    recovered = async_gets[i].get();
    REQUIRE(new_key_value_pairs[i].second == recovered);
  }

  auto status(async_gets.back().wait_for(std::chrono::milliseconds(500)));
  REQUIRE(std::future_status::ready == status);
  REQUIRE_THROWS_AS(async_gets.back().get(), std::exception);
}

TEST_CASE_METHOD(DataStoreTest, "AsyncPopOnDiskStoreOverfill", "[Behavioural]") {
  size_t index(0);
  std::mutex mutex;
  std::condition_variable condition_variable;
  KeyValueContainer old_key_value_pairs, new_key_value_pairs;
  PopFunctor pop_functor([this, &old_key_value_pairs, &index, &mutex, &condition_variable](
      const KeyType & key, const NonEmptyString & value) {
    PopFunction(key, value, old_key_value_pairs, index, mutex, condition_variable);
  });
  const size_t num_entries(6), num_memory_entries(1), num_disk_entries(6);
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  old_key_value_pairs = PopulateDataStore(num_entries, num_memory_entries, num_disk_entries,
                                                test_path, pop_functor);
  REQUIRE(0 == index);

  AddRandomKeyValuePairs(new_key_value_pairs, num_entries, OneKB);

  NonEmptyString value, recovered;
  KeyType key;
  std::vector<std::future<void>> async_operations;
  for (auto key_value : new_key_value_pairs) {
    value = key_value.second;
    key = key_value.first;
    async_operations.push_back(std::async(
        std::launch::async, [this, key, value] { data_store_->Store(key, value); }));
  }
  {
    std::unique_lock<std::mutex> lock(mutex);
    bool result(condition_variable.wait_for(lock, std::chrono::seconds(2),
                                            [&]()->bool { return index == num_entries; }));
    REQUIRE(result);
  }
  for (auto key_value : new_key_value_pairs) {
    REQUIRE_NOTHROW(recovered = data_store_->Get(key_value.first));
    REQUIRE(key_value.second == recovered);
  }
  REQUIRE(num_entries == index);
}

TEST_CASE_METHOD(DataStoreTest, "RepeatedlyStoreUsingSameKey", "[Behavioural]") {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  data_store_path_ = fs::path(*test_path / "data_store");
  PopFunctor pop_functor([this](const KeyType & key, const NonEmptyString & value) {
    GetIdentity get_identity;
    Identity key_id(boost::apply_visitor(get_identity, key));
    LOG(kInfo) << "Pop called on " << HexSubstr(key_id.string()) << "with value "
               << HexSubstr(value.string());
  });
  data_store_.reset(new DataStoreType(MemoryUsage(kDefaultMaxMemoryUsage),
                                      DiskUsage(kDefaultMaxDiskUsage), pop_functor,
                                      data_store_path_));
  KeyType key(GetRandomDataNameType());
  NonEmptyString value = GenerateKeyValueData(key, (RandomUint32() % 30) + 1), recovered,
                 last_value;
  REQUIRE_NOTHROW(data_store_->Store(key, value));
  REQUIRE_NOTHROW(recovered = data_store_->Get(key));
  REQUIRE(recovered == value);

  uint32_t events((RandomUint32() % 100) + 1);
  for (uint32_t i = 0; i != events; ++i) {
    last_value = NonEmptyString(RandomAlphaNumericString((RandomUint32() % 30) + 1));
    REQUIRE_NOTHROW(data_store_->Store(key, last_value));
  }
  REQUIRE_NOTHROW(recovered = data_store_->Get(key));
  REQUIRE(value != recovered);
  REQUIRE(last_value == recovered);
  data_store_.reset();
  REQUIRE(DeleteDirectory(data_store_path_));
}

TEST_CASE_METHOD(DataStoreTest, "RandomAsync", "[Behavioural]") {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));
  data_store_path_ = fs::path(*test_path / "data_store");
  PopFunctor pop_functor([this](const KeyType & key, const NonEmptyString & value) {
    GetIdentity get_identity;
    Identity key_id(boost::apply_visitor(get_identity, key));
    LOG(kInfo) << "Pop called on " << HexSubstr(key_id.string()) << "with value "
               << HexSubstr(value.string());
  });
  data_store_.reset(new DataStoreType(MemoryUsage(kDefaultMaxMemoryUsage),
                                      DiskUsage(kDefaultMaxDiskUsage), pop_functor,
                                      data_store_path_));
  KeyValueContainer key_value_pairs;
  uint32_t events(RandomUint32() % 500);
  std::vector<std::future<void>> future_stores, future_deletes;
  std::vector<std::future<NonEmptyString>> future_gets;

  for (uint32_t i = 0; i != events; ++i) {
    KeyType key(GetRandomDataNameType());
    NonEmptyString value = GenerateKeyValueData(key, (RandomUint32() % 300) + 1);
    key_value_pairs.push_back(std::make_pair(key, value));

    uint32_t event(RandomUint32() % 3);
    switch (event) {
      case 0: {
        if (!key_value_pairs.empty()) {
          KeyType event_key(key_value_pairs[RandomUint32() % key_value_pairs.size()].first);
          future_deletes.push_back(
              std::async([this, event_key] { data_store_->Delete(event_key); }));
        } else {
          future_deletes.push_back(std::async([this, key] { data_store_->Delete(key); }));
        }
        break;
      }
      case 1: {
        // uint32_t index(RandomUint32() % key_value_pairs.size());
        uint32_t index(i);
        KeyType event_key(key_value_pairs[index].first);
        NonEmptyString event_value(key_value_pairs[index].second);
        future_stores.push_back(std::async([this, event_key, event_value] {
          data_store_->Store(event_key, event_value);
        }));
        break;
      }
      case 2: {
        if (!key_value_pairs.empty()) {
          KeyType event_key(key_value_pairs[RandomUint32() % key_value_pairs.size()].first);
          future_gets.push_back(
              std::async([this, event_key] { return data_store_->Get(event_key); }));
        } else {
          future_gets.push_back(std::async([this, key] { return data_store_->Get(key); }));
        }
        break;
      }
    }
  }

  for (auto& future_store : future_stores) {
    REQUIRE_NOTHROW(future_store.get());
  }

  for (auto& future_delete : future_deletes) {
    try {
      future_delete.get();
    }
    catch (const std::exception& e) {
      std::string msg(e.what());
      LOG(kError) << msg;
    }
  }

  for (auto& future_get : future_gets) {
    try {
      NonEmptyString value(future_get.get());
      auto it = std::find_if(key_value_pairs.begin(), key_value_pairs.end(),
                             [this, &value](const KeyValueContainer::value_type & key_value_pair) {
        return key_value_pair.second == value;
      });
      REQUIRE(key_value_pairs.end() != it);
    }
    catch (const std::exception& e) {
      std::string msg(e.what());
      LOG(kError) << msg;
    }
  }
  // Need to destroy data_store_ so that test_path will be able to be deleted
  data_store_.reset();
}

namespace {

struct MaxDataStoreUsage { uint64_t memory_usage, disk_usage; };
MaxDataStoreUsage max_data_store_usage[] = {{1, 2}, {1, 1024}, {8, 1024}, {1024, 2048},
                                            {1024, 1024}, {16, 16 * 1024}, {32, 32}, {1000, 10000},
                                            {10000, 1000000}};
}  // unnamed namespace

TEST_CASE_METHOD(DataStoreTest, "Store", "[Behavioural]") {
  MaxDataStoreUsage* resource_usage = Catch::Generators::GENERATE(between(max_data_store_usage,
      &max_data_store_usage[sizeof(max_data_store_usage)/sizeof(MaxDataStoreUsage)-1]));
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));

  fs::path data_store_path(*test_path / "data_store");
  data_store_.reset(new DataStoreType(MemoryUsage(resource_usage->memory_usage),
                                      DiskUsage(resource_usage->disk_usage),
                                      pop_functor_, data_store_path));
  uint64_t disk_usage(resource_usage->disk_usage), memory_usage(resource_usage->memory_usage),
           total_usage(disk_usage + memory_usage);
  while (total_usage != 0) {
    KeyType key(GetRandomDataNameType());
    NonEmptyString value = GenerateKeyValueData(key, static_cast<uint32_t>(memory_usage)),
                   recovered;
    REQUIRE_NOTHROW(data_store_->Store(key, value));
    REQUIRE_NOTHROW(recovered = data_store_->Get(key));
    REQUIRE(value == recovered);
    if (disk_usage != 0) {
      disk_usage -= resource_usage->memory_usage;
      total_usage -= resource_usage->memory_usage;
    } else {
      total_usage -= resource_usage->memory_usage;
    }
  }
  data_store_.reset();
  REQUIRE(DeleteDirectory(data_store_path));
}

TEST_CASE_METHOD(DataStoreTest, "Delete", "[Behavioural]") {
  MaxDataStoreUsage* resource_usage = Catch::Generators::GENERATE(between(max_data_store_usage,
      &max_data_store_usage[sizeof(max_data_store_usage)/sizeof(MaxDataStoreUsage)-1]));
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_DataStore"));

  fs::path data_store_path(*test_path / "data_store");
  data_store_.reset(new DataStoreType(MemoryUsage(resource_usage->memory_usage),
                                      DiskUsage(resource_usage->disk_usage),
                                      pop_functor_, data_store_path));
  uint64_t disk_usage(resource_usage->disk_usage), memory_usage(resource_usage->memory_usage),
           total_usage(disk_usage + memory_usage);
  std::map<KeyType, NonEmptyString> key_value_pairs;
  while (total_usage != 0) {
    KeyType key(GetRandomDataNameType());
    NonEmptyString value = GenerateKeyValueData(key, static_cast<uint32_t>(memory_usage));
#if defined(__GNUC__) && !defined(__clang__)
    auto ret_val = key_value_pairs.insert(std::make_pair(key, value));  // Fails on clang
    if (!ret_val.second)
      ret_val.first->second = value;
#else
    key_value_pairs[key] = value;  // Fails to compile on gcc 4.7
#endif
    REQUIRE_NOTHROW(data_store_->Store(key, value));
    if (disk_usage != 0) {
      disk_usage -= resource_usage->memory_usage;
      total_usage -= resource_usage->memory_usage;
    } else {
      total_usage -= resource_usage->memory_usage;
    }
  }
  NonEmptyString recovered;
  for (auto key_value : key_value_pairs) {
    KeyType key(key_value.first);
    REQUIRE_NOTHROW(recovered = data_store_->Get(key));
    REQUIRE(key_value.second == recovered);
    REQUIRE_NOTHROW(data_store_->Delete(key));
    REQUIRE_THROWS_AS(recovered = data_store_->Get(key), std::exception);
  }
  data_store_.reset();
  REQUIRE(DeleteDirectory(data_store_path));
}

}  // namespace test

}  // namespace data_stores

}  // namespace maidsafe
