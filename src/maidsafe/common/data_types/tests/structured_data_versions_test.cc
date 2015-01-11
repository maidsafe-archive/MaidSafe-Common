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

#include <algorithm>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "boost/optional/optional_io.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/data_types/structured_data_versions.h"
#include "maidsafe/common/data_types/structured_data_versions_cereal.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace detail {  // Provide ADL for this TU to std::vector comparisons

bool operator==(const StructuredDataVersionsCereal::Branch& ref_lhs,
                const StructuredDataVersionsCereal::Branch& ref_rhs) {
  return ref_lhs.absent_parent == ref_rhs.absent_parent && ref_lhs.names == ref_rhs.names;
}

bool operator==(const StructuredDataVersionsCereal& ref_lhs,
                const StructuredDataVersionsCereal& ref_rhs) {
  return ref_lhs.max_branches == ref_rhs.max_branches &&
         ref_lhs.max_versions == ref_rhs.max_versions && ref_lhs.branches == ref_rhs.branches;
}

}  // namespace detail

namespace test {

namespace {

using VersionName = StructuredDataVersions::VersionName;
using Branch = std::vector<VersionName>;

VersionName::Id RandomId() { return VersionName::Id{Identity{RandomAlphaNumericString(64)}}; }

std::string DisplayVersion(const VersionName& version, bool to_hex) {
  return std::to_string(version.index) + "-" +
         (version.id->IsInitialised()
              ? (to_hex ? HexEncode(version.id.value) : version.id->string()).substr(0, 3)
              : ("Uninitialised"));
}

std::string DisplayVersions(const Branch& versions, bool to_hex) {
  std::string result;
  for (const auto& version : versions)
    result += DisplayVersion(version, to_hex) + "  ";
  return result;
}

const VersionName v0_aaa{0, ImmutableData::Name{Identity{std::string(64, 'a')}}};
const VersionName v1_bbb{1, ImmutableData::Name{Identity{std::string(64, 'b')}}};
const VersionName v2_ccc{2, ImmutableData::Name{Identity{std::string(64, 'c')}}};
const VersionName v2_ddd{2, ImmutableData::Name{Identity{std::string(64, 'd')}}};
const VersionName v2_eee{2, ImmutableData::Name{Identity{std::string(64, 'e')}}};
const VersionName v3_fff{3, ImmutableData::Name{Identity{std::string(64, 'f')}}};
const VersionName v3_ggg{3, ImmutableData::Name{Identity{std::string(64, 'g')}}};
const VersionName v3_hhh{3, ImmutableData::Name{Identity{std::string(64, 'h')}}};
const VersionName v4_iii{4, ImmutableData::Name{Identity{std::string(64, 'i')}}};
const VersionName v4_jjj{4, ImmutableData::Name{Identity{std::string(64, 'j')}}};
const VersionName v4_kkk{4, ImmutableData::Name{Identity{std::string(64, 'k')}}};
const VersionName v4_lll{4, ImmutableData::Name{Identity{std::string(64, 'l')}}};
const VersionName v4_mmm{4, ImmutableData::Name{Identity{std::string(64, 'm')}}};
const VersionName v5_nnn{5, ImmutableData::Name{Identity{std::string(64, 'n')}}};
const VersionName absent{6, ImmutableData::Name{Identity{std::string(64, 'x')}}};
const VersionName v7_yyy{7, ImmutableData::Name{Identity{std::string(64, 'y')}}};
const VersionName v8_zzz{8, ImmutableData::Name{Identity{std::string(64, 'z')}}};

void ConstructAsDiagram(StructuredDataVersions& versions, bool shuffle_order_of_puts = true) {
  /*
     7-yyy       0-aaa (root)
       |           |
       |           |
     8-zzz       1-bbb
              /    |   \
            /      |     \
         2-ccc   2-ddd   2-eee
         /         |          \
       /           |            \
    3-fff        3-ggg           3-hhh
      |           /  \             /  \
      |         /      \         /      \
    4-iii    4-jjj    4-kkk   4-lll    4-mmm
                        |
                        |
                      5-nnn
  */
  std::vector<std::pair<VersionName, VersionName>> puts;
  puts.push_back(std::make_pair(VersionName(), v0_aaa));
  puts.push_back(std::make_pair(v0_aaa, v1_bbb));
  puts.push_back(std::make_pair(v1_bbb, v2_ccc));
  puts.push_back(std::make_pair(v2_ccc, v3_fff));
  puts.push_back(std::make_pair(v3_fff, v4_iii));
  puts.push_back(std::make_pair(v1_bbb, v2_ddd));
  puts.push_back(std::make_pair(v2_ddd, v3_ggg));
  puts.push_back(std::make_pair(v3_ggg, v4_jjj));
  puts.push_back(std::make_pair(v3_ggg, v4_kkk));
  puts.push_back(std::make_pair(v4_kkk, v5_nnn));
  puts.push_back(std::make_pair(v1_bbb, v2_eee));
  puts.push_back(std::make_pair(v2_eee, v3_hhh));
  puts.push_back(std::make_pair(v3_hhh, v4_lll));
  puts.push_back(std::make_pair(v3_hhh, v4_mmm));
  puts.push_back(std::make_pair(absent, v7_yyy));
  puts.push_back(std::make_pair(v7_yyy, v8_zzz));

  if (shuffle_order_of_puts) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::shuffle(std::begin(puts), std::end(puts), generator);
  }
  for (const auto& put : puts) {
    LOG(kVerbose) << "Putting - old: " << DisplayVersion(put.first, false)
                  << "  new: " << DisplayVersion(put.second, false);
    boost::optional<VersionName> popped_version;
    ASSERT_NO_THROW(popped_version = versions.Put(put.first, put.second));
    ASSERT_FALSE(popped_version);
  }
}

template <typename... T>
testing::AssertionResult CheckVersions(const std::vector<VersionName>& actual_versions,
                                       T... expected_versions) {
  const std::vector<VersionName> expected{expected_versions...};
  return (actual_versions == expected)
             ? testing::AssertionSuccess()
             : testing::AssertionFailure()
                   << "\n    Expected: " << DisplayVersions(expected, false)
                   << "\n    Actual:   " << DisplayVersions(actual_versions, false) << '\n';
}

template <typename... T>
testing::AssertionResult CheckBranch(StructuredDataVersions& versions,
                                     const VersionName& first_expected_version,
                                     T... other_expected_versions) {
  try {
    const Branch actual_branch{versions.GetBranch(first_expected_version)};
    return CheckVersions(actual_branch, first_expected_version, other_expected_versions...);
  } catch (const std::exception& e) {
    return testing::AssertionFailure() << "\n    " << boost::diagnostic_information(e) << '\n';
  }
}

testing::AssertionResult Equivalent(const StructuredDataVersions& lhs,
                                    const StructuredDataVersions& rhs) {
  if (lhs.max_versions() != rhs.max_versions()) {
    return testing::AssertionFailure() << "lhs.max_versions (" << lhs.max_versions()
                                       << ") != rhs.max_versions(" << rhs.max_versions() << ")";
  }
  if (lhs.max_branches() != rhs.max_branches()) {
    return testing::AssertionFailure() << "lhs.max_branches (" << lhs.max_branches()
                                       << ") != rhs.max_branches(" << rhs.max_branches() << ")";
  }

  auto lhs_tots(lhs.Get());
  auto rhs_tots(rhs.Get());
  std::sort(std::begin(lhs_tots), std::end(lhs_tots));
  std::sort(std::begin(rhs_tots), std::end(rhs_tots));
  if (lhs_tots != rhs_tots) {
    return testing::AssertionFailure() << "lhs.tips_of_trees != rhs.tips_of_trees:\n"
                                       << "  lhs: " << DisplayVersions(lhs_tots, false)
                                       << "\n  rhs: " << DisplayVersions(rhs_tots, false) << '\n';
  }

  std::vector<Branch> lhs_branches, rhs_branches;
  for (const auto& lhs_tot : lhs_tots)
    lhs_branches.push_back(lhs.GetBranch(lhs_tot));
  for (const auto& rhs_tot : rhs_tots)
    rhs_branches.push_back(rhs.GetBranch(rhs_tot));
  if (lhs_branches != rhs_branches) {
    std::string output("lhs.branches != rhs.branches:\n");
    for (size_t i(0); i != lhs_branches.size(); ++i) {
      output += "lhs " + std::to_string(i) + ": " + DisplayVersions(lhs_branches[i], false) +
                "\n rhs " + std::to_string(i) + ": " + DisplayVersions(rhs_branches[i], false) +
                '\n';
    }
    return testing::AssertionFailure() << output;
  }
  return testing::AssertionSuccess();
}

}  // unnamed namespace

