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

#ifndef MAIDSAFE_COMMON_CLOCK_H_
#define MAIDSAFE_COMMON_CLOCK_H_

#include <chrono>

#include "maidsafe/common/config.h"

namespace maidsafe {

// Chrono clock that uses UTC from epoch 1970-01-01T00:00:00Z with 1 nanosecond resolution.
// The resolution accuracy depends on the system clock.
struct Clock {
  typedef std::chrono::nanoseconds duration;
  typedef duration::rep rep;
  typedef duration::period period;
  typedef std::chrono::time_point<Clock> time_point;

  static const bool is_steady = std::chrono::system_clock::is_steady;

  // Returns the current time.
  //
  // Assumes that chrono::system_clock uses 1970-01-01 as epoch. If this assumption does not
  // hold, then the MAIDSAFE_CLOCK_EPOCH_OFFSET_IN_DAYS can be defined to the number of days
  // from 1970-01-01 until the chrono::system_clock epoch.
  static time_point now() MAIDSAFE_NOEXCEPT;

  // Convert time_point to time_t
  static std::time_t to_time_t(const time_point&);
  // Convert time_t to time_point
  static time_point from_time_t(std::time_t);
};

namespace common {
using Clock = maidsafe::Clock;
}  // namespace common

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CLOCK_H_
