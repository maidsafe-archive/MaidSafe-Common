/*  Copyright 2014 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_MENU_ITEM_H_
#define MAIDSAFE_COMMON_MENU_ITEM_H_

#include <functional>
#include <string>
#include <vector>

#include "maidsafe/common/config.h"

namespace maidsafe {

class MenuItem {
 public:
  typedef std::function<void()> Functor;

  MenuItem() = delete;
  // This creates a MenuItem with a NULL parent - it should only be used when creating the top-level
  // set of MenuItems held in the Menu class.
  MenuItem(std::string name, Functor operation);
  MenuItem(const MenuItem& other) = delete;
  MenuItem(MenuItem&& other) = delete;
  MenuItem& operator=(MenuItem other) = delete;
  ~MenuItem() = default;

  // Returns a mutable pointer to the newly-added item so it can be populated with children if
  // required.
  MenuItem* AddChildItem(std::string name, Functor operation = nullptr);
  void ShowChildrenNames() const;
  const MenuItem* Child(int index) const;
  bool HasNoChildren() const;
  void DoOperation() const;

  const MenuItem* const kParent_;
  const std::string kName_;

 private:
  MenuItem(MenuItem* parent, std::string name, Functor operation);

  const Functor kOperation_;
  std::vector<std::unique_ptr<MenuItem>> children_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MENU_ITEM_H_
