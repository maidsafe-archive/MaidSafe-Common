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

#include "maidsafe/common/profiler.h"

#include <cstdio>
#include <future>
#include <iostream>
#include <iterator>

#include "boost/algorithm/string/replace.hpp"

#include "maidsafe/common/log.h"


namespace maidsafe {

namespace profile {

namespace {

std::string LocationToString(const ProfileEntry::Location& location) {
  auto result(location.file);
  boost::replace_all(result, "\\", "/");
  size_t position(result.rfind("maidsafe"));
  if (position != std::string::npos && position != 0)
    result = result.substr(position + 9);
  result += ":" + std::to_string(location.line) + "] " + location.function;
  return result;
}

std::string DurationToString(const std::chrono::high_resolution_clock::duration& duration) {
  long long nanos(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());  // NOLINT
  std::vector<char> buffer(21, 0);
  std::sprintf(&buffer[0], "%8lli.%09lli s", nanos / 1000000000, nanos % 1000000000);  // NOLINT
  return std::string(&buffer[0]);
}

void AppendInfo(
    const std::pair<std::string,
                    std::pair<uint64_t, std::chrono::high_resolution_clock::duration>>& entry,
    std::string& output) {
  const auto& count(entry.second.first);
  const auto& total_duration(entry.second.second);
  output += entry.first + "\n  Called:                   " + std::to_string(count) + " times\n";
  output += "  Average duration:  " + DurationToString(total_duration / count) + "\n";
  output += "  Total duration:    " + DurationToString(total_duration) + "\n\n";
}

}  // unnamed namespace

ProfileEntry::Location::Location(const std::string& file_in,
                                 int line_in,
                                 const std::string& function_in)
    : file(file_in),
      line(line_in),
      function(function_in) {}

ProfileEntry::Location::Location(const Location& other)
    : file(other.file),
      line(other.line),
      function(other.function) {}

ProfileEntry::Location::Location(Location&& other)
    : file(std::move(other.file)),
      line(std::move(other.line)),
      function(std::move(other.function)) {}

ProfileEntry::Location& ProfileEntry::Location::operator=(Location other) {
  swap(*this, other);
  return *this;
}

void swap(ProfileEntry::Location& lhs, ProfileEntry::Location& rhs) {
  using std::swap;
  swap(lhs.file, rhs.file);
  swap(lhs.line, rhs.line);
  swap(lhs.function, rhs.function);
}

bool operator<(const ProfileEntry::Location& lhs, const ProfileEntry::Location& rhs) {
  return std::tie(lhs.file, lhs.line, lhs.function) < std::tie(rhs.file, rhs.line, rhs.function);
}



ProfileEntry::~ProfileEntry() {
  Profiler::Instance().AddEntry(std::move(location),
                                std::chrono::high_resolution_clock::now() - start);
}

Profiler& Profiler::Instance() {
  static Profiler profiler;
  return profiler;
}

Profiler::~Profiler() {
  std::cout << "Preparing profiler output";
  auto output_future(std::async(std::launch::async,
                                std::bind(static_cast<void(std::shared_ptr<Active>::*)()>(
                                          &std::shared_ptr<Active>::reset), background_)));
  background_.reset();
  while (output_future.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
    std::cout << '.';
  }
  std::string output("\nSorted by name\n==============\n\n");
  for (const auto& entry : entries_)
    AppendInfo(entry, output);
  std::cout << output << "\n\n";

  typedef std::vector<std::pair<std::string, EntryDetails>> Entries;
  Entries entries(std::begin(entries_), std::end(entries_));
  entries_.clear();

  output.assign("\nSorted by call count\n====================\n\n");
  std::sort(std::begin(entries),
            std::end(entries),
            [](const Entries::value_type& lhs, const Entries::value_type& rhs) {
              return lhs.second.first > rhs.second.first;
            });
  for (const auto& entry : entries)
    AppendInfo(entry, output);
  std::cout << output << "\n\n";

  output.assign("\nSorted by average duration\n==========================\n\n");
  std::sort(std::begin(entries),
            std::end(entries),
            [](const Entries::value_type& lhs, const Entries::value_type& rhs) {
              return lhs.second.second / lhs.second.first > rhs.second.second / rhs.second.first;
            });
  for (const auto& entry : entries)
    AppendInfo(entry, output);
  std::cout << output << "\n\n";

  output.assign("\nSorted by total duration\n========================\n\n");
  std::sort(std::begin(entries),
            std::end(entries),
            [](const Entries::value_type& lhs, const Entries::value_type& rhs) {
              return lhs.second.second > rhs.second.second;
            });
  for (const auto& entry : entries)
    AppendInfo(entry, output);
  std::cout << output << "\n\n";
}

void Profiler::AddEntry(ProfileEntry::Location&& location,
                        const std::chrono::high_resolution_clock::duration& duration) {
  background_->Send([this, location, duration] {
    auto location_as_string(LocationToString(location));
    auto itr(entries_.find(location_as_string));
    if (itr == std::end(entries_)) {
      entries_.insert(itr, std::make_pair(std::move(location_as_string),
                                          std::make_pair(1, duration)));
    } else {
      ++itr->second.first;
      itr->second.second += duration;
    }
  });
}

}  // namespace profile

}  // namespace maidsafe