TEST(StructuredDataVersionsTest, BEH_VersionName) {
  // Default c'tor
  const VersionName defaulted;
  EXPECT_EQ(std::numeric_limits<VersionName::Index>::max(), defaulted.index);
  EXPECT_FALSE(defaulted.id->IsInitialised());
  EXPECT_FALSE(defaulted.forking_child_count);

  // Index and ID c'tor
  const VersionName::Index index{
      (RandomUint32() % (std::numeric_limits<VersionName::Index>::max() - 1)) + 1};  // want > 0
  const VersionName::Id id{RandomId()};
  const uint32_t forking_child_count = RandomUint32();
  VersionName version{index, id};
  EXPECT_EQ(index, version.index);
  EXPECT_EQ(id, version.id);
  EXPECT_FALSE(version.forking_child_count);
  version.forking_child_count = forking_child_count;

  // swap
  VersionName swapped;
  swap(version, swapped);
  EXPECT_EQ(std::numeric_limits<VersionName::Index>::max(), version.index);
  EXPECT_FALSE(version.id->IsInitialised());
  EXPECT_FALSE(version.forking_child_count);
  EXPECT_EQ(index, swapped.index);
  EXPECT_EQ(id, swapped.id);
  EXPECT_EQ(forking_child_count, swapped.forking_child_count);
  swap(version, swapped);

  // Relational operators (NB they don't use forking_child_count)
  VersionName version_without_optional{index, id};
  EXPECT_TRUE(version == version_without_optional);
  EXPECT_FALSE(version != version_without_optional);
  EXPECT_FALSE(version < version_without_optional);
  EXPECT_FALSE(version > version_without_optional);
  EXPECT_TRUE(version <= version_without_optional);
  EXPECT_TRUE(version >= version_without_optional);

  VersionName version_with_smaller_index{index - 1, id};
  EXPECT_FALSE(version == version_with_smaller_index);
  EXPECT_TRUE(version != version_with_smaller_index);
  EXPECT_FALSE(version < version_with_smaller_index);
  EXPECT_TRUE(version > version_with_smaller_index);
  EXPECT_FALSE(version <= version_with_smaller_index);
  EXPECT_TRUE(version >= version_with_smaller_index);

  VersionName::Id smaller_id{RandomId()};
  while (smaller_id >= id)
    smaller_id = RandomId();
  VersionName version_with_smaller_id{index, smaller_id};
  EXPECT_FALSE(version == version_with_smaller_id);
  EXPECT_TRUE(version != version_with_smaller_id);
  EXPECT_FALSE(version < version_with_smaller_id);
  EXPECT_TRUE(version > version_with_smaller_id);
  EXPECT_FALSE(version <= version_with_smaller_id);
  EXPECT_TRUE(version >= version_with_smaller_id);

  // Copy and assignment
  VersionName copied{version};
  EXPECT_EQ(index, copied.index);
  EXPECT_EQ(id, copied.id);
  EXPECT_EQ(forking_child_count, copied.forking_child_count);

  VersionName moved{std::move(copied)};
  EXPECT_EQ(index, moved.index);
  EXPECT_EQ(id, moved.id);
  EXPECT_EQ(forking_child_count, moved.forking_child_count);

  copied = defaulted;
  EXPECT_EQ(std::numeric_limits<VersionName::Index>::max(), copied.index);
  EXPECT_FALSE(copied.id->IsInitialised());
  EXPECT_FALSE(copied.forking_child_count);

  moved = std::move(copied);
  EXPECT_EQ(std::numeric_limits<VersionName::Index>::max(), moved.index);
  EXPECT_FALSE(moved.id->IsInitialised());
  EXPECT_FALSE(moved.forking_child_count);

  // Serialise and parse, with and without forking_child_count
  auto serialised = Serialise(version);
  auto parsed = Parse<VersionName>(serialised);
  EXPECT_EQ(index, parsed.index);
  EXPECT_EQ(id, parsed.id);
  EXPECT_EQ(forking_child_count, parsed.forking_child_count);

  copied = version;
  copied.forking_child_count = boost::none;
  serialised = Serialise(copied);
  parsed = Parse<VersionName>(serialised);
  EXPECT_EQ(index, parsed.index);
  EXPECT_EQ(id, parsed.id);
  EXPECT_FALSE(parsed.forking_child_count);
}

