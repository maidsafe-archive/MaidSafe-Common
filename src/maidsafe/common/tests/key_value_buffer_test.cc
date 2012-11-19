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

class KeyValueBufferTest : public testing::Test {
 protected:
  KeyValueBufferTest()
      : max_memory_usage_(1000),
        max_disk_usage_(2000),
        key_value_buffer_(new KeyValueBuffer(max_memory_usage_, max_disk_usage_)) {}
  MemoryUsage max_memory_usage_;
  DiskUsage max_disk_usage_;
  std::unique_ptr<KeyValueBuffer> key_value_buffer_;
};

TEST_F(KeyValueBufferTest, BEH_Constructor) {
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(0), DiskUsage(0)), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(1)), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(1), DiskUsage(0)), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(2), DiskUsage(1)), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(200001), DiskUsage(200000)), std::exception);
  EXPECT_NO_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000)));
  // Create a path to a file, and check that this can't be used as the disk buffer path.
  TestPath test_path(CreateTestPath("MaidSafe_Test_KeyValueBuffer"));
  ASSERT_FALSE(test_path->empty());
  boost::filesystem::path file_path(*test_path / "File");
  ASSERT_TRUE(WriteFile(file_path, " "));
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000), file_path), std::exception);
  EXPECT_THROW(KeyValueBuffer(MemoryUsage(199999), DiskUsage(200000), file_path / "base"),
               std::exception);
}

TEST_F(KeyValueBufferTest, BEH_SuccessfulStore) {
  NonEmptyString value(std::string(max_disk_usage_, 'a'));
  Identity key(crypto::Hash<crypto::SHA512>(value));
  EXPECT_NO_THROW(key_value_buffer_->Store(key, value));
}

TEST_F(KeyValueBufferTest, BEH_UnsuccessfulStore) {
  NonEmptyString value(std::string(max_disk_usage_ + 1, 'a'));
  Identity key(crypto::Hash<crypto::SHA512>(value));
  EXPECT_THROW(key_value_buffer_->Store(key, value), std::exception);
}

}  // namespace test

}  // namespace maidsafe
