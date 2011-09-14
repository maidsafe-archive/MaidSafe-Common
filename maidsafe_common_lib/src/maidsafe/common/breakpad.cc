/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/breakpad.h"

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/log.h"
#include "maidsafe/common/platform_config.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace crash_report {

#ifdef WIN32
  bool DumpCallback(const wchar_t* dump_path,
                    const wchar_t* minidump_id,
                    void* context,
                    EXCEPTION_POINTERS* /*exinfo*/,
                    MDRawAssertionInfo* /*assertion*/,
                    bool succeeded) {
    ProjectInfo* project = reinterpret_cast<ProjectInfo*>(context);
    std::wstring full_dump_name = dump_path;
    full_dump_name += L"\\";
    full_dump_name += minidump_id;
    full_dump_name += L".dmp";
    fs::path full_dump_path(full_dump_name);
    int current_modulepath_length = 0;
    int max_path_length = MAX_PATH;
    TCHAR *current_path = new TCHAR[max_path_length];
    while (current_modulepath_length <= max_path_length) {
      current_modulepath_length = GetModuleFileName(
                                          NULL, current_path, max_path_length);
      if (current_modulepath_length >= max_path_length) {
        max_path_length *= 2;
        delete [] current_path;
        current_path = new TCHAR[max_path_length];
      } else if (current_modulepath_length == 0) {
        DLOG(ERROR) << "Cannot Retrieve Current Path";
        break;
      } else {
        break;
      }
    }
    std::string current_directory(
                              fs::path(current_path).parent_path().string());
    delete [] current_path;
    if (fs::is_regular_file(current_directory + "\\CrashReporter.exe")) {
      std::string command = current_directory + "\\CrashReporter.exe " +
                            full_dump_path.string() + " " + project->name +
                            " " +
                            boost::lexical_cast<std::string>(project->version);
      std::system(command.c_str());
    } else {
      DLOG(ERROR) << "Crash Reporter Not Found.";
    }
    return succeeded;
  }
#else
  static bool DumpCallback(const char* dump_path,
                           const char* minidump_id,
                           void* context,
                           bool succeeded) {
    ProjectInfo* project = reinterpret_cast<ProjectInfo*>(context);
    std::string full_dump_name = dump_path;
    full_dump_name += "/";
    full_dump_name += minidump_id;
    full_dump_name += ".dmp";
    int current_modulepath_length = 0;
    int max_path_length = PATH_MAX;
    char *current_path = new char[max_path_length];
    while (current_modulepath_length <= max_path_length) {
      current_modulepath_length = readlink("/proc/self/exe",
                                            current_path, PATH_MAX);
      if (current_modulepath_length >= max_path_length) {
        max_path_length *= 2;
        delete [] current_path;
        current_path = new char[max_path_length];
      } else if (current_modulepath_length == 0) {
        DLOG(ERROR) << "Cannot Retrieve Current Path";
        break;
      } else {
        break;
      }
    }
    std::string current_directory(
                              fs::path(current_path).parent_path().string());
    delete [] current_path;
    if (fs::is_regular_file(current_directory + "/CrashReporter")) {
      std::string command = current_directory + "/CrashReporter "
                                          + full_dump_name;
      std::system(command.c_str());
    } else if (fs::is_regular_file(current_directory + "/CrashReporter")) {
      std::string command = current_directory + "/CrashReporter " +
                            full_dump_name + " " + project->name + " " +
                            boost::lexical_cast<std::string>(project->version);
      std::system(command.c_str());
    } else {
      DLOG(ERROR) << "Crash Reporter Not Found.";
    }
    return succeeded;
  }
#endif

ProjectInfo::ProjectInfo(std::string project_name,
                         std::string project_version)
    : name(project_name),
      version(project_version) {}

}  // namespace crypto

}  // namespace maidsafe
