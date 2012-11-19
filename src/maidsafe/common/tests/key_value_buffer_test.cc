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


namespace maidsafe {

namespace test {

typedef std::pair<uint64_t, uint64_t> MaxMemoryDiskUsage;

class KeyValueBufferTest : public testing::Test {
 protected:
  KeyValueBufferTest()
      : max_memory_usage_(1000),
        max_disk_usage_(2000),
        pop_functor_(),
        key_value_buffer_(new KeyValueBuffer(max_memory_usage_, max_disk_usage_, pop_functor_)) {}
  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  KeyValueBuffer::PopFunctor pop_functor_;
  std::unique_ptr<KeyValueBuffer> key_value_buffer_;
};

TEST_F(KeyValueBufferTest, BEH_Constructor) {
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(0), DiskUsage(0), pop_functor_), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(1), pop_functor_), std::exception);
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
  NonEmptyString recovered;
  EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key1));
  EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key2));
}

TEST_F(KeyValueBufferTest, BEH_UnsuccessfulStore) {
  NonEmptyString value(std::string(
                    static_cast<uint32_t>(max_disk_usage_ + max_memory_usage_ + 1), 'a'));
  Identity key(crypto::Hash<crypto::SHA512>(value));
  EXPECT_THROW(key_value_buffer_->Store(key, value), std::exception);
}

TEST_F(KeyValueBufferTest, BEH_SetMaxDiskMemoryUsage) {
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ - 1)));
  EXPECT_NO_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_)));
  EXPECT_THROW(key_value_buffer_->SetMaxMemoryUsage(MemoryUsage(max_disk_usage_ + 1)),
               std::exception);
  EXPECT_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_memory_usage_ - 1)),
               std::exception);
  //EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_memory_usage_)));
  //EXPECT_NO_THROW(key_value_buffer_->SetMaxDiskUsage(DiskUsage(max_memory_usage_ + 1)));
}

class KeyValueBufferTestDiskUsage : public testing::TestWithParam<MaxMemoryDiskUsage> {
 protected:
  KeyValueBufferTestDiskUsage()
      : max_memory_usage_(GetParam().first),
        max_disk_usage_(GetParam().second),
        pop_functor_(),
        key_value_buffer_(new KeyValueBuffer(max_memory_usage_, max_disk_usage_, pop_functor_)) {}
  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  KeyValueBuffer::PopFunctor pop_functor_;
  std::unique_ptr<KeyValueBuffer> key_value_buffer_;
};

// Store to max_disk_usage...
TEST_P(KeyValueBufferTestDiskUsage, BEH_SuccessfulStore) {
  uint64_t disk_usage(max_disk_usage_);
  while(disk_usage != 0) {
    NonEmptyString value(std::string(RandomAlphaNumericString(
                      static_cast<uint32_t>(max_memory_usage_))));
    Identity key(crypto::Hash<crypto::SHA512>(value));
    EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
    NonEmptyString recovered;
    EXPECT_NO_THROW(recovered = key_value_buffer_->Get(key));
    EXPECT_EQ(value, recovered);
    disk_usage -= max_memory_usage_;
  }
}

INSTANTIATE_TEST_CASE_P(KeyValueBufferSuccessfulStore,
                        KeyValueBufferTestDiskUsage,
                        testing::Values(std::make_pair(1, 1024),
                                        std::make_pair(8, 1024),
                                        std::make_pair(1024, 2048),
                                        std::make_pair(1024, 1024)));


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

// Store to max_disk_usage + max_memory_usage...
TEST_P(KeyValueBufferTestDiskMemoryUsage, BEH_SuccessfulStore) {
  uint64_t disk_usage(max_disk_usage_), memory_usage(max_memory_usage_),
           total_usage(disk_usage + memory_usage);
  while(total_usage != 0) {
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

// Store greater than max_disk_usage + max_memory_usage...
TEST_P(KeyValueBufferTestDiskMemoryUsage, BEH_UnsuccessfulStore) {
  uint64_t disk_usage(max_disk_usage_), memory_usage(max_memory_usage_),
           total_usage(disk_usage + memory_usage);
  while(total_usage != 0) {
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
  NonEmptyString value(std::string(RandomAlphaNumericString(static_cast<uint32_t>(1))));
  Identity key(crypto::Hash<crypto::SHA512>(value));
  EXPECT_THROW(key_value_buffer_->Store(key, value), std::exception);
}

TEST_P(KeyValueBufferTestDiskMemoryUsage, BEH_Delete) {
  uint64_t disk_usage(max_disk_usage_), memory_usage(max_memory_usage_),
           total_usage(disk_usage + memory_usage);
  std::map<Identity, NonEmptyString> key_value_pairs;
  while(total_usage != 0) {
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

INSTANTIATE_TEST_CASE_P(KeyValueBufferStore,
                        KeyValueBufferTestDiskMemoryUsage,
                        testing::Values(std::make_pair(1, 1024),
                                        std::make_pair(8, 1024),
                                        std::make_pair(1024, 2048),
                                        std::make_pair(1024, 1024)));

}  // namespace test

}  // namespace maidsafe
