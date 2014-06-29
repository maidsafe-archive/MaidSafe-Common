/*  Copyright 2013 MaidSafe.net limited

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

#include <streambuf>
#include <functional>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>

#include "maidsafe/common/cli.h"
#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/menu.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {


class CliTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
  test_value_ = 0;
  input_ = nullptr;
  original_cin_ = std::cin.rdbuf();
  } 
  virtual void TearDown() {
    std::cin.rdbuf(original_cin_); 
  }

  void SendToCin(std::string input) {
    input_ = maidsafe::make_unique<std::istringstream>(std::move(input));
    std::cin.rdbuf(input_->rdbuf());
  }

  int test_value_;

  std::streambuf* original_cin_;
  std::unique_ptr<std::istringstream> input_;
};

TEST_F(CliTest, BEH_GetTest) {
  SendToCin("input\n");
  CLI cli;
  std::string result{ cli.Get<std::string>("test") };
  EXPECT_TRUE(result == "input");
  result.clear();

  SendToCin("badinput\n");
  result = cli.Get<std::string>("test");
  EXPECT_TRUE(result != "input");
}

TEST_F(CliTest, BEH_MenuFunctions) {
  auto inc = [&] { ++test_value_; };
  auto dec = [&] { --test_value_; };

  Menu menu{ "Main" };
  menu.AddItem("Inc one", inc);
  menu.AddItem("Inc two", inc);
  menu.AddItem("Dec three", dec);

  SendToCin("1\n0\n");
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 1);

  SendToCin("1\n1\n0\n");
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 3);

  SendToCin("3\n3\n0\n");
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 1);
}

TEST_F(CliTest, BEH_MenuHierarchy) {
  Menu menu{ "Main" };
  menu.AddItem("Top level increment by 1", [&] { ++test_value_; });     // Action A
  menu.AddItem("Top level increment by 2", [&] { test_value_ += 2; });  // Action B
  menu.AddItem("Top level decrement by 1", [&] { --test_value_; });     // Action C

  MenuItem* sub_item{ menu.AddItem("Sub-menu") };
  sub_item->AddChildItem("Sub-menu increment by 100", [&] { test_value_ += 100; });  // Action D

  MenuItem* sub_sub_item{ sub_item->AddChildItem("Sub-sub-menu") };
  sub_sub_item->AddChildItem("Sub-sub-menu increment by 10,000", [&] { test_value_ += 10000; });    // Action E  NOLINT
  sub_sub_item->AddChildItem("Sub-sub-menu increment by 200,000", [&] { test_value_ += 200000; });  // Action F  NOLINT
  sub_sub_item->AddChildItem("Sub-sub-menu decrement by 3,000", [&] { test_value_ -= 3000; });      // Action G  NOLINT

  SendToCin("1\n0\n");  // A, Quit
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 1);

  SendToCin("1\n1\n0\n");  // A, A, Quit
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 3);

  SendToCin("1\n2\n3\n3\n0\n");  // A, B, C, C, Quit
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 4);

  SendToCin("4\n2\n99\n99\n0\n");  // Sub, Sub-sub, Back to Sub, Back to Main, Quit
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 4);

  // Sub, D, Sub-sub, E, F, G, Back to Sub, Back to Main, Quit
  SendToCin("4\n1\n2\n1\n2\n3\n99\n99\n0\n");
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 207104);

  SendToCin("0\n");  // Quit
  EXPECT_TRUE(menu.Run() == 0);
  EXPECT_TRUE(test_value_ == 207104);
}

TEST(CliTokenTest, BEH_TokenTest) {
  CLI cli;
  std::string test_str{ "this is five small tokens" };
  EXPECT_TRUE((cli.TokeniseLine(test_str).size()) == 5);
}

}  // namespace test

}  // namespace maidsafe