TEST(StructuredDataVersionsTest, BEH_Construct) {
  // NB - Constructor taking serialised SDV is tested later
  EXPECT_THROW(StructuredDataVersions(0, 1), common_error);
  EXPECT_THROW(StructuredDataVersions(1, 0), common_error);
  StructuredDataVersions versions{1, 2};
  EXPECT_EQ(1, versions.max_versions());
  EXPECT_EQ(2, versions.max_branches());
}

TEST(StructuredDataVersionsTest, BEH_Get) {
  // Check with empty SDV
  StructuredDataVersions versions{100, 20};
  EXPECT_TRUE(versions.Get().empty());

  // Check with SDV constructed as per the diagram at the top of structured_data_versions.h
  ConstructAsDiagram(versions);
  std::vector<VersionName> tips_of_trees{versions.Get()};
  EXPECT_TRUE(CheckVersions(tips_of_trees, v8_zzz, v5_nnn, v4_mmm, v4_lll, v4_jjj, v4_iii));
  EXPECT_TRUE(
      std::is_sorted(std::begin(tips_of_trees), std::end(tips_of_trees),
                     [](const VersionName& lhs, const VersionName& rhs) { return lhs > rhs; }));
}

TEST(StructuredDataVersionsTest, BEH_GetBranch) {
  // Check with empty SDV
  StructuredDataVersions versions{100, 20};
  EXPECT_THROW(versions.GetBranch(v0_aaa), common_error);

  // Check with SDV constructed as per the diagram at the top of structured_data_versions.h
  ConstructAsDiagram(versions);
  EXPECT_TRUE(CheckBranch(versions, v8_zzz, v7_yyy));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_jjj, v3_ggg, v2_ddd, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v5_nnn, v4_kkk, v3_ggg, v2_ddd, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_lll, v3_hhh, v2_eee, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_mmm, v3_hhh, v2_eee, v1_bbb, v0_aaa));

  // Check for version which is not a tip-of-tree
  try {
    versions.GetBranch(v0_aaa);
  } catch (const common_error& e) {
    EXPECT_EQ(make_error_code(CommonErrors::invalid_parameter), e.code());
  }

  // Check for version which doesn't exist
  try {
    versions.GetBranch(absent);
  } catch (const common_error& e) {
    EXPECT_EQ(make_error_code(CommonErrors::no_such_element), e.code());
  }
}

