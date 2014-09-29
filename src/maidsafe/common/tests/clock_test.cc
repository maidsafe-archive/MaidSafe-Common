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

#include "maidsafe/common/clock.h"
#include "maidsafe/common/test.h"

namespace maidsafe {
namespace test {

TEST(ClockTest, BEH_TimeConversion) {
  auto result = common::Clock::from_time_t(42);
  EXPECT_EQ(result.time_since_epoch(), std::chrono::seconds(42));
}

TEST(ClockTest, BEH_TimePointConversion) {
  common::Clock::time_point tp(std::chrono::seconds(43));
  auto result = common::Clock::to_time_t(tp);
  EXPECT_EQ(result, 43);
}

}  // namespace test
}  // namespace maidsafe
