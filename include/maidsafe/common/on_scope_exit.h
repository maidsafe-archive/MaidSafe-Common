/*  Copyright Jon Kalb 2007 - 2012.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_
#define MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_

#include <functional>
#include "boost/bind.hpp"

namespace maidsafe {


template<typename T>
void SetValue(T& t, T value) { t = value; }

struct on_scope_exit {
  typedef std::function<void()> ExitAction;

  template<typename CleanupAction>
  explicit on_scope_exit(CleanupAction action)
  try : action_(action) {}
  catch(...) {
    action();
  }

  explicit on_scope_exit(ExitAction action) : action_(action) {}

  ~on_scope_exit() {
    if (action_)
      action_();
  }

  void SetAction(ExitAction action = nullptr) { action_ = action; }

  void Release() { SetAction(); }

  template<typename T>
  static ExitAction RevertValue(T& t) { return boost::bind(SetValue<T>, std::ref(t), t); }

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