TEST(StructuredDataVersionsTest, BEH_Put) {
  // Keep a clone of 'versions' used to check that bad operations performed on 'versions' don't
  // modify its state (i.e. that it sticks to the strong exception guarantee).
  StructuredDataVersions versions{16, 6};
  StructuredDataVersions clone{versions.max_versions(), versions.max_branches()};
  ConstructAsDiagram(versions, false);
  ConstructAsDiagram(clone, false);
  ASSERT_TRUE(Equivalent(versions, clone));

  // Try to add a second root
  VersionName bad_root_version{0, RandomId()};
  EXPECT_THROW(versions.Put(VersionName(), bad_root_version), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));
  VersionName new_version{1, RandomId()};
  EXPECT_THROW(versions.Put(VersionName(), new_version), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));
  EXPECT_THROW(versions.Put(v5_nnn, bad_root_version), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Try to add a new version which implies the root is not the given one
  EXPECT_THROW(versions.Put(bad_root_version, new_version), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Try to put a version with an invalid ID
  EXPECT_THROW(versions.Put(v5_nnn, VersionName{6, VersionName::Id{}}), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Try to put a version which already exists (should be a no-op)
  boost::optional<VersionName> popped_version;
  EXPECT_NO_THROW(popped_version = versions.Put(v4_kkk, v5_nnn));
  EXPECT_FALSE(popped_version);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Try to put a version which already exists, but with a different 'old_version' parent
  EXPECT_THROW(versions.Put(v2_ccc, v3_ggg), common_error);  // version inside branch
  EXPECT_THROW(versions.Put(v4_jjj, v5_nnn), common_error);  // tip-of-tree
  EXPECT_THROW(versions.Put(v4_jjj, v7_yyy), common_error);  // orphan
  EXPECT_TRUE(Equivalent(versions, clone));

  // Try to put a version which causes a circular chain parent->child->parent
  EXPECT_THROW(versions.Put(v8_zzz, absent), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Put a valid new version, which should cause the root to be popped
  const VersionName v5_ooo{5, ImmutableData::Name{Identity{std::string(64, '0')}}};
  EXPECT_NO_THROW(popped_version = versions.Put(v4_jjj, v5_ooo));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v0_aaa, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(v4_jjj, v5_ooo);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Another two valid puts should cause 1-bbb then 2-ccc to get popped.  2-ccc should be chosen
  // over 2-ddd or 2-eee since it is the lowest version name.
  const VersionName v5_ppp{5, ImmutableData::Name{Identity{std::string(64, 'p')}}};
  EXPECT_NO_THROW(popped_version = versions.Put(v4_lll, v5_ppp));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v1_bbb, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(v4_lll, v5_ppp);
  EXPECT_TRUE(Equivalent(versions, clone));

  const VersionName v5_qqq{5, ImmutableData::Name{Identity{std::string(64, 'q')}}};
  EXPECT_NO_THROW(popped_version = versions.Put(v4_mmm, v5_qqq));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v2_ccc, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(v4_mmm, v5_qqq);
  EXPECT_TRUE(Equivalent(versions, clone));

  /* Confirm current SDV state is:

     7-yyy      3-fff      2-ddd               2-eee
       |        (root)       |                   |
       |          |          |                   |
     8-zzz      4-iii      3-ggg               3-hhh
                            /  \                /  \
                          /      \            /      \
                       4-jjj    4-kkk      4-lll    4-mmm
                         |        |          |        |
                         |        |          |        |
                       5-ooo    5-nnn      5-ppp    5-qqq
  */
  EXPECT_TRUE(CheckBranch(versions, v8_zzz, v7_yyy));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff));
  EXPECT_TRUE(CheckBranch(versions, v5_ooo, v4_jjj, v3_ggg, v2_ddd));
  EXPECT_TRUE(CheckBranch(versions, v5_nnn, v4_kkk, v3_ggg, v2_ddd));
  EXPECT_TRUE(CheckBranch(versions, v5_ppp, v4_lll, v3_hhh, v2_eee));
  EXPECT_TRUE(CheckBranch(versions, v5_qqq, v4_mmm, v3_hhh, v2_eee));

  // Check we can't create a new branch since we're at the limit of branches
  const VersionName v6_rrr{6, ImmutableData::Name{Identity{std::string(64, 'r')}}};
  EXPECT_THROW(versions.Put(v4_jjj, v6_rrr), common_error);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Put one more version to make root a branch with just one version
  EXPECT_NO_THROW(popped_version = versions.Put(v5_qqq, v6_rrr));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v3_fff, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(v5_qqq, v6_rrr);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Check we can now create a new branch even though we're at the limit of branches, since root
  // will be popped meaning we won't exceed the branch limit.
  const VersionName v7_sss{7, ImmutableData::Name{Identity{std::string(64, 's')}}};
  EXPECT_NO_THROW(popped_version = versions.Put(absent, v7_sss));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v4_iii, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(absent, v7_sss);
  EXPECT_TRUE(Equivalent(versions, clone));

  // Confirm the new root is 2-ddd by adding another version and checking 2-ddd is popped
  const VersionName v8_ttt{8, ImmutableData::Name{Identity{std::string(64, 't')}}};
  EXPECT_NO_THROW(popped_version = versions.Put(v7_sss, v8_ttt));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v2_ddd, *popped_version);
  EXPECT_FALSE(Equivalent(versions, clone));
  clone.Put(v7_sss, v8_ttt);
  EXPECT_TRUE(Equivalent(versions, clone));

  /* Check adding the absent version causes the orphan branches to join on.  Should yield SDV state:

         4-jjj      4-kkk      2-eee
         (root)       |          |
           |          |          |
         5-ooo      5-nnn      3-hhh
           |                    /  \
           |                  /      \
         6-xxx             4-lll    4-mmm
          / \                |        |
        /     \              |        |
     7-yyy   7-sss         5-ppp    5-qqq
       |       |                      |
       |       |                      |
     8-zzz   8-ttt                  6-rrr
  */
  EXPECT_NO_THROW(popped_version = versions.Put(v5_ooo, absent));
  ASSERT_TRUE(!!popped_version);
  EXPECT_EQ(v3_ggg, *popped_version);
  clone.Put(v5_ooo, absent);
  EXPECT_TRUE(Equivalent(versions, clone));

  EXPECT_TRUE(CheckBranch(versions, v8_zzz, v7_yyy, absent, v5_ooo, v4_jjj));
  EXPECT_TRUE(CheckBranch(versions, v8_ttt, v7_sss, absent, v5_ooo, v4_jjj));
  EXPECT_TRUE(CheckBranch(versions, v5_nnn, v4_kkk));
  EXPECT_TRUE(CheckBranch(versions, v5_ppp, v4_lll, v3_hhh, v2_eee));
  EXPECT_TRUE(CheckBranch(versions, v6_rrr, v5_qqq, v4_mmm, v3_hhh, v2_eee));
}

