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

#include "maidsafe/common/process.h"

#include <iterator>
#include <numeric>

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace process {

namespace {

std::string ConcatenateArgs(const std::vector<std::string>& args) {
  return std::accumulate(std::begin(args), std::end(args), std::string(""),
      [](const std::string& lhs, const std::string& rhs) {
        return lhs.empty() ? (rhs.empty() ? "" : rhs) : (rhs.empty() ? lhs : lhs + " " + rhs);
      });
}

}  // unnamed namespace

#ifdef MAIDSAFE_WIN32

std::wstring ConstructCommandLine(const std::vector<std::string>& process_args) {
  return StringToWstring(ConcatenateArgs(process_args));
}

#else

std::string ConstructCommandLine(const std::vector<std::string>& process_args) {
  return ConcatenateArgs(process_args);
}

#endif

boost::filesystem::path GetOtherExecutablePath(
    const boost::filesystem::path& name_without_extension) {
  return (ThisExecutableDir() /
          name_without_extension).replace_extension(ThisExecutablePath().extension());
}

}  // namespace process

}  // namespace maidsafe
