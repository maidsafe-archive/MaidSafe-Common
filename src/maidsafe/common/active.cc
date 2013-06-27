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

#include "maidsafe/common/active.h"


namespace maidsafe {

Active::Active() : running_(true),
                   functors_(),
                   flags_mutex_(),
                   mutex_(),
                   condition_(),
                   thread_([this] { Run(); }) {}

Active::~Active() {
  Send([this] {
    std::lock_guard<std::mutex> flags_lock(flags_mutex_);
    running_ = false;
  });
  thread_.join();
}

void Active::Send(Functor functor) {
  std::lock_guard<std::mutex> flags_lock(flags_mutex_);
  if (!running_)
    return;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors_.push(std::move(functor));
  }
  condition_.notify_one();
}

void Active::Run() {
  auto running = [this]()->bool {
    std::lock_guard<std::mutex> flags_lock(flags_mutex_);
    return running_;
  };

  while (running()) {
    Functor functor;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (functors_.empty())
        condition_.wait(lock);
      functor = std::move(functors_.front());
      functors_.pop();
    }
    functor();
  }
}

}  // namespace maidsafe
