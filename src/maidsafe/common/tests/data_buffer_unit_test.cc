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

#include <utility>

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/data_name_variant.h"

namespace fs = boost::filesystem;
namespace mpl = boost::mpl;

namespace maidsafe {

namespace data_stores {

namespace test {

TEST(DataBufferUnitTest, BEH_ZeroSizeMemory) {
  EXPECT_NO_THROW(DataBuffer<std::string>(MemoryUsage(0), DiskUsage(100), nullptr));
}

TEST(DataBufferUnitTest, BEH_MaxMemoryLessMaxDiskUsage) {
  EXPECT_THROW(DataBuffer<std::string>(MemoryUsage(1), DiskUsage(0), nullptr), std::exception);
}

TEST(DataBufferUnitTest, BEH_ZeroSizeDiskAndMemory) {
  EXPECT_NO_THROW(DataBuffer<std::string>(MemoryUsage(0), DiskUsage(0), nullptr));
}

TEST(DataBufferUnitTest, BEH_ConstructWithComplexKey) {
  typedef std::pair<std::string, std::string> Key;
  EXPECT_NO_THROW(DataBuffer<Key>(MemoryUsage(0), DiskUsage(100), nullptr));
}

TEST(DataBufferUnitTest, BEH_DiskOnlyInsertAndDelete) {
  DataBuffer<std::string> data_buffer(MemoryUsage(0), DiskUsage(100), nullptr);
  EXPECT_NO_THROW(data_buffer.Store("a", NonEmptyString("b")));
  EXPECT_TRUE(NonEmptyString("b") == data_buffer.Get("a"));
  EXPECT_NO_THROW(data_buffer.Delete(std::string("a")));
  EXPECT_THROW(data_buffer.Delete(std::string("a")), std::exception);
}

TEST(DataBufferUnitTest, BEH_DiskOnlyInsertAndDeleteComplexKey) {
  typedef std::pair<std::string, std::string> Key;
  DataBuffer<Key> data_buffer(MemoryUsage(0), DiskUsage(100), nullptr);
  EXPECT_NO_THROW(data_buffer.Store(std::make_pair("a", "b"), NonEmptyString("b")));
  EXPECT_TRUE(NonEmptyString("b") == data_buffer.Get(std::make_pair("a", "b")));
  EXPECT_NO_THROW(data_buffer.Delete(std::make_pair(std::string("a"), std::string("b"))));
  EXPECT_THROW(data_buffer.Delete(std::make_pair(std::string("a"), std::string("b"))),
      std::exception);
}

TEST(DataBufferUnitTest, BEH_DiskOnlyInsertAndDeleteRange) {
  typedef std::pair<std::string, std::string> Key;
  DataBuffer<Key> data_buffer(MemoryUsage(0), DiskUsage(100), nullptr);
  EXPECT_NO_THROW(data_buffer.Store(std::make_pair("a", "b"), NonEmptyString("b")));
  EXPECT_NO_THROW(data_buffer.Store(std::make_pair("b", "b"), NonEmptyString("b")));
  EXPECT_TRUE(NonEmptyString("b") == data_buffer.Get(std::make_pair("a", "b")));
  std::function<bool(const Key&)> predicate([](const Key& key) { return key.second == "b"; });
  EXPECT_NO_THROW(data_buffer.Delete(predicate));
  EXPECT_THROW(data_buffer.Delete(std::make_pair(std::string("a"), std::string("b"))),
                  std::exception);
  EXPECT_NO_THROW(data_buffer.Delete(predicate));
}


}  // namespace test

}  // namespace data_stores

}  // namespace maidsafe
