/*  Copyright 2011 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_TEST_H_
#define MAIDSAFE_COMMON_TEST_H_

#include <functional>
#include <memory>
#include <string>

#include "boost/filesystem/path.hpp"

#if defined USE_GTEST

#include "gtest/gtest.h"

#elif defined USE_CATCH

#include "catch.hpp"

#endif

#include "maidsafe/common/log.h"

namespace maidsafe {

namespace test {

typedef std::shared_ptr<boost::filesystem::path> TestPath;

// Tries to create a unique directory in the temp directory and returns a shared
// pointer to the path of the new directory.  If the creation fails, or a temp
// directory cannot be found, the method returns a pointer to an empty path.  If
// a directory is successfully created, an attempt to delete it is made when the
// shared_ptr is destroyed by using CleanupTest (below) as a custom deleter.
// The test_prefix should preferably be "MaidSafe_Test<optional test name>" or
// "Sigmoid_Test<optional test name>".
TestPath CreateTestPath(std::string test_prefix = "");

// Executes "functor" asynchronously "thread_count" times.
void RunInParallel(int thread_count, std::function<void()> functor);

// Returns a random port in the range [1025, 65535].
uint16_t GetRandomPort();

namespace detail {

int ExecuteGTestMain(int argc, char* argv[]);
int ExecuteCatchMain(int argc, char* argv[]);

}  // namespace detail

#if defined USE_GTEST

inline int ExecuteMain(int argc, char* argv[]) {
  return detail::ExecuteGTestMain(argc, argv);
}

#elif defined USE_CATCH

inline int ExecuteMain(int argc, char* argv[]) {
  return detail::ExecuteCatchMain(argc, argv);
}

#endif

}  // namespace test

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TEST_H_