TEST(StructuredDataVersionsTest, BEH_DeleteBranchUntilFork) {
  // Check with empty SDV
  StructuredDataVersions versions{100, 20};
  EXPECT_THROW(versions.DeleteBranchUntilFork(v0_aaa), common_error);

  // Keep a clone of 'versions' used to check that bad operations performed on 'versions' don't
  // modify its state (i.e. that it sticks to the strong exception guarantee).
  StructuredDataVersions clone{versions.max_versions(), versions.max_branches()};
  ConstructAsDiagram(versions);
  ConstructAsDiagram(clone);
  ASSERT_TRUE(Equivalent(versions, clone));

  // Check for version which is not a tip-of-tree
  try {
    versions.DeleteBranchUntilFork(v0_aaa);
  } catch (const common_error& e) {
    EXPECT_EQ(make_error_code(CommonErrors::invalid_parameter), e.code());
  }
  EXPECT_TRUE(Equivalent(versions, clone));

  // Check for version which doesn't exist
  try {
    versions.DeleteBranchUntilFork(absent);
  } catch (const common_error& e) {
    EXPECT_EQ(make_error_code(CommonErrors::no_such_element), e.code());
  }
  EXPECT_TRUE(Equivalent(versions, clone));

  // Delete 4-jjj branch until fork (should only remove 4-jjj)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v4_jjj));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v4_jjj), common_error);
  std::vector<VersionName> tips_of_trees{versions.Get()};
  EXPECT_TRUE(CheckVersions(tips_of_trees, v8_zzz, v5_nnn, v4_mmm, v4_lll, v4_iii));
  EXPECT_TRUE(CheckBranch(versions, v8_zzz, v7_yyy));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v5_nnn, v4_kkk, v3_ggg, v2_ddd, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_lll, v3_hhh, v2_eee, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_mmm, v3_hhh, v2_eee, v1_bbb, v0_aaa));

  // Delete 5-nnn branch until fork (should remove 5-nnn, 4-kkk, 3-ggg and 2-ddd)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v5_nnn));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v5_nnn), common_error);
  tips_of_trees = versions.Get();
  EXPECT_TRUE(CheckVersions(tips_of_trees, v8_zzz, v4_mmm, v4_lll, v4_iii));
  EXPECT_TRUE(CheckBranch(versions, v8_zzz, v7_yyy));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_lll, v3_hhh, v2_eee, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_mmm, v3_hhh, v2_eee, v1_bbb, v0_aaa));

  // Delete 8-zzz branch until fork (should remove 8-zzz and 7-yyy)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v8_zzz));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v8_zzz), common_error);
  tips_of_trees = versions.Get();
  EXPECT_TRUE(CheckVersions(tips_of_trees, v4_mmm, v4_lll, v4_iii));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_lll, v3_hhh, v2_eee, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_mmm, v3_hhh, v2_eee, v1_bbb, v0_aaa));

  // Delete 4-lll branch until fork (should only remove 4-lll)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v4_lll));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v4_lll), common_error);
  tips_of_trees = versions.Get();
  EXPECT_TRUE(CheckVersions(tips_of_trees, v4_mmm, v4_iii));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));
  EXPECT_TRUE(CheckBranch(versions, v4_mmm, v3_hhh, v2_eee, v1_bbb, v0_aaa));

  // Delete 4-mmm branch until fork (should remove 4-mmm, 3-hhh and 2-eee)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v4_mmm));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v4_mmm), common_error);
  tips_of_trees = versions.Get();
  EXPECT_TRUE(CheckVersions(tips_of_trees, v4_iii));
  EXPECT_TRUE(CheckBranch(versions, v4_iii, v3_fff, v2_ccc, v1_bbb, v0_aaa));

  // Delete 4-iii branch until fork (should remove all remaining)
  EXPECT_NO_THROW(versions.DeleteBranchUntilFork(v4_iii));
  EXPECT_THROW(versions.DeleteBranchUntilFork(v4_iii), common_error);
  EXPECT_TRUE(versions.Get().empty());
}

