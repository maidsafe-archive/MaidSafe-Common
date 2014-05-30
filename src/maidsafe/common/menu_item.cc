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

#include "maidsafe/common/menu_item.h"

#include <algorithm>
#include <tuple>
#include <utility>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/make_unique.h"

namespace maidsafe {

MenuItem::MenuItem(std::string name, Functor operation)
    : kParent_(nullptr), kName_(std::move(name)), kOperation_(std::move(operation)), children_() {
  if (kName_.empty())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
}

MenuItem::MenuItem(MenuItem* parent, std::string name, Functor operation)
    : kParent_(parent), kName_(std::move(name)), kOperation_(std::move(operation)), children_() {
  assert(kParent_);
  if (kName_.empty())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
}

MenuItem* MenuItem::AddChildItem(std::string name, Functor operation) {
  if (std::any_of(std::begin(children_), std::end(children_),
      [&name](const std::unique_ptr<MenuItem>& child) { return name == child->kName_; })) {
    LOG(kError) << "This menu item already has a child with the given name.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
  children_.emplace_back(
      std::unique_ptr<MenuItem>{ new MenuItem{ this, std::move(name), std::move(operation) } });
  return children_.back().get();
}

void MenuItem::ShowChildrenNames() const {
  size_t max_length{ kName_.size() };
  std::string output;
  for (size_t i{ 1 }; i <= children_.size(); ++i) {
    max_length = std::max(max_length, children_[i - 1]->kName_.size() + 6);
    output += std::to_string(i) + (i < 10 ? " " : "") + "  " + children_[i - 1]->kName_ + '\n';
  }
  if (kParent_) {
    max_length = std::max(max_length, kParent_->kName_.size() + 13);
    output += "99  Back to: " + kParent_->kName_ + '\n';
  }
  std::string line(max_length, '~');
  TLOG(kDefaultColour) << line << '\n' << output << line << '\n';
}

const MenuItem* MenuItem::Child(int index) const {
  return children_.at(static_cast<size_t>(index)).get();
}

bool MenuItem::HasNoChildren() const {
  return children_.empty();
}

void MenuItem::DoOperation() const {
  if (kOperation_)
    kOperation_();
}

}  // namespace maidsafe
