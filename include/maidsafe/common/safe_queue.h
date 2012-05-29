/* Copyright (c) 2011 maidsafe.net limited
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

#ifndef MAIDSAFE_COMMON_SAFE_QUEUE_H_
#define MAIDSAFE_COMMON_SAFE_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

// thread and exception safe queue
// pop elements by reference to not throw :)

template<typename T>
class SafeQueue {
public:
  SafeQueue() :
    queue_(),
    mutex_(),
    condition_() {};
    // move OK copy disallowed
  SafeQueue(const SafeQueue&& other) {
    other.queue_(std::move(queue_));
    other.condition_(std::move(condition_));
    other.mutex_(std::move(mutex_));
  }
  SafeQueue& operator=(const SafeQueue&& other) {
    other.queue_ = (std::move(queue_));
    other.condition_ = (std::move(condition_));
    other.mutex_ = (std::move(mutex_));
  }
  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }
  unsigned Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }
  void Push(T element){
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(element);
    condition_.notify_one();
  }
  bool TryPop(T &element) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(queue_.empty()){
      return false;
    }
    element = queue_.front();
    queue_.pop();
    return true;
  }
  void WaitAndPop(T &element) {
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty()) {
      condition_.wait(lock);  // needs unique_lock
    }
    element = queue_.front();
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
