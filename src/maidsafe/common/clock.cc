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

namespace maidsafe {

Clock::time_point Clock::now() MAIDSAFE_NOEXCEPT {
  auto time_now = std::chrono::system_clock::now();
// Assumes that system_clock uses 1970-01-01 as epoch
#if defined(MAIDSAFE_CLOCK_EPOCH_OFFSET_IN_DAYS)
  time_now += std::chrono::days(MAIDSAFE_CLOCK_EPOCH_OFFSET_IN_DAYS);
#endif
  return time_point(std::chrono::duration_cast<duration>(time_now.time_since_epoch()));
}

std::time_t Clock::to_time_t(const time_point& t) {
  return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
}

Clock::time_point Clock::from_time_t(std::time_t t) { return time_point(std::chrono::seconds(t)); }

}  // namespace maidsafe
