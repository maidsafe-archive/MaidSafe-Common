/* Copyright 2011 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_TEST_H_
#define MAIDSAFE_COMMON_TEST_H_

#include <functional>
#include <memory>
#include <string>

#include "boost/filesystem/path.hpp"

#ifdef __MSVC__
#  pragma warning(push, 1)
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#ifdef __MSVC__
#  pragma warning(pop)
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

int ExecuteMain(int argc, char **argv);

}  // namespace test

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TEST_H_
