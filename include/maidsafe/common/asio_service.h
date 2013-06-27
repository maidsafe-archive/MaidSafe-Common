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
