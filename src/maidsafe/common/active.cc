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

#include "maidsafe/common/active.h"


namespace maidsafe {

Active::Active() : done_(false),
                   accepting_(true),
                   functors_(),
                   mutex_(),
                   condition_(),
                   thread_([=] { Run(); }) {}

Active::~Active() {
  Send([&] { done_ = true; });  // NOLINT (Fraser)
  if (thread_.joinable())
    thread_.join();
}

void Active::SetAccepting(const bool accepting) {
  accepting_ = accepting;
}

bool Active::Accepting() {
  return accepting_;
}

void Active::Send(Functor functor) {
  if (!accepting_)
    return;
  std::lock_guard<std::mutex> lock(mutex_);
  functors_.push(functor);
  condition_.notify_one();
}

void Active::Run() {
  while (!done_) {
    Functor functor;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (functors_.empty())
        condition_.wait(lock);
      functor = functors_.front();
      functors_.pop();
    }
    functor();
  }
}

}  // namespace maidsafe
