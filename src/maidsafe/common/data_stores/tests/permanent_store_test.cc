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

#include "maidsafe/common/data_stores/permanent_store.h"

#include <memory>

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

#include "boost/mpl/size.hpp"

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
namespace args = std::placeholders;

namespace maidsafe {

namespace data_stores {

namespace test {

const uint64_t kDefaultMaxDiskUsage(4 * 1024);
const uint64_t OneKB(1024);

class PermanentStoreTest {
 public:
  typedef PermanentStore::KeyType KeyType;
  typedef std::vector<std::pair<KeyType, NonEmptyString>> KeyValueContainer;

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
      return key.data;
    }
  };

 protected:
  PermanentStoreTest()
      : test_path(maidsafe::test::CreateTestPath("MaidSafe_Test_PermanentStore")),
        permanent_store_path_(*test_path / "permanent_store"),
        max_disk_usage_(kDefaultMaxDiskUsage),
        permanent_store_(new PermanentStore(permanent_store_path_, max_disk_usage_)) {}

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

  KeyValueContainer PopulatePermanentStore(uint32_t num_entries, uint32_t disk_entries,
                                           const fs::path& test_path) {
    boost::system::error_code error_code;
    permanent_store_path_ = test_path;
    KeyValueContainer key_value_pairs;
    NonEmptyString value, recovered;
    KeyType key;

    if (!fs::exists(test_path))
      REQUIRE(fs::create_directories(permanent_store_path_, error_code));
    REQUIRE(0 == error_code.value());
    REQUIRE(fs::exists(permanent_store_path_, error_code));
    REQUIRE(0 == error_code.value());

    AddRandomKeyValuePairs(key_value_pairs, num_entries, OneKB);

    DiskUsage disk_usage(disk_entries * OneKB);
    permanent_store_.reset(new PermanentStore(permanent_store_path_, disk_usage));
    for (auto key_value : key_value_pairs) {
      REQUIRE_NOTHROW(permanent_store_->Put(key_value.first, key_value.second));
      REQUIRE_NOTHROW(recovered = permanent_store_->Get(key_value.first));
      REQUIRE(key_value.second == recovered);
    }
    return key_value_pairs;
  }

