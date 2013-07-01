/* Copyright 2011 MaidSafe.net limited

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
