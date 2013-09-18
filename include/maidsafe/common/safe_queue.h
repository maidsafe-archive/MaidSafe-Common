/*  Copyright 2011 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_SAFE_QUEUE_H_
#define MAIDSAFE_COMMON_SAFE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

// thread and exception safe queue
// pop elements by reference to not throw :)

template<typename T>
class SafeQueue {
 public:
  SafeQueue() : queue_(), mutex_(), condition_() {}

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  void Push(T element) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(element));
    }
    condition_.notify_one();
  }

  bool TryPop(T &element) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
      return false;
    element = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  void WaitAndPop(T &element) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
      condition_.wait(lock);
    element = std::move(queue_.front());
    queue_.pop();
  }

 private:
  SafeQueue& operator=(const SafeQueue&);
  SafeQueue(const SafeQueue& other);
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable condition_;
};

#endif  // MAIDSAFE_COMMON_SAFE_QUEUE_H_
