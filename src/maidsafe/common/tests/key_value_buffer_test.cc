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
  ASSERT_EQ(0, max_memory_usage);
  ASSERT_EQ(0, max_disk_usage);
  KeyValueBuffer key_value_buffer(max_memory_usage, max_disk_usage);
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
