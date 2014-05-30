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

#include "maidsafe/common/menu.h"

#include <cstdint>
#include <iostream>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/menu_item.h"

namespace maidsafe {

Menu::Menu(std::string main_menu_name)
    : top_level_item_(std::move(main_menu_name), [] {}), current_item_(&top_level_item_), cli_() {}

Menu::Menu(std::string main_menu_name, std::string prompt)
    : top_level_item_(std::move(main_menu_name), [] {}),
      current_item_(&top_level_item_),
      cli_(std::move(prompt)) {}

MenuItem* Menu::AddItem(std::string name, MenuItem::Functor operation) {
  return top_level_item_.AddChildItem(std::move(name), std::move(operation));
}

int Menu::Run() {
  try {
    bool clear_cli{ true };
    for (;;) {
      if (clear_cli) {
        cli_.Clear();
        clear_cli = false;
        if (current_item_->HasNoChildren())
          current_item_ = current_item_->kParent_;
        ShowOptions();
      }
      try {
        int result{ cli_.Get<int>("Please enter option (0 to quit)") };
        if (result == 0) {
          break;
        } else if (result == 99 && current_item_->kParent_) {
          current_item_ = current_item_->kParent_;
        } else {
          current_item_ = current_item_->Child(result - 1);
        }
      }
      catch (const std::exception&) {
        TLOG(kYellow) << "Invalid choice\n";
        continue;
      }
      current_item_->DoOperation();
      clear_cli = true;
    }
    return 0;
  }
  catch (const maidsafe_error& error) {
    TLOG(kRed) << boost::diagnostic_information(error) << "\n\n";
    return ErrorToInt(error);
  }
  catch (const std::exception& e) {
    TLOG(kRed) << e.what() << "\n\n";
    return ErrorToInt(MakeError(CommonErrors::unknown));
  }
}

void Menu::ShowOptions() const {
  TLOG(kDefaultColour) << current_item_->kName_ << '\n';
  current_item_->ShowChildrenNames();
}

}  // namespace maidsafe
