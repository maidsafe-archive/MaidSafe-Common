/*  Copyright 2014 MaidSafe.net limited

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


#include "maidsafe/common/tools/address_space_tool.h"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <string>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace tools {

const std::string kDefaultConfigFilename{"address_space_tool.conf"};

int Test::Accumulate(std::vector<Node>::const_iterator first,
                     std::vector<Node>::const_iterator last, const NodeId& target, int& highest,
                     int& lowest) const {
  return std::accumulate(first, last, 0, [&](int running_total, const Node& node) -> int {
    int common_leading_bits{node.id.CommonLeadingBits(target)};
    if (common_leading_bits > highest)
      highest = common_leading_bits;
    if (common_leading_bits < lowest)
      lowest = common_leading_bits;
    return running_total + common_leading_bits;
  });
}

int Test::CommonLeadingBits(int highest, int lowest, int sum, int count) const {
  switch (config_.algorithm) {
    case CommonLeadingBitsAlgorithm::kHighest:
      return highest;
    case CommonLeadingBitsAlgorithm::kLowest:
      return lowest;
    case CommonLeadingBitsAlgorithm::kMean:
      return sum / count;
    default:
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
}

int Test::GroupCommonLeadingBits(size_t group_size) const {
  if (config_.algorithm == CommonLeadingBitsAlgorithm::kClosest)
    return all_nodes_[0].id.CommonLeadingBits(all_nodes_[1].id);
  if (all_nodes_.size() == 1)
    return 0;

  int sum{0}, count{0}, highest{0}, lowest{512};
  auto itr(std::begin(all_nodes_));
  const auto end_itr(std::begin(all_nodes_) + group_size);
  while (itr != end_itr - 1) {
    sum += Accumulate(itr + 1, end_itr, itr->id, highest, lowest);
    ++itr;
    count += static_cast<int>(std::distance(itr, end_itr));
  }
  return CommonLeadingBits(highest, lowest, sum, count);
}

int Test::CandidateCommonLeadingBits(const NodeId& candidate_node, size_t group_size) const {
  if (config_.algorithm == CommonLeadingBitsAlgorithm::kClosest)
    return all_nodes_[0].id.CommonLeadingBits(candidate_node);

  int highest{0}, lowest{512};
  const auto end_itr(std::begin(all_nodes_) + group_size);
  int sum{Accumulate(std::begin(all_nodes_), end_itr, candidate_node, highest, lowest)};
  int count{static_cast<int>(std::distance(std::begin(all_nodes_), end_itr))};
  return CommonLeadingBits(highest, lowest, sum, count);
}

void Test::UpdateRank(size_t group_size) {
  std::for_each(std::begin(all_nodes_), std::begin(all_nodes_) + group_size, [](Node& node) {
    node.rank = std::min(node.rank + (RandomInt32() % 20) + 10, 100);
  });
}

std::pair<int, int> Test::RankValues(size_t group_size) const {
  int close(0);
  int proximity(0);

  std::for_each(std::begin(all_nodes_), std::begin(all_nodes_) + group_size,
                [&close](const Node& node) { close += node.rank; });
  std::for_each(std::begin(all_nodes_), std::begin(all_nodes_) + (group_size * 4),
                [&proximity](const Node& node) { proximity += node.rank; });
  return {static_cast<int>(close / group_size), static_cast<int>(proximity / (group_size * 4))};
}

bool Test::RankAllowed(size_t group_size) const {
  auto rank(RankValues(group_size));
  return rank.first > rank.second;
}

void Test::DoAddNode(const NodeId& node_id, bool good, int attempts) {
  all_nodes_.emplace_back(node_id, good);
  LOG(kInfo) << "Added a " << (good ? "good" : "bad") << " node after " << attempts
             << " attempt(s) in a network of size " << all_nodes_.size() << '.';
  total_attempts_ += attempts;
  good ? ++good_count_ : ++bad_count_;
}

void Test::AddNode(bool good) {
  size_t group_size{std::min(static_cast<size_t>(config_.group_size), all_nodes_.size())};
  int attempts{0};
  for (;;) {
    ++attempts;
    NodeId node_id(RandomString(NodeId::kSize));
    std::partial_sort(std::begin(all_nodes_), std::begin(all_nodes_) + group_size,
                      std::end(all_nodes_), [&node_id](const Node& lhs, const Node& rhs) {
      return NodeId::CloserToTarget(lhs.id, rhs.id, node_id);
    });
    UpdateRank(group_size);
    if (all_nodes_.size() > (config_.group_size * 4) && !RankAllowed(group_size))
      continue;

    if (config_.algorithm == CommonLeadingBitsAlgorithm::kNone)
      return DoAddNode(node_id, good, attempts);

    int group_common_leading_bits{GroupCommonLeadingBits(group_size)};
    int candidate_common_leading_bits{CandidateCommonLeadingBits(node_id, group_size)};
    if (candidate_common_leading_bits <
        static_cast<int>(group_common_leading_bits + config_.leeway))
      return DoAddNode(node_id, good, attempts);
  }
}

void Test::InitialiseNetwork() {
  all_nodes_.clear();
  all_nodes_.reserve(config_.initial_good_count);
  // Add first node
  DoAddNode(NodeId(RandomString(NodeId::kSize)), true, 1);
  // Add others
  for (size_t i(1); i < config_.initial_good_count; ++i)
    AddNode(true);
  std::string output{"Added "};
  output += std::to_string(config_.initial_good_count) + " good nodes";
  if (config_.algorithm != CommonLeadingBitsAlgorithm::kNone) {
    output += ", averaging ";
    output += std::to_string(static_cast<double>(total_attempts_) / all_nodes_.size());
    output += " attempt(s) each.";
  } else {
    output += '.';
  }
  LOG(kSuccess) << output;
  total_attempts_ = 0;
}

std::vector<NodeId> Test::GetUniformlyDistributedTargetPoints() const {
  const size_t kStepCount(1024);
  std::vector<NodeId> steps;
  steps.reserve(kStepCount);
  crypto::BigInt step_size(
      (NodeId(std::string(NodeId::kSize, -1)).ToStringEncoded(NodeId::EncodingType::kHex) + "h")
          .c_str());
  step_size /= kStepCount;
  crypto::BigInt step(0l);
  for (size_t i(0); i < kStepCount; ++i) {
    std::string output(64, '\0');
    step.Encode(reinterpret_cast<byte*>(const_cast<char*>(output.c_str())), 64);
    steps.emplace_back(output);
    step += step_size;
  }
  LOG(kSuccess) << "Created " << kStepCount << " evenly-distributed target addresses.";
  return steps;
}

BadGroup Test::GetBadGroup(const NodeId& target_id) const {
  std::vector<Node> bad_group{config_.group_size};
  // Get close group
  std::partial_sort_copy(std::begin(all_nodes_), std::end(all_nodes_), std::begin(bad_group),
                         std::end(bad_group), [&target_id](const Node& lhs, const Node& rhs) {
    return NodeId::CloserToTarget(lhs.id, rhs.id, target_id);
  });
  auto is_bad([](const Node& node) { return !node.good; });
  // Count bad nodes in close group and return the group if majority are bad
  if (static_cast<size_t>(std::count_if(std::begin(bad_group), std::end(bad_group), is_bad)) >=
      config_.majority_size) {
    std::sort(std::begin(bad_group), std::end(bad_group));
  } else {
    bad_group.clear();
  }
  return std::make_pair(target_id, std::move(bad_group));
}

std::vector<BadGroup> Test::InjectBadGroups(const std::vector<NodeId>& steps) {
  LOG(kSuccess) << "Adding bad nodes and checking for compromised groups...";
  std::vector<BadGroup> bad_groups;
  while (bad_groups.size() < config_.bad_group_count) {
    for (size_t i(0); i < config_.good_added_per_bad; ++i)
      AddNode(true);

    bad_groups.clear();
    AddNode(false);
    // Iterate through evenly-spread target IDs
    for (const auto& target_id : steps) {
      auto new_bad_group(GetBadGroup(target_id));
      if (!new_bad_group.second.empty()) {
        // Only add if none of the bad nodes are already in a bad group
        bool should_add(true);
        for (const auto& existing_bad_group : bad_groups) {
          std::vector<Node> intersection;
          std::set_intersection(std::begin(existing_bad_group.second),
                                std::end(existing_bad_group.second),
                                std::begin(new_bad_group.second), std::end(new_bad_group.second),
                                std::back_inserter(intersection));
          should_add &= intersection.empty();
        }
        if (should_add)
          bad_groups.emplace_back(std::move(new_bad_group));
      }
    }
  }

  TLOG(kRed) << "For a network of " << config_.initial_good_count << " got "
             << config_.bad_group_count << " bad group(s) after adding " << bad_count_
             << " bad nodes and " << good_count_ - config_.initial_good_count << " good nodes";
  if (config_.algorithm != CommonLeadingBitsAlgorithm::kNone) {
    TLOG(kRed) << ", averaging "
               << static_cast<double>(total_attempts_) /
                      (all_nodes_.size() - config_.initial_good_count) << " attempt(s) each";
  }
  TLOG(kRed) << ".  Network population = " << all_nodes_.size()
             << "  Attack = " << static_cast<double>(bad_count_) * 100 / all_nodes_.size()
             << "%.\n";
  return bad_groups;
}

void Test::ReportBadGroups(const std::vector<BadGroup>& bad_groups) const {
  for (size_t i(0); i < bad_groups.size(); ++i) {
    LOG(kInfo) << "Bad group " << i << " close to target " << bad_groups[i].first;
    for (const auto& node : bad_groups[i].second) {
      if (node.good)
        LOG(kSuccess) << node;
      else
        LOG(kWarning) << node;
    }
  }
}

void Test::CheckLinkedAddresses() const {
  if (!config_.total_random_attempts)
    return;

  LOG(kSuccess) << "Checking linked random addresses...";
  size_t attempts(0), compromised_attempts(0);
  std::vector<BadGroup> bad_groups;
  while (attempts < config_.total_random_attempts) {
    bad_groups.clear();
    ++attempts;
    NodeId target_id(RandomString(NodeId::kSize));
    for (size_t i(0); i < config_.bad_group_count; ++i) {
      if (i > 0)  // Hash previous target to get new linked one
        target_id = NodeId(crypto::Hash<crypto::SHA512>(target_id.string()).string());
      auto bad_group(GetBadGroup(target_id));
      if (bad_group.second.empty())  // Not a bad group - start a new attempt
        break;
      bad_groups.emplace_back(std::move(bad_group));
    }
    if (bad_groups.size() == config_.bad_group_count) {
      ++compromised_attempts;
      LOG(kError) << "Got bad group chain of " << config_.bad_group_count << " after " << attempts
                  << " linked random ID attempts.";
      ReportBadGroups(bad_groups);
    }
  }
  std::string output{
      std::to_string(compromised_attempts) + " out of " +
      std::to_string(config_.total_random_attempts) +
      " linked random addresses were fully managed by compromised close groups.\n\n"};
  if (compromised_attempts)
    TLOG(kRed) << output;
  else
    TLOG(kGreen) << output;
}

void Test::Run() {
  InitialiseNetwork();
  auto steps(GetUniformlyDistributedTargetPoints());
  auto bad_groups(InjectBadGroups(steps));
  ReportBadGroups(bad_groups);
  CheckLinkedAddresses();
}



bool IsHelpOption(const std::vector<std::string>& unused_options) {
  return std::any_of(
      std::begin(unused_options), std::end(unused_options),
      [](const std::string& option) { return option == "--help" || option == "-h"; });
}

Config GetConfig(const std::vector<std::string>& unused_options) {
  fs::path config_path;
  if (unused_options.empty())
    config_path = ThisExecutableDir() / kDefaultConfigFilename;
  else
    config_path = unused_options.front();

  Config config;
  if (fs::exists(config_path)) {
    std::ifstream ifstream{config_path.string()};
    cereal::JSONInputArchive input_archive{ifstream};
    input_archive(CEREAL_NVP(config));
  } else {
    config_path = ThisExecutableDir() / kDefaultConfigFilename;
    std::ofstream ofstream{config_path.string()};
    cereal::JSONOutputArchive output_archive{ofstream};
    Config config;
    output_archive(CEREAL_NVP(config));
    LOG(kInfo) << "Wrote config file to " << config_path;
  }
  return config;
}

}  // namespace tools

}  // namespace maidsafe

int main(int argc, char* argv[]) {
  auto unuseds(maidsafe::log::Logging::Instance().Initialise(argc, argv));
  std::vector<std::string> unused_options;
  for (const auto& unused : unuseds)
    unused_options.emplace_back(&unused[0]);
  // skip the first arg which is the path to this tool
  unused_options.erase(std::begin(unused_options));

  if (unuseds.size() > 1 || maidsafe::tools::IsHelpOption(unused_options)) {
    TLOG(kYellow) << "This tool should be invoked with logging arguments, and an optional path to "
                     "a config file.\nIf no config file path is provided, the tool will look for "
                     "one named " << maidsafe::tools::kDefaultConfigFilename
                  << "\nin the same folder as this executable, i.e. \n"
                  << maidsafe::ThisExecutableDir() / maidsafe::tools::kDefaultConfigFilename
                  << "\nIf it doesn't find this, it will be created using default configuration "
                     "values at this location.\n\n";
    return -1;
  }

  try {
    maidsafe::tools::Config config{maidsafe::tools::GetConfig(unused_options)};
    TLOG(kDefaultColour) << "Config values:\n" << config;

    for (size_t i(0); i < config.iterations; ++i) {
      LOG(kSuccess) << "\nRunning iteration " << i << " with config values:\n" << config;
      maidsafe::tools::Test(config).Run();
      config.initial_good_count = static_cast<size_t>(
          static_cast<double>(config.initial_good_count) * config.initial_factor);
    }
  } catch (const std::exception& e) {
    TLOG(kRed) << "Failed: " << e.what() << '\n';
    return -2;
  }
  return 0;
}