TEST(StructuredDataVersionsTest, BEH_Clear) {
  // Check with empty SDV
  StructuredDataVersions versions{100, 20};
  EXPECT_NO_THROW(versions.clear());

  // Populate SDV
  ConstructAsDiagram(versions);
  EXPECT_FALSE(versions.Get().empty());
  EXPECT_NO_THROW(versions.clear());
  EXPECT_TRUE(versions.Get().empty());
}

TEST(StructuredDataVersionsTest, BEH_Serialise) {
  StructuredDataVersions versions1(100, 20), versions2(100, 20), versions3(1, 1);
  ConstructAsDiagram(versions1);
  ConstructAsDiagram(versions2);
  ImmutableData::Name single_id(Identity(RandomString(64)));
  versions3.Put(StructuredDataVersions::VersionName(),
                StructuredDataVersions::VersionName(0, single_id));

  ASSERT_TRUE(Equivalent(versions1, versions2));

  auto serialised1(versions1.Serialise());
  auto serialised2(versions2.Serialise());
  auto serialised3(versions3.Serialise());

  ASSERT_EQ(serialised1, serialised2);

  StructuredDataVersions parsed1(serialised1);
  StructuredDataVersions parsed2(serialised2);
  StructuredDataVersions parsed3(serialised3);

  EXPECT_TRUE(Equivalent(versions1, parsed1));
  EXPECT_TRUE(Equivalent(versions2, parsed2));
  EXPECT_TRUE(Equivalent(parsed1, parsed2));
  EXPECT_TRUE(Equivalent(versions3, parsed3));

  auto reserialised1(parsed1.Serialise());
  auto reserialised2(parsed2.Serialise());
  auto reserialised3(parsed3.Serialise());
  EXPECT_EQ(serialised1, reserialised1);
  EXPECT_EQ(serialised2, reserialised2);
  EXPECT_EQ(reserialised1, reserialised2);
  EXPECT_EQ(serialised3, reserialised3);
}

