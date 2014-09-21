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

#include <algorithm>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/common/log.h"
#include "maidsafe/common/node_id.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

struct Node {
  Node(NodeId id_in, bool good_in) : id(std::move(id_in)), good(good_in), rank(0) {}

  NodeId id;
  bool good;
  int rank;
};

bool operator<(const Node& lhs, const Node& rhs) { return lhs.id < rhs.id; }

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,
                                             const Node& node) {
  ostream << node.id << (node.good ? " (good node)" : " (bad node) ") << " with rank " << node.rank;
  return ostream;
}

typedef std::pair<NodeId, std::vector<Node>> BadGroup;

enum class CommonLeadingBitsAlgorithm { kHighest, kLowest, kMean, kNone };

size_t g_good_count{1000}, g_group_size{4}, g_majority_size{3}, g_bad_group_count{2},
    g_total_random_attempts{1000000}, g_leeway{2};
std::vector<Node> all_nodes;
CommonLeadingBitsAlgorithm g_algorithm{CommonLeadingBitsAlgorithm::kLowest};
size_t g_total_attempts{0};

void GetChoice(std::string input_text, size_t& value) {
  input_text = "\nEnter " + input_text + " (default " + std::to_string(value) +
               " - hit enter to use default): ";
  std::string input;
  int choice(-1);
  for (;;) {
    TLOG(kDefaultColour) << input_text;
    std::getline(std::cin, input);
    if (input.empty()) {
      TLOG(kDefaultColour) << value;
      return;
    }
    try {
      choice = std::stoi(input);
    } catch (const std::exception&) {
    }

    if (choice >= 0) {
      TLOG(kDefaultColour) << choice;
      break;
    }
  }
  value = static_cast<size_t>(choice);
}

void GetValues() {
  GetChoice("initial network size", g_good_count);
  GetChoice("close group size", g_group_size);
  GetChoice("majority size", g_majority_size);
  GetChoice("target number of compromised groups", g_bad_group_count);
  GetChoice("number of random attempts", g_total_random_attempts);
  size_t algorithm{0};
  GetChoice(
      "leading bits algorithm: '0' for none, '1' for highest, '2' for lowest, or '3' "
      "for mean",
      algorithm);
  g_algorithm = static_cast<CommonLeadingBitsAlgorithm>(algorithm);
  if (g_algorithm != CommonLeadingBitsAlgorithm::kNone)
    GetChoice("leeway of common leading bits", g_leeway);
  TLOG(kDefaultColour) << '\n';
}

int Accumulate(std::vector<Node>::iterator first, std::vector<Node>::iterator last,
               const NodeId& target, int& highest, int& lowest) {
  return std::accumulate(first, last, 0, [&](int running_total, const Node& node) -> int {
    int common_leading_bits{node.id.CommonLeadingBits(target)};
    if (common_leading_bits > highest)
      highest = common_leading_bits;
    if (common_leading_bits < lowest)
      lowest = common_leading_bits;
    return running_total + common_leading_bits;
  });
}

