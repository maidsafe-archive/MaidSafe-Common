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

#include "maidsafe/common/asio_service.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

AsioService::AsioService(size_t thread_count)
    : thread_count_(thread_count),
      service_(),
      work_(make_unique<boost::asio::io_service::work>(service_)),
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
        throw;
      }
    });
}

AsioService::~AsioService() { Stop(); }

void AsioService::Stop() {
  thread_count_ = 0U;
  std::lock_guard<std::mutex> lock{mutex_};
  if (!work_) {
    LOG(kVerbose) << "AsioService has already stopped.";
    return;
  }
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

boost::asio::io_service& AsioService::service() { return service_; }

size_t AsioService::ThreadCount() const { return thread_count_; }

}  // namespace maidsafe
