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


#include <functional>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <streambuf>

#include "maidsafe/common/cli.h"
#include "maidsafe/common/menu.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace test {

TEST_CASE("TokenTest", "[cli][Unit]") {
  CLI cli;
  std::string  test_str("this is five small tokens");
  CHECK((cli.TokeniseLine(test_str).size()) == 5);
}


TEST_CASE("GetTest", "[cli][Unit]") {
  CLI cli;
  std::string result;
  // grab original cin as streambuf ptr
  std::streambuf* orig = std::cin.rdbuf();
  // redirect cin 
  std::istringstream input("input\n");
  std::cin.rdbuf(input.rdbuf());
  // test
  result = cli.Get<std::string>("test");
  CHECK(result == "input");
  result.clear(); 
  CHECK(result.empty());

  // redirect cin 
  std::istringstream input2("badinput\n");
  std::cin.rdbuf(input2.rdbuf());
  // test
  result = cli.Get<std::string>("test");
  CHECK(result != "input");
  // reset cin back to normal
  std::cin.rdbuf(orig);

}


TEST_CASE("MenuTest", "[cli][Unit]") {
  int test_value(0);
  auto inc = [&] () { ++test_value;};
  MenuLevel main("Main");
  MenuItem one("one", inc);
  MenuItem two("two", inc);
  MenuItem three("three", [&] () { --test_value;});
  // grab original cin as streambuf ptr
  std::streambuf* orig = std::cin.rdbuf();
  // redirect cin 
  std::istringstream input("1\n0\n");
  std::cin.rdbuf(input.rdbuf());

  Menu menu;
  menu.add_level(main);
  menu.add_item(one);
  menu.add_item(two);
  menu.add_item(three);
  menu.start_menu();
  CHECK(test_value == 1);
  std::cin.rdbuf(orig);

  // grab original cin as streambuf ptr
  orig = std::cin.rdbuf();
  // redirect cin 
  std::istringstream input2("1\n1\n0\n");
  std::cin.rdbuf(input2.rdbuf());
  menu.start_menu();
  CHECK(test_value == 3);  // 3 as we are updating a reference
  std::cin.rdbuf(orig);
  
  // grab original cin as streambuf ptr
  orig = std::cin.rdbuf();
  // redirect cin 
  std::istringstream input3("3\n3\n0\n");
  std::cin.rdbuf(input3.rdbuf());
  menu.start_menu();
  CHECK(test_value == 1);  // 3 as we are updating a reference
  std::cin.rdbuf(orig);
}


}  // namespace test

}  // namespace maidsafe