  void AddRandomKeyValuePairs(KeyValueContainer& container, uint32_t number, uint32_t size) {
    // Currently there is 13 types defined, but do the calculation anyway...
    uint32_t number_of_types = boost::mpl::size<typename KeyType::types>::value, type_number;
    NonEmptyString value;
    for (uint32_t i = 0; i != number; ++i) {
      type_number = RandomUint32() % number_of_types;
      value = NonEmptyString(RandomAlphaNumericString(size));
      switch (type_number) {
        case 0: {
          passport::PublicAnmid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 1: {
          passport::PublicAnsmid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 2: {
          passport::PublicAntmid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 3: {
          passport::PublicAnmaid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 4: {
          passport::PublicMaid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 5: {
          passport::PublicPmid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 6: {
          passport::Mid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 7: {
          passport::Smid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 8: {
          passport::Tmid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 9: {
          passport::PublicAnmpid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 10: {
          passport::PublicMpid::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 11: {
          ImmutableData::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
        case 12: {
          MutableData::Name key(Identity(crypto::Hash<crypto::SHA512>(value)));
          container.push_back(std::make_pair(key, value));
          break;
        }
      }
    }
  }

  KeyType GetRandomKey() {
    // Currently 13 types are defined, but...
    uint32_t number_of_types = boost::mpl::size<typename KeyType::types>::value, type_number;
    type_number = RandomUint32() % number_of_types;
    switch (type_number) {
      case 0:
        return passport::PublicAnmid::Name();
      case 1:
        return passport::PublicAnsmid::Name();
      case 2:
        return passport::PublicAntmid::Name();
      case 3:
        return passport::PublicAnmaid::Name();
      case 4:
        return passport::PublicMaid::Name();
      case 5:
        return passport::PublicPmid::Name();
      case 6:
        return passport::Mid::Name();
      case 7:
        return passport::Smid::Name();
      case 8:
        return passport::Tmid::Name();
      case 9:
        return passport::PublicAnmpid::Name();
      case 10:
        return passport::PublicMpid::Name();
      case 11:
        return ImmutableData::Name();
      case 12:
        return MutableData::Name();
        // default:
        // Throw something!
        //  ;
    }
    return KeyType();
  }

  NonEmptyString GenerateKeyValueData(KeyType& key, uint32_t size) {
    GenerateKeyValuePair generate_key_value_pair_(size);
    return boost::apply_visitor(generate_key_value_pair_, key);
  }

  void PrintResult(const pt::ptime& start_time, const pt::ptime& stop_time) {
    uint64_t duration = (stop_time - start_time).total_microseconds();
    if (duration == 0)
      duration = 1;
    std::cout << "Operation completed in " << duration / 1000000.0 << " secs." << std::endl;
  }

  maidsafe::test::TestPath test_path;
  fs::path permanent_store_path_;
  DiskUsage max_disk_usage_;
  std::unique_ptr<PermanentStore> permanent_store_;
};

TEST_CASE_METHOD(PermanentStoreTest, "Constructor", "[Behavioural]") {
  REQUIRE_NOTHROW(PermanentStore(permanent_store_path_, DiskUsage(0)));
  REQUIRE_NOTHROW(PermanentStore(permanent_store_path_, DiskUsage(1)));
  REQUIRE_NOTHROW(PermanentStore(permanent_store_path_, DiskUsage(200000)));
  // Create a path to a file, and check that this can't be used as the disk store path.
  maidsafe::test::TestPath test_path(
      maidsafe::test::CreateTestPath("MaidSafe_Test_PermanentStore"));
  REQUIRE_FALSE(test_path->empty());
  boost::filesystem::path file_path(*test_path / "File");
  REQUIRE(WriteFile(file_path, " "));
  REQUIRE_THROWS_AS(PermanentStore(file_path, DiskUsage(200000)), std::exception);
  REQUIRE_THROWS_AS(PermanentStore(file_path / "base", DiskUsage(200000)), std::exception);
  boost::filesystem::path directory_path(*test_path / "Directory");
  REQUIRE_NOTHROW(PermanentStore(directory_path, DiskUsage(1)));
  REQUIRE(fs::exists(directory_path));
}

TEST_CASE_METHOD(PermanentStoreTest, "RemoveDiskStore", "[Behavioural]") {
  boost::system::error_code error_code;
  maidsafe::test::TestPath test_path(
      maidsafe::test::CreateTestPath("MaidSafe_Test_PermanentStore"));
  fs::path permanent_store_path(*test_path / "new_permanent_store");
  const uintmax_t kSize(1), kDiskSize(2);
  permanent_store_.reset(new PermanentStore(permanent_store_path, DiskUsage(kDiskSize)));
  KeyType key(GetRandomKey());
  NonEmptyString small_value = GenerateKeyValueData(key, kSize);
  REQUIRE_NOTHROW(permanent_store_->Put(key, small_value));
  REQUIRE_NOTHROW(permanent_store_->Delete(key));
  REQUIRE(6 == fs::remove_all(permanent_store_path, error_code));
  REQUIRE_FALSE(fs::exists(permanent_store_path, error_code));
  KeyType key1(GetRandomKey());
  NonEmptyString large_value = GenerateKeyValueData(key1, kDiskSize);
  REQUIRE_THROWS_AS(permanent_store_->Put(key, small_value), std::exception);
  REQUIRE_THROWS_AS(permanent_store_->Get(key), std::exception);
  REQUIRE_THROWS_AS(permanent_store_->Delete(key), std::exception);
  permanent_store_.reset(new PermanentStore(permanent_store_path, DiskUsage(kDiskSize)));
  REQUIRE_NOTHROW(permanent_store_->Put(key1, large_value));
  REQUIRE_NOTHROW(permanent_store_->Delete(key1));
  REQUIRE(6 != fs::remove_all(permanent_store_path, error_code));
  REQUIRE_FALSE(fs::exists(permanent_store_path, error_code));
  REQUIRE_THROWS_AS(permanent_store_->Put(key, small_value), std::exception);
  REQUIRE_THROWS_AS(permanent_store_->Get(key), std::exception);
  REQUIRE_THROWS_AS(permanent_store_->Delete(key), std::exception);
}

TEST_CASE_METHOD(PermanentStoreTest, "SuccessfulStore", "[Behavioural]") {
  KeyType key1(GetRandomKey()), key2(GetRandomKey());
  NonEmptyString value1 = GenerateKeyValueData(key1, static_cast<uint32_t>(2 * OneKB)),
                 value2 = GenerateKeyValueData(key2, static_cast<uint32_t>(2 * OneKB)), recovered;
  REQUIRE_NOTHROW(permanent_store_->Put(key1, value1));
  REQUIRE_NOTHROW(permanent_store_->Put(key2, value2));
  REQUIRE_NOTHROW(recovered = permanent_store_->Get(key1));
  REQUIRE(recovered == value1);
  REQUIRE_NOTHROW(recovered = permanent_store_->Get(key2));
  REQUIRE(recovered == value2);
}

TEST_CASE_METHOD(PermanentStoreTest, "UnsuccessfulStore", "[Behavioural]") {
  KeyType key(GetRandomKey());
  NonEmptyString value = GenerateKeyValueData(key, static_cast<uint32_t>(kDefaultMaxDiskUsage) + 1);
  REQUIRE_THROWS_AS(permanent_store_->Put(key, value), std::exception);
}

TEST_CASE_METHOD(PermanentStoreTest, "DeleteOnDiskStoreOverfill", "[Behavioural]") {
  const size_t num_entries(4), num_disk_entries(4);
  KeyValueContainer key_value_pairs(
      PopulatePermanentStore(num_entries, num_disk_entries, permanent_store_path_));
  KeyType key(GetRandomKey());
  NonEmptyString value = GenerateKeyValueData(key, 2 * OneKB), recovered;
  KeyType first_key(key_value_pairs[0].first), second_key(key_value_pairs[1].first);
  REQUIRE_THROWS_AS(permanent_store_->Put(key, value), std::exception);
  REQUIRE_THROWS_AS(recovered = permanent_store_->Get(key), std::exception);
  REQUIRE_NOTHROW(permanent_store_->Delete(first_key));
  REQUIRE_NOTHROW(permanent_store_->Delete(second_key));
  REQUIRE_NOTHROW(permanent_store_->Put(key, value));
  REQUIRE_NOTHROW(recovered = permanent_store_->Get(key));
  REQUIRE(recovered == value);
}

TEST_CASE_METHOD(PermanentStoreTest, "RepeatedlyStoreUsingSameKey", "[Behavioural]") {
  KeyType key(GetRandomKey());
  NonEmptyString value = GenerateKeyValueData(key, (RandomUint32() % 30) + 1), recovered,
                 last_value;
  REQUIRE_NOTHROW(permanent_store_->Put(key, value));
  REQUIRE_NOTHROW(recovered = permanent_store_->Get(key));
  REQUIRE(recovered == value);

  uint32_t events(RandomUint32() % 100);
  for (uint32_t i = 0; i != events; ++i) {
    last_value = NonEmptyString(RandomAlphaNumericString((RandomUint32() % 30) + 1));
    REQUIRE_NOTHROW(permanent_store_->Put(key, last_value));
  }
  REQUIRE_NOTHROW(recovered = permanent_store_->Get(key));
  REQUIRE(value != recovered);
  REQUIRE(last_value == recovered);
  REQUIRE(last_value.string().size() == permanent_store_->GetCurrentDiskUsage().data);
}

TEST_CASE_METHOD(PermanentStoreTest, "Restart", "[Functional]") {
  const size_t num_entries(10 * OneKB), disk_entries(1000 * OneKB);
  KeyValueContainer key_value_pairs(
      PopulatePermanentStore(num_entries, disk_entries, permanent_store_path_));
  DiskUsage disk_usage(1000 * OneKB * OneKB);
  std::cout << "Resetting permanent store..." << std::endl;
  pt::ptime start_time(pt::microsec_clock::universal_time());
  permanent_store_.reset(new PermanentStore(permanent_store_path_, disk_usage));
  pt::ptime stop_time(pt::microsec_clock::universal_time());
  PrintResult(start_time, stop_time);
  REQUIRE((num_entries * OneKB) == permanent_store_->GetCurrentDiskUsage().data);
}

}  // namespace test

}  // namespace data_stores

}  // namespace maidsafe
