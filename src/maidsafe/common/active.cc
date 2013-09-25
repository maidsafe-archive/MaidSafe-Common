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

#include "maidsafe/common/active.h"

namespace maidsafe {

Active::Active()
    : running_(true),
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
