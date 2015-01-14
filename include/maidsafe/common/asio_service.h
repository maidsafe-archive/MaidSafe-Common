/*  Copyright 2012 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_ASIO_SERVICE_H_
#define MAIDSAFE_COMMON_ASIO_SERVICE_H_

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "asio/io_service.hpp"
#include "boost/asio/io_service.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

template <typename IoServiceType>
class IoService {
 public:
  explicit IoService(size_t thread_count);
  ~IoService() { Stop(); }
  void Stop();
  IoServiceType& service() { return service_; }
  size_t ThreadCount() const { return thread_count_; }

 private:
  std::atomic<size_t> thread_count_;
  IoServiceType service_;
  std::unique_ptr<typename IoServiceType::work> work_;
  std::vector<std::thread> threads_;
  mutable std::mutex mutex_;
};

using AsioService = IoService<asio::io_service>;
using BoostAsioService = IoService<boost::asio::io_service>;



template <typename IoServiceType>
IoService<IoServiceType>::IoService(size_t thread_count)
    : thread_count_(thread_count),
      service_(),
      work_(make_unique<typename IoServiceType::work>(service_)),
      threads_(),
      mutex_() {
  if (thread_count == 0)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  for (size_t i(0); i != thread_count; ++i)
    threads_.emplace_back([&] {
      try {
        service_.run();
      } catch (...) {
        LOG(kError) << boost::current_exception_diagnostic_information();
        // Rethrowing here will cause the application to terminate - so flush the log message first.
        log::Logging::Instance().Flush();
        assert(0);
        throw;
      }
    });
}

template <typename IoServiceType>
void IoService<IoServiceType>::Stop() {
  thread_count_ = 0U;
  std::lock_guard<std::mutex> lock{mutex_};
  if (!work_)
    return;
  for (const auto& asio_thread : threads_) {
    if (std::this_thread::get_id() == asio_thread.get_id())
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::cannot_invoke_from_this_thread));
  }

  work_.reset();

  for (auto& asio_thread : threads_) {
    try {
      asio_thread.join();
    } catch (const std::exception& e) {
      LOG(kError) << "Exception joining asio thread: " << boost::diagnostic_information(e);
      asio_thread.detach();
    }
  }

  threads_.clear();
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ASIO_SERVICE_H_
