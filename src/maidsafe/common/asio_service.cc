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
