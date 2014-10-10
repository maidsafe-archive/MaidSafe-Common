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

#ifndef MAIDSAFE_COMMON_TOOLS_ADDRESS_SPACE_TOOL_H_
#define MAIDSAFE_COMMON_TOOLS_ADDRESS_SPACE_TOOL_H_

// This tool runs a basic simulation of a SAFE network under attack from "bad nodes" allowing
// testing of various algorithms and constants in the face of such an attack, without defining how
// the bad nodes would act.
//
// The tool can run multiple iterations of a test comprising the following actions:
//   1. Set up a network of "good" nodes
//   2. Add bad nodes until a defined number of groups are controlled by bad nodes
//   3. Examine random addresses to check if they are fully controlled by bad groups (i.e. a bad
//      consensus chain)
//
// There are various values which can be set to modify how the tool runs.  These should be set in a
// config file and this filepath passed as the only non-logging command line arg.  If no filepath is
// passed, the tool will look for a config file named "address_space_tool.conf" in its own parent
// folder, and if not found will fall back to hard-coded default values.

#include <ostream>
#include <utility>
#include <vector>

#include "cereal/cereal.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/node_id.h"

namespace maidsafe {

namespace tools {

// This defines how new a node's ID is considered for acceptance into an existing network, based on
// the new node's closeness to the existing nodes compared to the existing nodes' closeness to
// eachother.  Since the network measures closeness by XOR, we can consider the the common leading
// bits (the high-order ones) as an equivalent measure; the more CLBs the closer two IDs are.
//
// In the comments beside each enum value below, "CandidiateCLB" represents the single integral
// value denoting the CLB between the new node and the current group, and "GroupCLB" represents a
// similar value for the existing group only.
//
// In all cases except 'kNone', the new node is acceptable if CandidiateCLB > GroupCLB &&
// CandidiateCLB < GroupCLB + 'config.leeway'.
enum class CommonLeadingBitsAlgorithm {
  kClosest,  // CandidiateCLB == CLB between candidate and closest in group.
             // GroupCLB == CLB between closest and second closest in group.
  kHighest,  // CandidiateCLB == highest CLB between candidate and each node in close group.
             // GroupCLB == highest CLB between any two nodes in close group.
  kLowest,   // CandidiateCLB == lowest CLB between candidate and each node in close group.
             // GroupCLB == lowest CLB between any two nodes in close group
  kMean,     // CandidateCLB == mean CLB rounded down to nearest int between candidate and each node
             // in close group.
             // GroupCLB == mean CLB rounded down to nearest int between all nodes in close group.
  kNone      // CLBs aren't considered when adding a new node.
};

struct Config {
  // Cereal v1.0 has a bug (https://github.com/USCiLab/cereal/issues/81) which means we can't add
  // size_t values to JSON archives.  The bug appears to be fixed and is scheduled for v1.1
  // TODO(Fraser): Remove this conditional typedef once the bug is fixed.
#ifdef MAIDSAFE_APPLE
  typedef uint64_t Size;
#else
  typedef size_t Size;
#endif
  Size iterations{4};                   // Test iterations
  Size initial_good_count{1000};        // No. of good nodes used to pre-populate network on first
                                        // test iteration
  double initial_factor{2.0};           // Factor to multiply 'initial_good_count' by for each test
                                        // iteration
  Size group_size{11};                  // Close group size
  Size majority_size{7};                // Majority of close group required to make a mutating
                                        // action on data
  Size bad_group_count{1};              // Target no. of "bad groups" (i.e. a group where
                                        // 'majority_size' are bad)
  Size total_random_attempts{1000000};  // Target no. of random addresses to check after required
                                        // bad group count has been hit
  Size leeway{2};                       // How many more common leading bits will be allowed when
                                        // considering a new node, i.e. how much closer it's
                                        // allowed to be compared to the other nodes
  Size good_added_per_bad{0};           // No. of good nodes added every time a bad node is added
  CommonLeadingBitsAlgorithm algorithm{CommonLeadingBitsAlgorithm::kLowest};  // Described above

  template <typename Archive>
  void save(Archive& archive) const {
    archive(CEREAL_NVP(iterations), CEREAL_NVP(initial_good_count), CEREAL_NVP(initial_factor),
            CEREAL_NVP(group_size), CEREAL_NVP(majority_size), CEREAL_NVP(bad_group_count),
            CEREAL_NVP(total_random_attempts), CEREAL_NVP(leeway), CEREAL_NVP(good_added_per_bad),
            CEREAL_NVP(algorithm));
  }

  template <typename Archive, typename NameValuePair>
  void load_optional_element(Archive& archive, NameValuePair name_value_pair) {
    try {
      archive(name_value_pair);
    } catch (const std::exception&) {
    }
  }

