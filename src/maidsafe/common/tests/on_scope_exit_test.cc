/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#include "maidsafe/common/on_scope_exit.h"

#include <vector>

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"


namespace maidsafe {

namespace test {

void Increment(std::vector<int32_t>& data) {
  data.push_back(static_cast<int32_t>(data.size()));
}

void IncrementAndThrow(std::vector<int32_t>& data) {
  Increment(data);
  ThrowError(CommonErrors::invalid_parameter);
}

TEST(OnScopeExit, BEH_RevertValue) {
  std::vector<int32_t> before;
  for (int i(0); i != 100; ++i)
    before.push_back(i);
  {
    on_scope_exit strong_guarantee(on_scope_exit::RevertValue(before));
    Increment(before);
    EXPECT_EQ(before.size(), 101);
    strong_guarantee.Release();
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);

  try {
    on_scope_exit strong_guarantee(on_scope_exit::RevertValue(before));
    IncrementAndThrow(before);
  }
  catch(const maidsafe_error&) {
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);
}

TEST(OnScopeExit, BEH_SetAction) {
  std::vector<int32_t> before;
  for (int i(0); i != 100; ++i)
    before.push_back(i);
  {
    on_scope_exit strong_guarantee([&before]() { before.clear(); });
    Increment(before);
    EXPECT_EQ(before.size(), 101);
    strong_guarantee.Release();
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);

  try {
    on_scope_exit strong_guarantee([&before]() { before.clear(); });
    IncrementAndThrow(before);
  }
  catch(const maidsafe_error&) {
    EXPECT_TRUE(before.empty());
  }
  EXPECT_TRUE(before.empty());
}

}  // namespace test

}  // namespace maidsafe
