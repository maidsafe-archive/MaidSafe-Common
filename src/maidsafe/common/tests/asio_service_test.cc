/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

TEST(AsioServiceTest, BEH_All) {
  bool done(false);
  std::mutex mutex;
  std::condition_variable cond_var;

  auto task([&] {
    std::lock_guard<std::mutex> lock(mutex);
    done = true;
    cond_var.notify_one();
  });

  {  // Allocate no threads
    AsioService asio_service(0);
    asio_service.Start();
    asio_service.service().post(task);
    std::unique_lock<std::mutex> lock(mutex);
    EXPECT_FALSE(cond_var.wait_for(lock, std::chrono::milliseconds(100), [&] { return done; }));
    EXPECT_FALSE(done);
  }

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

  {  // Stop while executing interruptible long-running tasks
    int task_count(500);
    const bptime::seconds kTaskDuration(2);
    int sleeps_interrupted(0), sleeps_not_interrupted(0);

    auto interruptible_task([&] {
      bool sleep_completed(Sleep(kTaskDuration));
      std::lock_guard<std::mutex> lock(mutex);
      sleep_completed ? ++sleeps_not_interrupted : ++sleeps_interrupted;
    });

    AsioService asio_service(5);
    asio_service.Start();
    bptime::ptime start_time(bptime::microsec_clock::local_time());
    for (int i(0); i != task_count; ++i)
      asio_service.service().post(interruptible_task);
    asio_service.Stop();
    EXPECT_EQ(task_count, sleeps_interrupted);
    EXPECT_EQ(0, sleeps_not_interrupted);
    EXPECT_LT(bptime::microsec_clock::local_time() - start_time, kTaskDuration * 2);

    // Stop while executing non-interruptible long-running tasks
    task_count = 4;
    sleeps_interrupted = sleeps_not_interrupted = 0;
    auto non_interruptible_task([interruptible_task] {
      boost::this_thread::disable_interruption disable_interruption;
      interruptible_task();
    });

    asio_service.Start();
    start_time = bptime::microsec_clock::local_time();
    for (int i(0); i != task_count; ++i)
      asio_service.service().post(non_interruptible_task);
    asio_service.Stop();
    EXPECT_EQ(task_count, sleeps_not_interrupted);
    EXPECT_EQ(0, sleeps_interrupted);
    EXPECT_GE(bptime::microsec_clock::local_time() - start_time, kTaskDuration);
  }
}

}  // namespace test

}  // namespace maidsafe
