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

#include "maidsafe/common/node_id.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace detail {

namespace test {


TEST(KeyValueBufferTest, BEH_Constructor) {
  MemoryUsage max_memory_usage;
  DiskUsage max_disk_usage;
  EXPECT_EQ(0, max_memory_usage);
  EXPECT_EQ(0, max_disk_usage);
  KeyValueBuffer key_value_buffer(max_memory_usage, max_disk_usage);
}

TEST(KeyValueBufferTest, BEH_ZeroMemory) {
  MemoryUsage mem_usage(0);
  DiskUsage disk_usage(100);
  EXPECT_EQ(0, mem_usage);
  EXPECT_EQ(100, disk_usage);
  KeyValueBuffer key_value_buffer(mem_usage, disk_usage);
  Identity(NodeId(NodeId::kRandomId).string());

}

TEST(KeyValueBufferTest, BEH_ZeroDisk) {
  MemoryUsage memory_usage(100);
  DiskUsage disk_usage(0);
  EXPECT_EQ(100, memory_usage);
  EXPECT_EQ(0, disk_usage);
  KeyValueBuffer key_value_buffer(memory_usage, disk_usage);
}

TEST(KeyValueBufferTest, BEH_SuccessfulStore) {
  uint64_t disk_usage(256);
  MemoryUsage max_memory_usage(1);
  DiskUsage max_disk_usage(disk_usage);
  EXPECT_EQ(1, max_memory_usage);
  EXPECT_EQ(disk_usage, max_disk_usage);
  KeyValueBuffer key_value_buffer(max_memory_usage, max_disk_usage);
  NonEmptyString content(std::string(static_cast<uint32_t>(disk_usage), 'a'));
  Identity hash(crypto::Hash<crypto::SHA512>(content.string()));
  EXPECT_NO_THROW(key_value_buffer.Store(hash, content));
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
