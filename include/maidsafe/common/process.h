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

#ifndef MAIDSAFE_COMMON_PROCESS_H_
#define MAIDSAFE_COMMON_PROCESS_H_

#ifdef MAIDSAFE_WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstdint>
#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/config.h"

namespace maidsafe {

namespace process {

typedef uint64_t ProcessId;

#ifdef MAIDSAFE_WIN32

std::wstring ConstructCommandLine(const std::vector<std::string>& process_args);

struct ManagedHandle {
  explicit ManagedHandle(HANDLE handle_in);
  ~ManagedHandle();
  HANDLE handle;
};

typedef ManagedHandle ProcessInfo;

bool IsRunning(HANDLE handle);

#else

typedef pid_t ProcessInfo;

std::string ConstructCommandLine(const std::vector<std::string>& process_args);

#endif

ProcessId GetProcessId();

bool IsRunning(const ProcessInfo& process_info);

// Returns the full path to an exe which is in the same dir as the currently-running exe.
boost::filesystem::path GetOtherExecutablePath(
    const boost::filesystem::path& name_without_extension);

}  // namespace process

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_PROCESS_H_