int CommonLeadingBits(int highest, int lowest, int sum, int count) {
  switch (g_algorithm) {
    case CommonLeadingBitsAlgorithm::kHighest:
      return highest;
    case CommonLeadingBitsAlgorithm::kLowest:
      return lowest;
    case CommonLeadingBitsAlgorithm::kMean:
      return sum / count;
    default:
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
}

// Requires first 'group_size' entries of 'all_nodes' to be sorted by closeness to target.
int GroupCommonLeadingBits(size_t group_size) {
  int sum{0}, count{0}, highest{0}, lowest{512};
  auto itr(std::begin(all_nodes));
  const auto end_itr(std::begin(all_nodes) + group_size);
  while (itr != end_itr - 1) {
    sum += Accumulate(itr + 1, end_itr, itr->id, highest, lowest);
    ++itr;
    count += static_cast<int>(std::distance(itr, end_itr));
  }
  return CommonLeadingBits(highest, lowest, sum, count);
}

// Requires first 'group_size' entries of 'all_nodes' to be sorted by closeness to 'candidate_node'.
int CandidateCommonLeadingBits(const NodeId& candidate_node, size_t group_size) {
  int highest{0}, lowest{512};
  const auto end_itr(std::begin(all_nodes) + group_size);
  int sum{Accumulate(std::begin(all_nodes), end_itr, candidate_node, highest, lowest)};
  int count{static_cast<int>(std::distance(std::begin(all_nodes), end_itr))};
  return CommonLeadingBits(highest, lowest, sum, count);
}

void UpdateRank(size_t group_size) {
  std::for_each(std::begin(all_nodes), std::begin(all_nodes) + group_size, [](Node& node) {
    node.rank = std::min(node.rank + (RandomInt32() % 20) + 10, 100);
  });
}


std::pair<int, int> RankValues(size_t group_size) {
  int close(0);
  int proximity(0);

  std::for_each(std::begin(all_nodes), std::begin(all_nodes) + group_size,
                [&close](const Node& node) { close += node.rank; });
  std::for_each(std::begin(all_nodes), std::begin(all_nodes) + (group_size * 4),
                [&proximity](const Node& node) { proximity += node.rank; });
  return {close / group_size, proximity / (group_size * 4)};
}

bool RankAllowed(size_t group_size) {
  auto rank(RankValues(group_size));
  return rank.first > rank.second;
}

void DoAddNode(const NodeId& node_id, bool good, int attempts) {
  all_nodes.emplace_back(node_id, good);
  LOG(kInfo) << "Added a " << (good ? "good" : "bad") << " node after " << attempts
             << " attempt(s) in a network of size " << all_nodes.size() << "\n";
  g_total_attempts += attempts;
}

void AddNode(bool good) {
  size_t group_size{std::min(g_group_size, all_nodes.size())};
  int attempts{0};
  for (;;) {
    ++attempts;
    NodeId node_id(NodeId::IdType::kRandomId);
    std::partial_sort(std::begin(all_nodes), std::begin(all_nodes) + group_size,
                      std::end(all_nodes), [&node_id](const Node& lhs, const Node& rhs) {
      return NodeId::CloserToTarget(lhs.id, rhs.id, node_id);
    });
    UpdateRank(group_size);
    if (all_nodes.size() > (g_group_size * 4) && !RankAllowed(group_size))
      continue;

    if (g_algorithm == CommonLeadingBitsAlgorithm::kNone)
      return DoAddNode(node_id, good, attempts);

    // int group_common_leading_bits{ GroupCommonLeadingBits(group_size) };
    // int candidate_common_leading_bits{ CandidateCommonLeadingBits(node_id, group_size) };
    if (all_nodes[0].id.CommonLeadingBits(node_id) >
            all_nodes[0].id.CommonLeadingBits(all_nodes[1].id) &&
        all_nodes[0].id.CommonLeadingBits(node_id) <
            all_nodes[0].id.CommonLeadingBits(all_nodes[1].id) + 2)
      return DoAddNode(node_id, good, attempts);
  }
}

void InitialiseNetwork() {
  all_nodes.clear();
  all_nodes.reserve(g_good_count);
  // Add first node
  all_nodes.emplace_back(NodeId(NodeId::IdType::kRandomId), true);
  // Add others
  for (size_t i(1); i < g_good_count; ++i)
    AddNode(true);
  // TLOG(kCyan) << "\nAdded " << g_good_count << " good nodes";
  // if (g_algorithm != CommonLeadingBitsAlgorithm::kNone) {
  //   TLOG(kCyan) << ", averaging " << static_cast<double>(g_total_attempts) / all_nodes.size()
  //               << " attempt(s) each.\n";
  // } else {
  //   TLOG(kCyan) << ".\n";
  // }
  g_total_attempts = 0;
}

// Constructs a series of NodeIds spread evenly across address space
std::vector<NodeId> GetUniformlyDistributedTargetPoints() {
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
  // TLOG(kCyan) << "Created " << kStepCount << " evenly-distributed target addresses.\n";
  return steps;
}

size_t BadCount() { return all_nodes.size() - g_good_count; }

// Returns group if >= g_majority_size are bad, else returns empty vector.  Returned group is
// default-sorted (i.e. not sorted close to target_id).
BadGroup GetBadGroup(const NodeId& target_id) {
  std::vector<Node> bad_group;
  // Get close group
  std::partial_sort(std::begin(all_nodes), std::begin(all_nodes) + g_group_size,
                    std::end(all_nodes), [&target_id](const Node& lhs, const Node& rhs) {
    return NodeId::CloserToTarget(lhs.id, rhs.id, target_id);
  });
  auto is_bad([](const Node& node) { return !node.good; });
  // Count bad nodes in close group and return the group if majority are bad
  if (static_cast<size_t>(std::count_if(std::begin(all_nodes), std::begin(all_nodes) + g_group_size,
                                        is_bad)) >= g_majority_size) {
    bad_group.assign(std::begin(all_nodes), std::begin(all_nodes) + g_group_size);
    std::sort(std::begin(bad_group), std::end(bad_group));
  }
  return std::make_pair(target_id, std::move(bad_group));
}

// Add bad nodes until we have 'g_bad_group_count' entirely separate bad close groups
std::vector<BadGroup> InjectBadGroups(const std::vector<NodeId>& steps) {
  // TLOG(kCyan) << "Adding bad nodes and checking for compromised groups...\n";
  std::vector<BadGroup> bad_groups;
  while (bad_groups.size() < g_bad_group_count) {
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
  TLOG(kRed) << "for a network of " << g_good_count << " got " << g_bad_group_count
             << " bad groups after adding " << BadCount() << " bad nodes";
  if (g_algorithm != CommonLeadingBitsAlgorithm::kNone) {
    TLOG(kRed) << ", averaging " << static_cast<double>(g_total_attempts) / BadCount()
               << " attempt(s) each. Network poulation = " << g_good_count + BadCount()
               << " Attack = "
               << static_cast<double>(
                      (static_cast<double>(BadCount()) / (g_good_count + BadCount())) * 100.0)
               << "% \n";
  } else {
    TLOG(kRed) << ".\n";
  }
  return bad_groups;
}

void ReportBadGroups(const std::vector<BadGroup>& bad_groups) {
  for (size_t i(0); i < bad_groups.size(); ++i) {
    TLOG(kDefaultColour) << "Bad group " << i << " close to target " << bad_groups[i].first
                         << ":\n";
    for (const auto& node : bad_groups[i].second) {
      if (node.good)
        TLOG(kGreen) << node << '\n';
      else
        TLOG(kYellow) << node << '\n';
    }
  }
}

void CheckLinkedAddresses() {
  return;
  TLOG(kCyan) << "\nChecking linked random addresses...\n";
  size_t attempts(0), compromised_attempts(0);
  std::vector<BadGroup> bad_groups;
  while (attempts < g_total_random_attempts) {
    bad_groups.clear();
    ++attempts;
    NodeId target_id(NodeId::IdType::kRandomId);
    for (size_t i(0); i < g_bad_group_count; ++i) {
      if (i > 0)  // Hash previous target to get new linked one
        target_id = NodeId(crypto::Hash<crypto::SHA512>(target_id.string()).string());
      auto bad_group(GetBadGroup(target_id));
      if (bad_group.second.empty())  // Not a bad group - start a new attempt
        break;
      bad_groups.emplace_back(std::move(bad_group));
    }
    if (bad_groups.size() == g_bad_group_count) {
      ++compromised_attempts;
      TLOG(kRed) << "\nGot bad group chain of " << g_bad_group_count << " after " << attempts
                 << " linked random ID attempts.\n";
      ReportBadGroups(bad_groups);
    }
  }
  std::string output(
      "\n" + std::to_string(compromised_attempts) + " out of " +
      std::to_string(g_total_random_attempts) +
      " linked random addresses were fully managed by compromised close groups.\n\n");
  if (compromised_attempts)
    TLOG(kRed) << output;
  else
    TLOG(kGreen) << output;
}

void RunTest() {
  InitialiseNetwork();
  auto steps(GetUniformlyDistributedTargetPoints());
  auto bad_groups(InjectBadGroups(steps));
  // ReportBadGroups(bad_groups);
  // CheckLinkedAddresses();
}

}  // namespace maidsafe

int main(int argc, char* argv[]) {
  maidsafe::log::Logging::Instance().Initialise(argc, argv);
  // maidsafe::GetValues();

  for (; maidsafe::g_group_size < 16; ++maidsafe::g_group_size) {
    maidsafe::g_majority_size = (maidsafe::g_group_size / 2) + 1;
    for (; maidsafe::g_majority_size < maidsafe::g_group_size; ++maidsafe::g_majority_size) {
      maidsafe::g_good_count = 1000;
      TLOG(kGreen) << "\nClose group size " << maidsafe::g_group_size << " Majority "
                   << maidsafe::g_majority_size << "\n";
      for (int i(1); i < 12; ++i) {
        maidsafe::g_good_count *= 2;
        maidsafe::RunTest();
      }
    }
  }
  return 0;
}
