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
#include "maidsafe/common/log.h"

namespace maidsafe {

AsioService::AsioService(uint32_t thread_count)
    : service_(), work_(), threads_(thread_count), mutex_() {
  if (thread_count == 0)
    ThrowError(CommonErrors::invalid_parameter);
}

AsioService::~AsioService() { Stop(); }

void AsioService::Start() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto& asio_thread : threads_) {
    if (std::this_thread::get_id() == asio_thread.get_id())
      ThrowError(CommonErrors::cannot_invoke_from_this_thread);
  }

  if (work_) {
    LOG(kError) << "AsioService is already running with " << threads_.size() << " threads.";
    return;
  }
  service_.reset();
  work_.reset(new boost::asio::io_service::work(service_));
  for (auto& asio_thread : threads_)
    asio_thread = std::move(std::thread([&] { service_.run(); }));
}

void AsioService::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!work_) {
    LOG(kVerbose) << "AsioService has already stopped.";
    return;
  }
  for (const auto& asio_thread : threads_) {
    if (std::this_thread::get_id() == asio_thread.get_id())
      ThrowError(CommonErrors::cannot_invoke_from_this_thread);
  }

  work_.reset();

  for (auto& asio_thread : threads_) {
    try {
      asio_thread.join();
    }
    catch (const std::exception& e) {
      LOG(kError) << "Exception joining asio thread: " << e.what();
    }
  }
}

boost::asio::io_service& AsioService::service() { return service_; }

}  // namespace maidsafe
