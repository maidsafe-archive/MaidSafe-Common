/*  Copyright 2014 MaidSafe.net limited

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

#include "maidsafe/common/make_unique.h"

#include <string>

#include "maidsafe/common/test.h"

namespace maidsafe {

namespace test {

TEST(MakeUniqueTest, BEH_MakeUnique) {
  EXPECT_EQ((*make_unique<int>()), 0);
  EXPECT_EQ((*make_unique<int>(1314)), 1314);
  EXPECT_TRUE(make_unique<std::string>()->empty());
  EXPECT_EQ((*make_unique<std::string>("Caol Ila")), "Caol Ila");
  EXPECT_EQ((*make_unique<std::string>(6, 'z')), "zzzzzz");
  auto up(make_unique<int[]>(5));
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(up[i], 0);
}

}  // namespace test

}  // namespace maidsafe
