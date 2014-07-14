// NoCheck
/*  Copyright Jon Kalb 2007 - 2012.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_
#define MAIDSAFE_COMMON_ON_SCOPE_EXIT_H_

#include <functional>
#include "boost/bind.hpp"

namespace maidsafe {

struct on_scope_exit {
  typedef std::function<void()> ExitAction;

  template <typename CleanupAction>
  explicit on_scope_exit(CleanupAction action) try : action_(action) {}
  catch (...) {
    action();
  }

  explicit on_scope_exit(ExitAction action) : action_(std::move(action)) {}

  ~on_scope_exit() {
    if (action_)
      action_();
  }

  void SetAction(ExitAction action = nullptr) { action_ = action; }

  template <typename T>
  static void SetValue(T& t, T value) {
    t = value;
  }

  void Release() { SetAction(); }

  template <typename T>
#ifdef __clang__
  static ExitAction RevertValue(T& t) {
    return boost::bind(SetValue<T>, std::ref(t), t);
  }
#else
  static ExitAction RevertValue(T& t) { return std::bind(SetValue<T>, std::ref(t), t); }
#endif

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
