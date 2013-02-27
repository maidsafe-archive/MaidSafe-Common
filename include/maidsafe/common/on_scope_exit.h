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

 *  Original author:
 *  Copyright Jon Kalb 2007 - 2012.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_
#define MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_

#include <functional>


namespace maidsafe {

struct on_scope_exit {
  typedef std::function<void(void)> ExitAction;

  template<typename CleanupAction>
  explicit on_scope_exit(CleanupAction action) : action_(action) {}

  explicit on_scope_exit(ExitAction action) : action_(action) {}

  ~on_scope_exit() {
    if (action_)
      action_();
  }

  void SetAction(ExitAction action = nullptr) { action_ = action; }

  void Release() { SetAction(); }

  template<typename T>
  static void SetValue(T& t, T value) { t = value; }

  template<typename T>
  static ExitAction RevertValue(T& t) { return std::bind(SetValue<T>, std::ref(t), t); }

 private:
  on_scope_exit();
  on_scope_exit(const on_scope_exit&);
  on_scope_exit& operator=(const on_scope_exit&);
  on_scope_exit(on_scope_exit&&);
  on_scope_exit& operator=(on_scope_exit&&);
  ExitAction action_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_
