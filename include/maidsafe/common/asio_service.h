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

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/thread/thread.hpp"


namespace maidsafe {

class AsioService {
 public:
  explicit AsioService(const uint32_t &thread_count);
  ~AsioService();
  void Start();
  void Stop();
  boost::asio::io_service& service();

 private:
  boost::asio::io_service service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  std::vector<boost::thread> threads_;
  std::mutex mutex_;
};

}  // namespace maidsafe



#endif  // MAIDSAFE_COMMON_ASIO_SERVICE_H_