  template <typename Archive>
  void load(Archive& archive) {
    load_optional_element(archive, CEREAL_NVP(iterations));
    load_optional_element(archive, CEREAL_NVP(initial_good_count));
    load_optional_element(archive, CEREAL_NVP(initial_factor));
    load_optional_element(archive, CEREAL_NVP(group_size));
    load_optional_element(archive, CEREAL_NVP(majority_size));
    load_optional_element(archive, CEREAL_NVP(bad_group_count));
    load_optional_element(archive, CEREAL_NVP(total_random_attempts));
    load_optional_element(archive, CEREAL_NVP(leeway));
    load_optional_element(archive, CEREAL_NVP(good_added_per_bad));
    load_optional_element(archive, CEREAL_NVP(algorithm));
  }
};

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,
                                             const Config& config) {
  ostream << "\titerations:            " << config.iterations << '\n';
  ostream << "\tinitial_good_count:    " << config.initial_good_count << '\n';
  ostream << "\tinitial_factor:        " << config.initial_factor << '\n';
  ostream << "\tgroup_size:            " << config.group_size << '\n';
  ostream << "\tmajority_size:         " << config.majority_size << '\n';
  ostream << "\tbad_group_count:       " << config.bad_group_count << '\n';
  ostream << "\ttotal_random_attempts: " << config.total_random_attempts << '\n';
  ostream << "\tleeway:                " << config.leeway << '\n';
  ostream << "\tgood_added_per_bad:    " << config.good_added_per_bad << '\n';
  ostream << "\talgorithm:             ";
  switch (config.algorithm) {
    case CommonLeadingBitsAlgorithm::kClosest:
      ostream << "kClosest";
      break;
    case CommonLeadingBitsAlgorithm::kHighest:
      ostream << "kHighest";
      break;
    case CommonLeadingBitsAlgorithm::kLowest:
      ostream << "kLowest";
      break;
    case CommonLeadingBitsAlgorithm::kMean:
      ostream << "kMean";
      break;
    case CommonLeadingBitsAlgorithm::kNone:
      ostream << "kNone";
      break;
    default:
      ostream << "INVALID VALUE";
  }
  ostream << '\n';
  return ostream;
}



struct Node {
  Node(NodeId id_in, bool good_in) : id(std::move(id_in)), good(good_in), rank(0) {}
  Node() : id(), good(true), rank(0) {}
  Node(Node&& o) MAIDSAFE_NOEXCEPT : id(std::move(o.id)), good(o.good), rank(std::move(o.rank)) {}
  Node(const Node& other) = default;
  Node& operator=(const Node& other) = default;
  Node& operator=(Node&& other) {
    id = std::move(other.id);
    good = std::move(other.good);
    rank = std::move(other.rank);
    return *this;
  }

  NodeId id;
  bool good;
  int rank;
};

inline bool operator<(const Node& lhs, const Node& rhs) { return lhs.id < rhs.id; }

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,
                                             const Node& node) {
  ostream << node.id << (node.good ? " (good node)" : " (bad node) ") << " with rank " << node.rank;
  return ostream;
}

typedef std::pair<NodeId, std::vector<Node>> BadGroup;



class Test {
 public:
  explicit Test(Config config) : config_(std::move(config)), all_nodes_() {}
  void Run();

 private:
  int Accumulate(std::vector<Node>::const_iterator first, std::vector<Node>::const_iterator last,
                 const NodeId& target, int& highest, int& lowest) const;

  int CommonLeadingBits(int highest, int lowest, int sum, int count) const;

  // Requires first 'group_size' entries of 'g_all_nodes' to be sorted by closeness to target.
  int GroupCommonLeadingBits(size_t group_size) const;

  // Requires first 'group_size' entries of 'g_all_nodes' to be sorted by closeness to
  // 'candidate_node'.
  int CandidateCommonLeadingBits(const NodeId& candidate_node, size_t group_size) const;

  void UpdateRank(size_t group_size);

  std::pair<int, int> RankValues(size_t group_size) const;

  bool RankAllowed(size_t group_size) const;

  void AddNode(bool good);

  void DoAddNode(const NodeId& node_id, bool good, int attempts);

  void InitialiseNetwork();

  // Constructs a series of NodeIds spread evenly across address space
  std::vector<NodeId> GetUniformlyDistributedTargetPoints() const;

  // Returns group if >= g_config.majority_size are bad, else returns empty vector.  Returned group
  // is default-sorted (i.e. not sorted close to target_id).
  BadGroup GetBadGroup(const NodeId& target_id) const;

  // Add bad nodes until we have 'g_config.bad_group_count' entirely separate bad close groups
  std::vector<BadGroup> InjectBadGroups(const std::vector<NodeId>& steps);

  void ReportBadGroups(const std::vector<BadGroup>& bad_groups) const;

  void CheckLinkedAddresses() const;

  Config config_;
  std::vector<Node> all_nodes_;
  size_t total_attempts_{0};
  size_t good_count_{0};
  size_t bad_count_{0};
};

}  // namespace tools

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TOOLS_ADDRESS_SPACE_TOOL_H_
