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

#ifndef MAIDSAFE_COMMON_PROFILER_H_
#define MAIDSAFE_COMMON_PROFILER_H_

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "boost/current_function.hpp"

#include "maidsafe/common/active.h"


#ifndef NDEBUG
#  define USE_PROFILING
#endif

namespace maidsafe {

namespace profile {

#ifdef USE_PROFILING
#  ifdef _MSC_VER
#    define SCOPED_PROFILE maidsafe::profile::ProfileEntry \
                           scoped_profile_entry(__FILE__, __LINE__, __FUNCTION__);
#  else
#    define SCOPED_PROFILE maidsafe::profile::ProfileEntry \
                           scoped_profile_entry(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION);
#  endif
#else
#  define SCOPED_PROFILE
#endif

struct ProfileEntry {
  struct Location {
    Location(const std::string& file_in, int line_in, const std::string& function_in);
    Location(const Location& other);
    Location(Location&& other);
    Location& operator=(Location other);

    std::string file;
    int line;
    std::string function;
  };

  ProfileEntry(const std::string &file, int line, const std::string &function)
      : location(file, line, function),
        start(std::chrono::high_resolution_clock::now()) {}

  ~ProfileEntry();

  Location location;  // location
  std::chrono::high_resolution_clock::time_point start;
};

void swap(ProfileEntry::Location& lhs, ProfileEntry::Location& rhs);

bool operator<(const ProfileEntry::Location& lhs, const ProfileEntry::Location& rhs);



class Profiler {
 public:
  static Profiler& Instance();
  ~Profiler();
  void AddEntry(ProfileEntry::Location&& location,
                const std::chrono::high_resolution_clock::duration& duration);

 private:
  typedef std::pair<uint64_t, std::chrono::high_resolution_clock::duration> EntryDetails;
  Profiler() : entries_(), background_(new Active) {}

  std::map<std::string, EntryDetails> entries_;
  std::shared_ptr<Active> background_;
};

}  // namespace profile

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_PROFILER_H_
