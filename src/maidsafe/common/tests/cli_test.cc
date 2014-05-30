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
#include "maidsafe/common/menu.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

TEST_CASE("TokenTest", "[cli][Unit]") {
  CLI cli;
  std::string test_str{ "this is five small tokens" };
  CHECK((cli.TokeniseLine(test_str).size()) == 5);
}

TEST_CASE("GetTest", "[cli][Unit]") {
  // grab original cin as streambuf ptr
  std::streambuf* orig{ std::cin.rdbuf() };
  // redirect cin
  std::istringstream input{ "input\n" };
  std::cin.rdbuf(input.rdbuf());
  // test
  CLI cli;
  std::string result{ cli.Get<std::string>("test") };
  CHECK(result == "input");
  result.clear();

  // redirect cin
  std::istringstream input2{ "badinput\n" };
  std::cin.rdbuf(input2.rdbuf());
  // test
  result = cli.Get<std::string>("test");
  CHECK(result != "input");
  // reset cin back to normal
  std::cin.rdbuf(orig);
}

TEST_CASE("MenuFunctions", "[cli][Unit]") {
  int test_value{ 0 };
  auto inc = [&] { ++test_value; };
  auto dec = [&] { --test_value; };
  // grab original cin as streambuf ptr
  std::streambuf* orig{ std::cin.rdbuf() };
  // redirect cin
  std::istringstream input{ "1\n0\n" };
  std::cin.rdbuf(input.rdbuf());

  Menu menu{ "Main" };
  menu.AddItem("Inc one", inc);
  menu.AddItem("Inc two", inc);
  menu.AddItem("Dec three", dec);
  CHECK(menu.Run() == 0);
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);

  // grab original cin as streambuf ptr
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input2{ "1\n1\n0\n" };
  std::cin.rdbuf(input2.rdbuf());
  CHECK(menu.Run() == 0);
  CHECK(test_value == 3);  // 3 as we are updating a reference
  std::cin.rdbuf(orig);

  // grab original cin as streambuf ptr
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input3{ "3\n3\n0\n" };
  std::cin.rdbuf(input3.rdbuf());
  CHECK(menu.Run() == 0);
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);
}

/*
TEST_CASE("MenuHierarchy", "[cli][Unit]") {
  int test_value{ 0 };
  auto inc = [&] {
    ++test_value;
  };
  auto dec = [&] {
    --test_value;
  };
  std::string main{ "Main" };
  MenuItem one{ "one", inc }, two{ "two", inc }, three{ "three", dec };
  std::string sub{ "Sub" }, subsub{ "SubSub" };

  Menu menu;
  menu.AddLevel(main, main);
  menu.AddItem(one);
  menu.AddItem(two);
  menu.AddItem(three);

  menu.AddLevel(sub, main);
  menu.AddItem(one);

  menu.AddLevel(subsub, sub);
  menu.AddItem(three);
  menu.AddItem(one);
  menu.AddItem(three);

  // grab original cin as streambuf ptr
  std::streambuf* orig{ std::cin.rdbuf() };
  // redirect cin
  std::istringstream input{ "1\n0\n" };
  std::cin.rdbuf(input.rdbuf());
  menu.Start();
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);

  // grab original cin as streambuf ptr
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input2{ "1\n1\n0\n" };
  std::cin.rdbuf(input2.rdbuf());
  menu.Start();
  CHECK(test_value == 3);  // 3 as we are updating a reference
  std::cin.rdbuf(orig);

  // inc and dec to make sure we ar doing the right thing
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input3{ "3\n3\n0\n0\n" };
  std::cin.rdbuf(input3.rdbuf());
  menu.Start();
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);

  // check up and down heirarchy
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input4{ "4\n2\n99\n99\n0\n" };
  std::cin.rdbuf(input4.rdbuf());
  menu.Start();
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);

  // check running copied function ptr is OK
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input5{ "4\n2\n1\n99\n99\n0\n" };
  std::cin.rdbuf(input5.rdbuf());
  menu.Start();
  CHECK(test_value == 0);
  std::cin.rdbuf(orig);

  // just exit
  orig = std::cin.rdbuf();
  // redirect cin
  std::istringstream input6{ "0\n\r\n" };
  std::cin.rdbuf(input6.rdbuf());
  menu.Start();
  CHECK(test_value == 0);
  std::cin.rdbuf(orig);
}
*/

}  // namespace test

}  // namespace maidsafe
