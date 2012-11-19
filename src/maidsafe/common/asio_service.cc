/* Copyright (c) 2012 maidsafe.net limited
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
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"


namespace bptime = boost::posix_time;

namespace maidsafe {

AsioService::AsioService(const uint32_t &thread_count)
    : service_(),
      work_(),
      threads_(thread_count),
      mutex_() {
  if (thread_count == 0)
    ThrowError(CommonErrors::invalid_parameter);
}

AsioService::~AsioService() {
  Stop();
}

void AsioService::Start() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& asio_thread : threads_) {
    if (boost::this_thread::get_id() == asio_thread.get_id())
      ThrowError(CommonErrors::cannot_invoke_from_this_thread);
  }

  if (work_) {
    LOG(kError) << "AsioService is already running with " << threads_.size() << " threads.";
    return;
  }
  service_.reset();
  work_.reset(new boost::asio::io_service::work(service_));
  for (auto& asio_thread : threads_) {
    asio_thread = std::move(boost::thread([&] {
#ifdef NDEBUG
        try {
          service_.run();
        }
        catch(const boost::system::system_error& e) {
          LOG(kError) << e.what();
          assert(false);
        }
        catch(const std::exception& e) {
          LOG(kError) << e.what();
          assert(false);
        }
        catch(...) {
          LOG(kError) << "Unknown exception.";
          assert(false);
        }
#else
        service_.run();
#endif
    }));
  }
}

void AsioService::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!work_) {
    LOG(kVerbose) << "AsioService has already stopped.";
    return;
  }
  for (auto& asio_thread : threads_) {
    if (boost::this_thread::get_id() == asio_thread.get_id())
      ThrowError(CommonErrors::cannot_invoke_from_this_thread);
  }

  work_.reset();
  // Interrupt and join all asio worker threads concurrently
  std::vector<boost::thread> joining_workers(threads_.size());
  for (size_t i(0); i < threads_.size(); ++i) {
    joining_workers[i] = std::move(boost::thread([&, i] {
        while (threads_[i].joinable()) {
          try {
            threads_[i].interrupt();
            threads_[i].timed_join(bptime::milliseconds(1));
            boost::this_thread::sleep(bptime::milliseconds(10));
          }
          catch(const boost::thread_interrupted&) {
            LOG(kError) << "Exception joining boost thread with ID " << threads_[i].get_id();
            boost::this_thread::yield();
          }
        }
    }));
  }

  for (boost::thread &joining_worker : joining_workers) {
    while (joining_worker.joinable()) {
      try {
        joining_worker.join();
      }
      catch(const boost::thread_interrupted&) {
        LOG(kError) << "Exception joining worker thread";
      }
    }
  }
}

boost::asio::io_service& AsioService::service() {
  return service_;
}

}  // namespace maidsafe
