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
#include "maidsafe/common/log.h"


namespace maidsafe {

AsioService::AsioService(const uint32_t &thread_count) : service_(), work_(), threads_(thread_count) {}

AsioService::~AsioService() {
  Stop();
}

void AsioService::Start() {
  if (work_) {
    LOG(kError) << "AsioService is already running with "
                << threads_.size() << " threads.";
    return;
  }
  service_.reset();
  work_.reset(new boost::asio::io_service::work(service_));
  for (uint32_t i = 0; i != threads_.size(); ++i) {
      threads_[i] = std::move(boost::thread([&]() {
        for (;;) {
          try {
            service_.run();
            if (!work_)
              break;
          }
          catch(const boost::system::system_error &e) {
            LOG(kError) << e.what();
          }
        }
    }));
  }
}

void AsioService::Stop() {
  work_.reset();
  for (boost::thread &thread : threads_) {
    thread.interrupt();
    thread.join();
  }
}

boost::asio::io_service& AsioService::service() {
  return service_;
}

}  // namespace maidsafe
