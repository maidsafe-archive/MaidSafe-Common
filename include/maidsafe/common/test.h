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

#if !defined(MAIDSAFE_WIN32) && !defined(__ANDROID__)
#include <ulimit.h>
#endif
#include <functional>
#include <memory>
#include <string>

#include "boost/filesystem/path.hpp"
#include "boost/optional.hpp"
#include "gtest/gtest.h"

#include "maidsafe/common/log.h"

namespace maidsafe {

namespace test {

typedef std::shared_ptr<boost::filesystem::path> TestPath;

// Tries to create a unique directory in the temp directory and returns a shared
// pointer to the path of the new directory.  If the creation fails, or a temp
// directory cannot be found, the method returns a pointer to an empty path.  If
// a directory is successfully created, an attempt to delete it is made when the
// shared_ptr is destroyed by using CleanupTest (below) as a custom deleter.
// The test_prefix should preferably be "MaidSafe_Test<optional test name>".
TestPath CreateTestPath(std::string test_prefix = "");

// Executes "functor" asynchronously "thread_count" times.
void RunInParallel(int thread_count, std::function<void()> functor);

// Returns a random port in the range [1025, 65535].
uint16_t GetRandomPort();

std::string GetRandomIPv4AddressAsString();

std::string GetRandomIPv6AddressAsString();

#ifdef TESTING

// Allows:
// * the random number generator in 'utils.cc' to be seeded via the command line (makes the
//   functions 'RandomInt32', 'RandomUint32', 'RandomString', and 'RandomAlphaNmericString' produce
//   deterministic output).
// * an initial delay to be invoked at the start of 'main' (useful for pausing a process which
//   is started as a child of another, but which needs to be attached to a debugger as soon as
//   possible).
// * passing an override bootstrap file path to allow tests to join a testnet.
void HandleTestOptions(int argc, char* argv[]);

// This GTest listener seeds the random number generator in 'utils.cc', and outputs the current seed
// for any failing test.
class RandomNumberSeeder : public testing::EmptyTestEventListener {
 public:
  RandomNumberSeeder();

 private:
  virtual void OnTestStart(const testing::TestInfo& test_info);
  virtual void OnTestEnd(const testing::TestInfo& test_info);
  uint32_t current_seed_;
};
#ifndef MAIDSAFE_WIN32
// This GTest listener check the ulimit configurations.
class UlimitConfigurer : public testing::EmptyTestEventListener {
 public:
  UlimitConfigurer();

 private:
  virtual void OnTestProgramStart(const testing::UnitTest& uinit_test);
  virtual void OnTestProgramEnd(const testing::UnitTest& uint_testinfo);
  uint32_t prev_open_files_;
  uint32_t prev_file_size_;
  uint32_t kLimitsOpenFiles;
  uint32_t kLimitsFileSize;
};
#endif
// Copies 'bootstrap_file' to location of Routing's override_bootstrap.dat, allowing all Routing
// objects to bootstrap from the contacts provided in 'bootstrap_file'.  Throws if the file can't be
// copied.
void PrepareBootstrapFile(boost::filesystem::path bootstrap_file);

// If '--bootstrap_file <path>' has been passed, returns the path, otherwise the optional returned
// is false (uninitialised).  The function is not thread-safe.
boost::optional<boost::filesystem::path> GetBootstrapFilePath();

// This GTest listener calls 'PrepareBootstrapFile' (see above) at the start of every test if
// GetBootstrapFilePath()'s optional return is true (i.e. if '--bootstrap_file <path>' has been
// passed).
class BootstrapFileHandler : public testing::EmptyTestEventListener {
 private:
  virtual void OnTestStart(const testing::TestInfo& test_info);
};

#endif

namespace detail {

int ExecuteGTestMain(int argc, char* argv[]);

}  // namespace detail

inline int ExecuteMain(int argc, char* argv[]) { return detail::ExecuteGTestMain(argc, argv); }

}  // namespace test

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TEST_H_
