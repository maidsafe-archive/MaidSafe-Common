/* Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/asio_service.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace bptime = boost::posix_time;

namespace maidsafe {

namespace test {

TEST(AsioServiceTest, BEH_InvalidStart) {
  bool done(false);
  std::mutex mutex;
  std::condition_variable cond_var;

  auto task([&] {
    std::lock_guard<std::mutex> lock(mutex);
    done = true;
    cond_var.notify_one();
  });

  // Allocate no threads
  EXPECT_THROW(AsioService asio_service(0), std::exception);

  {  // Start after posting tasks
    AsioService asio_service(2);
    asio_service.service().post(task);
    std::unique_lock<std::mutex> lock(mutex);
    EXPECT_FALSE(cond_var.wait_for(lock, std::chrono::milliseconds(100), [&] { return done; }));
    EXPECT_FALSE(done);

    asio_service.Start();
    EXPECT_TRUE(cond_var.wait_for(lock, std::chrono::milliseconds(100), [&] { return done; }));
    EXPECT_TRUE(done);
    asio_service.Start();
  }
}

TEST(AsioServiceTest, BEH_Interrupt) {
  // Stop while executing interruptible long-running tasks
  std::mutex mutex;
  int task_count(100);
  const boost::chrono::seconds kTaskDuration(2);
  int sleeps_interrupted(0), sleeps_not_interrupted(0);

  auto interruptible_task([&] {
    bool sleep_completed(Sleep(kTaskDuration));
    std::lock_guard<std::mutex> lock(mutex);
    sleep_completed ? ++sleeps_not_interrupted : ++sleeps_interrupted;
  });

  AsioService asio_service(5);
  asio_service.Start();
  auto start_time(boost::chrono::high_resolution_clock::now());
  for (int i(0); i != task_count; ++i)
    asio_service.service().post(interruptible_task);
  asio_service.Stop();
  EXPECT_EQ(task_count, sleeps_interrupted);
  EXPECT_EQ(0, sleeps_not_interrupted);
  EXPECT_LT(boost::chrono::high_resolution_clock::now() - start_time, kTaskDuration * 2);

  // Stop while executing non-interruptible long-running tasks
  task_count = 4;
  sleeps_interrupted = sleeps_not_interrupted = 0;
  auto non_interruptible_task([interruptible_task] {
    boost::this_thread::disable_interruption disable_interruption;
    interruptible_task();
  });

  asio_service.Start();
  start_time = boost::chrono::high_resolution_clock::now();
  for (int i(0); i != task_count; ++i)
    asio_service.service().post(non_interruptible_task);
  asio_service.Stop();
  EXPECT_EQ(task_count, sleeps_not_interrupted);
  EXPECT_EQ(0, sleeps_interrupted);
  EXPECT_GE(boost::chrono::high_resolution_clock::now() - start_time, kTaskDuration);
}

}  // namespace test

}  // namespace maidsafe
