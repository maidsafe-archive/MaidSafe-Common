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