TEST(StructuredDataVersionsTest, BEH_ApplySerialised) {
  StructuredDataVersions versions1(100, 20);
  ASSERT_NO_THROW(ConstructAsDiagram(versions1));
  auto serialised1(versions1.Serialise());
  // Check applying all included versions doesn't modify the SDV.
  EXPECT_NO_THROW(versions1.ApplySerialised(serialised1));
  auto temp_serialised(versions1.Serialise());
  EXPECT_EQ(serialised1, temp_serialised);

  // Construct SDV with only "absent" version from diagram included.
  VersionName v5_nnn(5, ImmutableData::Name(Identity(std::string(64, 'n'))));
  VersionName absent(6, ImmutableData::Name(Identity(std::string(64, 'x'))));
  StructuredDataVersions versions2(100, 20);
  EXPECT_NO_THROW(versions2.Put(v5_nnn, absent));
  auto serialised2(versions2.Serialise());

  // Apply each serialised SDV to the other and check they produce the same resultant SDV.
  EXPECT_NO_THROW(versions1.ApplySerialised(serialised2));
  EXPECT_NO_THROW(versions2.ApplySerialised(serialised1));
  EXPECT_TRUE(Equivalent(versions1, versions2));
}

TEST(StructuredDataVersionsTest, BEH_SerialisationOptionalFieldTest) {
  using StructuredDataVersionsCereal = maidsafe::detail::StructuredDataVersionsCereal;

  VersionName ver_0, ver_1, ver_2, ver_3;
  ver_0.index = ver_1.index = ver_2.index = ver_3.index = 100;
  ver_0.id = ver_1.id = ver_2.id = ver_3.id = RandomId();

  ver_0.forking_child_count = ver_2.forking_child_count = 33;

  EXPECT_TRUE(ver_0.forking_child_count && ver_2.forking_child_count);
  EXPECT_FALSE(ver_1.forking_child_count || ver_3.forking_child_count);

  StructuredDataVersionsCereal sdv_lhs, sdv_rhs;
  sdv_lhs.max_versions = sdv_rhs.max_versions = 20;
  sdv_lhs.max_branches = sdv_rhs.max_branches = 10;

  StructuredDataVersionsCereal::Branch sdv_br_0, sdv_br_1;

  sdv_br_0.absent_parent = ver_0;
  sdv_br_0.names.push_back(ver_3);

  sdv_br_1.names.push_back(ver_2);

  sdv_lhs.branches.push_back(sdv_br_0);
  sdv_rhs.branches.push_back(sdv_br_1);

  EXPECT_FALSE(sdv_lhs == sdv_rhs);
  // Lhs has optional absent_parent, Rhs has optional absent_parent missing
  EXPECT_FALSE(sdv_lhs.branches[0].absent_parent == sdv_rhs.branches[0].absent_parent);

  // Serialise
  std::string str_serialised_0{ConvertToString(sdv_lhs)};

  // Deserialise
  ConvertFromString(str_serialised_0, sdv_rhs);
  // Rhs has optional absent_parent restored by presence of it in stream.
  // Rhs has optional forking_child_count forced absent by absence of it in stream.
  EXPECT_TRUE(sdv_lhs == sdv_rhs);

  // Reserialise
  std::string str_serialised_1{ConvertToString(sdv_rhs)};
  EXPECT_TRUE(str_serialised_0 == str_serialised_1);
}

}  // namespace test

}  // namespace maidsafe
