/*  Copyright 2012 MaidSafe.net limited

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

#include <chrono>
#include <string>
#include <thread>

#include "maidsafe/common/log.h"
#include "maidsafe/common/menu.h"

#include "maidsafe/common/menu_item.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/common/tools/sqlite3_wrapper_benchmark.h"

int main(int argc, char* argv[]) {
  maidsafe::log::Logging::Instance().Initialise(argc, argv);

  maidsafe::Menu menu{ "Main menu" };

  // QA
  maidsafe::MenuItem* qa_item{ menu.AddItem("QA (stress tests, dynamic and static analysis)") };

  maidsafe::MenuItem* qa_dev_item{ qa_item->AddChildItem("Developer's Menu (core dev help)") };

  maidsafe::MenuItem* qa_dev_test_item{ qa_dev_item->AddChildItem("Test Suite") };
  qa_dev_test_item->AddChildItem("Test 1", [] {
    TLOG(kGreen) << "Running dev test 1.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });
  qa_dev_test_item->AddChildItem("Test 2", [] {
    TLOG(kGreen) << "Running dev test 2.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  maidsafe::MenuItem* qa_dev_bench_item{ qa_dev_item->AddChildItem("Benchmark Suite") };
  qa_dev_bench_item->AddChildItem("sqlite_wrapper benchmark", [] {
    TLOG(kGreen) << "Running sqlite_wrapper benchmark test\n";
    maidsafe::benchmark::Sqlite3WrapperBenchmark sqlite_wrapper_benchmark_test;
    sqlite_wrapper_benchmark_test.Run();
  });
  qa_dev_bench_item->AddChildItem("Benchmark 2", [] {
    TLOG(kGreen) << "Running benchmark 2.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  // Builders
  maidsafe::MenuItem* builders_item{ menu.AddItem("Builder's Menu (includes tools and examples)") };
  maidsafe::MenuItem* builders_examples_item{ builders_item->AddChildItem("Examples") };
  builders_examples_item->AddChildItem("Example 1", [] {
    TLOG(kGreen) << "Running builder's example 1.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });
  builders_examples_item->AddChildItem("Example 2", [] {
    TLOG(kGreen) << "Running builder's example 2.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  maidsafe::MenuItem* builders_tools_item{ builders_item->AddChildItem("Tools") };
  builders_tools_item->AddChildItem("Tool 1", [] {
    TLOG(kGreen) << "Running builder's tool 1.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });
  builders_tools_item->AddChildItem("Tool 2", [] {
    TLOG(kGreen) << "Running builder's tool 2.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  // Suitability tests
  menu.AddItem("Suitability Tests (check your setup and farming ability)", [] {
    TLOG(kGreen) << "Running setup and farming test.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  return menu.Run();
}

