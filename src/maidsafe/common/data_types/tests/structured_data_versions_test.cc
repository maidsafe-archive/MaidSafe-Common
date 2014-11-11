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
#include <random>
#include <string>
#include <vector>

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/common/data_types/structured_data_versions.h"
#include "maidsafe/common/data_types/structured_data_versions_cereal.h"

namespace maidsafe {

namespace detail {  // Provide ADL for this TU to std::vector comparisons

using Sdv_t = StructuredDataVersionsCereal;
using SdvBr_t = StructuredDataVersionsCereal::Branch;
using Ver_t = VersionCereal;

bool operator==(const Ver_t& ref_lhs, const Ver_t& ref_rhs) {
  return ref_lhs.forking_child_count_ == ref_rhs.forking_child_count_ &&
      ref_lhs.id_ == ref_rhs.id_ && ref_lhs.index_ == ref_rhs.index_;
}

bool operator==(const SdvBr_t& ref_lhs, const SdvBr_t& ref_rhs) {
  return ref_lhs.absent_parent_ == ref_rhs.absent_parent_ && ref_lhs.name_ == ref_rhs.name_;
}

bool operator==(const Sdv_t& ref_lhs, const Sdv_t& ref_rhs) {
  return ref_lhs.max_branches_ == ref_rhs.max_branches_ &&
      ref_lhs.max_versions_ == ref_rhs.max_versions_ && ref_lhs.branch_ == ref_rhs.branch_;
}

}  // namespace detail

namespace test {

namespace {

typedef StructuredDataVersions::VersionName VersionName;

ImmutableData::Name RandomId() {
  return ImmutableData::Name(Identity(RandomAlphaNumericString(64)));
}

std::vector<VersionName> AddBranch(StructuredDataVersions& versions, VersionName old_version,
                                   uint32_t start_index, uint32_t count) {
  std::vector<VersionName> branch(1, old_version);
  for (uint32_t i(0); i != count; ++i) {
    VersionName new_version(start_index + i, RandomId());
    versions.Put(old_version, new_version);
    branch.push_back(new_version);
    old_version = new_version;
  }
  return branch;
}

std::string DisplayVersion(const VersionName& version, bool to_hex) {
  return std::to_string(version.index) + "-" +
         (version.id->IsInitialised()
              ? (to_hex ? HexEncode(version.id.value) : version.id->string()).substr(0, 3)
              : ("Uninitialised"));
}

std::string DisplayVersions(const std::vector<VersionName>& versions, bool to_hex) {
  std::string result;
  for (const auto& version : versions)
    result += DisplayVersion(version, to_hex) + "  ";
  return result;
}

void ConstructAsDiagram(StructuredDataVersions& versions) {
  /*   7-yyy       0-aaa
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
                        5-nnn                        */
  VersionName v0_aaa(0, ImmutableData::Name(Identity(std::string(64, 'a'))));
  VersionName v1_bbb(1, ImmutableData::Name(Identity(std::string(64, 'b'))));
  VersionName v2_ccc(2, ImmutableData::Name(Identity(std::string(64, 'c'))));
  VersionName v2_ddd(2, ImmutableData::Name(Identity(std::string(64, 'd'))));
  VersionName v2_eee(2, ImmutableData::Name(Identity(std::string(64, 'e'))));
  VersionName v3_fff(3, ImmutableData::Name(Identity(std::string(64, 'f'))));
  VersionName v3_ggg(3, ImmutableData::Name(Identity(std::string(64, 'g'))));
  VersionName v3_hhh(3, ImmutableData::Name(Identity(std::string(64, 'h'))));
  VersionName v4_iii(4, ImmutableData::Name(Identity(std::string(64, 'i'))));
  VersionName v4_jjj(4, ImmutableData::Name(Identity(std::string(64, 'j'))));
  VersionName v4_kkk(4, ImmutableData::Name(Identity(std::string(64, 'k'))));
  VersionName v4_lll(4, ImmutableData::Name(Identity(std::string(64, 'l'))));
  VersionName v4_mmm(4, ImmutableData::Name(Identity(std::string(64, 'm'))));
  VersionName v5_nnn(5, ImmutableData::Name(Identity(std::string(64, 'n'))));
  VersionName absent(6, ImmutableData::Name(Identity(std::string(64, 'x'))));
  VersionName v7_yyy(7, ImmutableData::Name(Identity(std::string(64, 'y'))));
  VersionName v8_zzz(8, ImmutableData::Name(Identity(std::string(64, 'z'))));
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

  std::random_device random_device;
  std::mt19937 generator(random_device());
  std::shuffle(std::begin(puts), std::end(puts), generator);
  for (const auto& put : puts) {
    LOG(kInfo) << "Putting - old: " << DisplayVersion(put.first, false)
               << "  new: " << DisplayVersion(put.second, false);
    versions.Put(put.first, put.second);
  }
}

bool Equivalent(const StructuredDataVersions& lhs, const StructuredDataVersions& rhs) {
  if (lhs.max_versions() != rhs.max_versions()) {
    LOG(kInfo) << "lhs.max_versions (" << lhs.max_versions()
               << ") != rhs.max_versions(" << rhs.max_versions() << ")";
    return false;
  }
  if (lhs.max_branches() != rhs.max_branches()) {
    LOG(kInfo) << "lhs.max_branches (" << lhs.max_branches()
               << ") != rhs.max_branches(" << rhs.max_branches() << ")";
    return false;
  }

  auto lhs_tots(lhs.Get());
  auto rhs_tots(rhs.Get());
  std::sort(std::begin(lhs_tots), std::end(lhs_tots));
  std::sort(std::begin(rhs_tots), std::end(rhs_tots));
  if (lhs_tots != rhs_tots) {
    LOG(kInfo) << "lhs.tips_of_trees != rhs.tips_of_trees:\n"
               << "lhs: " << DisplayVersions(lhs_tots, false)
               << "\n rhs: " << DisplayVersions(rhs_tots, false) << '\n';
    return false;
  }

  std::vector<std::vector<VersionName>> lhs_branches, rhs_branches;
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
    LOG(kInfo) << output;
    return false;
  }
  return true;
}

}  // unnamed namespace

TEST(StructuredDataVersionsTests, BEH_Put) {
  StructuredDataVersions versions(100, 10);
  VersionName old_version, new_version;
  for (uint32_t i(0); i != 100; ++i) {
    new_version = VersionName(i, RandomId());
    ASSERT_NO_THROW(versions.Put(old_version, new_version));
    if (i % 20 == 0 && i != 0) {
      for (uint32_t j(0); j != (i / 20); ++j)
        ASSERT_NO_THROW(AddBranch(versions, old_version, i, 20));
    }
    old_version = new_version;
  }
}

TEST(StructuredDataVersionsTests, BEH_PutOrphans) {
  StructuredDataVersions versions(1000, 100);
  VersionName old_version, new_version;
  std::vector<std::pair<VersionName, VersionName>> missing_names;
  for (uint32_t i(0); i != 100; ++i) {
    new_version = VersionName(i, RandomId());
    if (i % 20 == 0 && i != 0 && i != 20) {
      for (uint32_t j(0); j != (i / 20); ++j) {
        std::vector<VersionName> branch;
        ASSERT_NO_THROW(branch = AddBranch(versions, new_version, i, 20));
        ASSERT_NO_THROW(AddBranch(versions, branch[7], i + 7, 20));
        ASSERT_NO_THROW(AddBranch(versions, branch[14], i + 14, 20));
      }
      missing_names.push_back(std::make_pair(old_version, new_version));
    } else {
      ASSERT_NO_THROW(versions.Put(old_version, new_version));
    }
    old_version = new_version;
  }

  for (const auto& missing_name : missing_names) {
    ASSERT_NO_THROW(versions.Put(missing_name.first, missing_name.second));
  }
}

TEST(StructuredDataVersionsTests, BEH_Serialise) {
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

  ASSERT_TRUE(Equivalent(versions1, parsed1));
  ASSERT_TRUE(Equivalent(versions2, parsed2));
  ASSERT_TRUE(Equivalent(parsed1, parsed2));
  ASSERT_TRUE(Equivalent(versions3, parsed3));

  auto reserialised1(parsed1.Serialise());
  auto reserialised2(parsed2.Serialise());
  auto reserialised3(parsed3.Serialise());
  ASSERT_EQ(serialised1, reserialised1);
  ASSERT_EQ(serialised2, reserialised2);
  ASSERT_EQ(reserialised1, reserialised2);
  ASSERT_EQ(serialised3, reserialised3);
}

TEST(StructuredDataVersionsTests, BEH_ApplySerialised) {
  StructuredDataVersions versions1(100, 20);
  ASSERT_NO_THROW(ConstructAsDiagram(versions1));
  auto serialised1(versions1.Serialise());
  // Check applying all included versions doesn't modify the SDV.
  ASSERT_NO_THROW(versions1.ApplySerialised(serialised1));
  auto temp_serialised(versions1.Serialise());
  ASSERT_EQ(serialised1, temp_serialised);

  // Construct SDV with only "absent" version from diagram included.
  VersionName v5_nnn(5, ImmutableData::Name(Identity(std::string(64, 'n'))));
  VersionName absent(6, ImmutableData::Name(Identity(std::string(64, 'x'))));
  StructuredDataVersions versions2(100, 20);
  ASSERT_NO_THROW(versions2.Put(v5_nnn, absent));
  auto serialised2(versions2.Serialise());

  // Apply each serialised SDV to the other and check they produce the same resultant SDV.
  ASSERT_NO_THROW(versions1.ApplySerialised(serialised2));
  ASSERT_NO_THROW(versions2.ApplySerialised(serialised1));
  ASSERT_TRUE(Equivalent(versions1, versions2));
}

TEST(StructuredDataVersionsTests, BEH_SerialisationOptionalFieldTest) {
  using Sdv_t = maidsafe::detail::Sdv_t;
  using SdvBr_t = maidsafe::detail::SdvBr_t;
  using Ver_t = maidsafe::detail::Ver_t;

  Ver_t ver_0, ver_1, ver_2, ver_3;
  ver_0.index_ = ver_1.index_ = ver_2.index_ = ver_3.index_ = 100;
  ver_0.id_ = ver_1.id_ = ver_2.id_ = ver_3.id_ = "Data";

  ver_0.forking_child_count_ = ver_2.forking_child_count_ = 33;

  EXPECT_TRUE(ver_0.forking_child_count_ && ver_2.forking_child_count_);
  EXPECT_FALSE(ver_1.forking_child_count_ || ver_3.forking_child_count_);

  Sdv_t sdv_lhs, sdv_rhs;
  sdv_lhs.max_versions_ = sdv_rhs.max_versions_ = 20;
  sdv_lhs.max_branches_ = sdv_rhs.max_branches_ = 10;

  SdvBr_t sdv_br_0, sdv_br_1;

  sdv_br_0.absent_parent_ = ver_0;
  sdv_br_0.name_.push_back(ver_3);

  sdv_br_1.name_.push_back(ver_2);

  sdv_lhs.branch_.push_back(sdv_br_0);
  sdv_rhs.branch_.push_back(sdv_br_1);

  EXPECT_FALSE(sdv_lhs == sdv_rhs);
  // Lhs has optional absent_parent_, Rhs has optional absent_parent_ missing
  EXPECT_FALSE(sdv_lhs.branch_[0].absent_parent_ == sdv_rhs.branch_[0].absent_parent_);
  // Lhs has optional forking_child_count_ missing, Rhs has optional forking_child_count_
  EXPECT_FALSE(sdv_lhs.branch_[0].name_[0] == sdv_rhs.branch_[0].name_[0]);

  // Serialise
  std::string str_serialised_0 {ConvertToString(sdv_lhs)};

  // Deserialise
  ConvertFromString(str_serialised_0, sdv_rhs);
  // Rhs has optional absent_parent_ restored by presence of it in stream.
  // Rhs has optional forking_child_count_ forced absent by absence of it in stream.
  EXPECT_TRUE(sdv_lhs == sdv_rhs);

  // Reserialise
  std::string str_serialised_1 {ConvertToString(sdv_rhs)};
  EXPECT_TRUE(str_serialised_0 == str_serialised_1);
}

}  // namespace test

}  // namespace maidsafe
