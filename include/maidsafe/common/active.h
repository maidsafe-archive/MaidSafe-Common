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

#ifndef MAIDSAFE_COMMON_ACTIVE_H_
#define MAIDSAFE_COMMON_ACTIVE_H_

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

#include "boost/thread/thread.hpp"


namespace maidsafe {

class Active {
 public:
  typedef std::function<void()> Functor;
  Active();
  ~Active();
  void Send(Functor functor);

 private:
  Active(const Active&);
  Active& operator=(const Active&);
  void Run();
  // FIXME this should be atomic<bool> but clang is violently complaining !!
  bool running_;
  std::queue<Functor> functors_;
  std::mutex flags_mutex_, mutex_;
  std::condition_variable condition_;
  boost::thread thread_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ACTIVE_H_
