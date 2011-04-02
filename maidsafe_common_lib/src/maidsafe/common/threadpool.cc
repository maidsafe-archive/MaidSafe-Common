/* Copyright (c) 2009 maidsafe.net limited
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

#include "maidsafe/common/threadpool.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

Threadpool::Threadpool(const boost::uint8_t &poolsize)
    : io_service_(), work_(), thread_group_() {
  // add work to prevent io_service completing
  work_.reset(new boost::asio::io_service::work(io_service_));
  for (boost::uint8_t i = 0; i < poolsize; ++i)
    thread_group_.create_thread(boost::bind(&Threadpool::Start, this));
}

void Threadpool::Start() {
  while (work_) {
    try {
      io_service_.run();
    }
    catch(const std::exception &e) {
      DLOG(ERROR) << "Threadpool::Start - " << e.what() << std::endl;
    }
  }
}

void Threadpool::Stop() {
  work_.reset();
}

Threadpool::~Threadpool() {
  Stop();
  thread_group_.join_all();
}

bool Threadpool::EnqueueTask(const VoidFunctor &functor) {
  try {
    io_service_.post(functor);
    return true;
  }
  catch(const std::exception &e) {
    DLOG(ERROR) << "Cannot post job to pool: " << e.what() << std::endl;
    return false;
  }
}

}  // namespace maidsafe
