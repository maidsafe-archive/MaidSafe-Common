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

/*

     7-yyy       0-aaa
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

The tree above represents the map of Versions with each node representing a different VersionName.
In the diagram, '0-aaa' is the first version (root) and has no parent (parent == end()), but is not
an orphan.  '7-yyy' is an orphan.

'0-aaa' is the parent of '1-bbb' and has a child count of 1.  '1-bbb' is the parent of '2-ccc',
'2-ddd' and '2-eee' and has a child count of 3.

All versions other than the root ('0-aaa') without a parent are orphans.  There will always only be
one root.  If the current root is erased, a new root is chosen from the remaining versions.  This
will be the child of the deleted root, or if the entire branch containing the root was erased, an
orphan will be chosen.

The "tips of trees" are '8-zzz', '4-iii', '5-nnn', '4-lll' and '4-mmm'.
*/

#ifndef MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_H_
#define MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_H_

#include <algorithm>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <utility>
#include <vector>
#include <string>

#include "boost/optional/optional.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/data_types/immutable_data.h"

namespace maidsafe {

namespace detail {

struct StructuredDataVersionsCereal;
struct StructuredDataVersionsBranchCereal;
struct VersionCereal;

}  // namespace detail

// All public functions in this class provide the strong exception guarantee.
class StructuredDataVersions {
 private:
  struct StructuredDataVersionsTag;

 public:
  struct VersionName {
    VersionName();
    VersionName(uint64_t index_in, ImmutableData::Name id_in);
    explicit VersionName(const std::string& serialised_version_name);
    VersionName(const VersionName& other);
    VersionName(VersionName&& other);
    VersionName& operator=(VersionName other);
    std::string Serialise() const;

    uint64_t index;
    ImmutableData::Name id;
  };

  typedef TaggedValue<NonEmptyString, StructuredDataVersionsTag> serialised_type;

  // Construct with a limit of 'max_versions' different versions and 'max_branches' different
  // branches (or "tips of trees").  Both must be >= 1 otherwise CommonErrors::invalid_parameter is
  // thrown.
  StructuredDataVersions(uint32_t max_versions, uint32_t max_branches);
  StructuredDataVersions& operator=(StructuredDataVersions other);
  friend void swap(StructuredDataVersions& lhs, StructuredDataVersions& rhs) MAIDSAFE_NOEXCEPT;

  explicit StructuredDataVersions(const serialised_type& serialised_data_versions);
  serialised_type Serialise() const;
  // This merges any existing data in the StructuredDataVersions (SDV) with that passed in
  // 'serialised_data_versions'.  It should be used to merge a resolved SDV into an existing SDV
  // at the end of an account transfer due to a churn event.  If the merge cannot be resolved, a
  // maidsafe_error will be thrown.  The values for 'max_versions' and 'max_branches' will be
  // overwritten with those in 'serialised_data_versions'.
  void ApplySerialised(const serialised_type& serialised_data_versions);

  // Inserts the 'new_version' into the map with 'old_version' as the parent.  Returns the version
  // which was removed if any.
  // * If 'old_version' doesn't exist in the tree, the version is added as an orphan.  For the root
  //   entry, 'old_version.id' should be uninitialised (a default-constructed VersionName will do).
  //   A root should only be provided once for a given SDV.  All non-root versions should have
  //   index > 0 and an initialised ID.
  // * If adding the version causes 'max_versions_' to be exceeded, the root will be erased and one
  //   of its immediate children assigned as the new root.  If the current root has > 1 children,
  //   the child with the lowest VersionName will be chosen.  If the root has no children, the
  //   orphan with the lowest VersionName will be chosen.
  // * If 'old_version.id' is uninitialised and the existing root's parent is uninitialised (i.e.
  //   two roots have deliberately been passed), CommonErrors::invalid_parameter is thrown.
  // * If adding the version causes 'max_branches_' to be exceeded, the root is considered for
  //   deletion.  If deletion avoids exceeding 'max_branches_', it's done, otherwise the root is
  //   left as is, and CommonErrors::cannot_exceed_limit is thrown.
  // * If 'new_version' already exists but with a different 'old_version' parent,
  //   CommonErrors::invalid_parameter is thrown.
  // * If inserting the new version causes a circular chain parent->child->parent,
  //   CommonErrors::invalid_parameter is thrown.
  boost::optional<VersionName> Put(const VersionName& old_version, const VersionName& new_version);
  // Returns all the "tips of trees" in order starting from least.
  std::vector<VersionName> Get() const;
  // Returns all the versions comprising a branch, starting at the tip, through to (including) the
  // root or the orphan at the start of that branch.  e.g., in the diagram above, GetBranch(4-jjj)
  // would return <4-jjj, 3-ggg, 2-ddd, 1-bbb, 0-aaa>.  GetBranch(5-nnn) would return
  // <5-nnn, 4-kkk, 3-ggg, 2-ddd, 1-bbb, 0-aaa>.  GetBranch(8-zzz) would return <8-zzz, 7-yyy>.
  // * If 'branch_tip' is not a "tip of tree" but does exist, CommonErrors::invalid_parameter is
  //   thrown.
  // * If 'branch_tip' doesn't exist, CommonErrors::no_such_element is thrown.
  std::vector<VersionName> GetBranch(const VersionName& branch_tip) const;
  // Similar to GetBranch except Versions are erased through to (excluding) the first version which
  // has > 1 child, or through to (including) the first version which has 0 children.  e.g. in the
  // diagram, DeleteBranchUntilFork(4-jjj) would erase 4-jjj only.  DeleteBranchUntilFork(5-nnn)
  // would erase <5-nnn, 4-kkk>.  DeleteBranchUntilFork(8-zzz) would erase <8-zzz, 7-yyy>.
  // * If 'branch_tip' is not a "tip of tree" but does exist, CommonErrors::invalid_parameter is
  //   thrown.
  // * If 'branch_tip' doesn't exist, CommonErrors::no_such_element is thrown.
  void DeleteBranchUntilFork(const VersionName& branch_tip);
  // Removes all versions from the container.
  void clear();

  uint32_t max_versions() const { return max_versions_; }
  uint32_t max_branches() const { return max_branches_; }

  // TODO(Fraser#5#): 2013-05-14 - Do we need another GetBranch function which allows start point
  //                  other than TOT, and/or a max_count number of versions to return?  Similarly
  //                  DeleteBranch or Delete x from root upwards.  Maybe also LockBranch function to
  //                  disallow further versions being added while a client is attempting to resolve
  //                  conflicts?

 private:
  StructuredDataVersions(const StructuredDataVersions& other);
  StructuredDataVersions(StructuredDataVersions&& other);

  struct Details;
  typedef std::map<VersionName, std::shared_ptr<Details>> Versions;
  typedef Versions::value_type Version;
  typedef Versions::iterator VersionsItr;
  typedef std::set<VersionsItr, std::function<bool(VersionsItr, VersionsItr)>> SortedVersionsItrs;
  // The first value of the pair is the "old version" or parent ID which the orphan was added under.
  // The expectation is that the missing parent will soon be added, allowing the second value(s) of
  // the pair to become "un-orphaned".
  typedef std::map<VersionName, SortedVersionsItrs> Orphans;
  typedef std::pair<Orphans::iterator, SortedVersionsItrs::iterator> OrphanItr;
  typedef std::pair<Orphans::const_iterator, SortedVersionsItrs::const_iterator> OrphanConstItr;

  struct Details {
    Details();
    explicit Details(VersionsItr parent_in);
    Details(const Details& other);
    Details(Details&& other);
    Details& operator=(Details other);

    VersionsItr parent;
    SortedVersionsItrs children;
  };
  friend void swap(Details& lhs, Details& rhs) MAIDSAFE_NOEXCEPT;

  void ValidateLimits() const;
  void BranchFromCereal(VersionsItr parent_itr,
                        const detail::StructuredDataVersionsCereal& serialised_versions,
                        std::size_t& serialised_branch_index);
  VersionsItr HandleFirstVersionInBranchFromCereal(
      VersionsItr parent_itr,
      const detail::StructuredDataVersionsBranchCereal& serialised_branch);

  VersionsItr CheckedInsert(const detail::VersionCereal& serialised_version);
  void BranchToCereal(VersionsItr itr,
                      detail::StructuredDataVersionsCereal& serialised_versions,
                      const VersionName& absent_parent) const;
  void BranchToCereal(VersionsItr itr,
                      detail::StructuredDataVersionsCereal& serialised_versions,
                      detail::StructuredDataVersionsBranchCereal* serialised_branch) const;

  void ApplyBranch(VersionName parent, VersionsItr itr,
                   StructuredDataVersions& new_versions) const;
  VersionName ParentName(VersionsItr itr) const;
  VersionName ParentName(Versions::const_iterator itr) const;
  VersionName RootParentName() const;
  bool NewVersionPreExists(const VersionName& old_version, const VersionName& new_version) const;
  void CheckForUnorphaning(Version& version, bool& unorphans_existing_root,
                           size_t& unorphan_count) const;
  std::future<void> CheckVersionNotInBranch(VersionsItr itr, const VersionName& version) const;
  void CheckBranchCount(const Version& version, bool is_orphan, size_t unorphaned_count,
                        bool& erase_existing_root) const;
  boost::optional<VersionName> Insert(const Version& version, bool is_root, bool is_orphan,
                                      const VersionName& old_version, bool unorphans_existing_root,
                                      size_t unorphan_count, bool erase_existing_root);
  void SetVersionAsChildOfItsParent(VersionsItr versions_itr);
  void UnorphanRoot(VersionsItr parent, bool is_root_or_orphan, const VersionName& old_version);
  void Unorphan(VersionsItr parent);
  void ReplaceRoot();
  void ReplaceRootFromOrphans();
  void ReplaceRootFromChildren();
  SortedVersionsItrs::const_iterator FindBranchTip(const VersionName& name) const;
  OrphanItr FindOrphan(const VersionName& name);
  OrphanConstItr FindOrphan(const VersionName& name) const;
  void InsertOrphan(const VersionName& absent_parent_name, VersionsItr orphan);
  void EraseOrphan(OrphanItr orphan_itr);
  void CheckBranchTipIterator(const VersionName& name,
                              SortedVersionsItrs::const_iterator branch_tip_itr) const;
  void EraseFrontOfBranch(VersionsItr front_of_branch);
  bool AtVersionsLimit() const;
  bool AtBranchesLimit() const;
  void CheckedInsert(SortedVersionsItrs& container, VersionsItr element) const;

  uint32_t max_versions_, max_branches_;
  Versions versions_;
  std::pair<VersionName, VersionsItr> root_;
  SortedVersionsItrs tips_of_trees_;
  Orphans orphans_;
};

void swap(StructuredDataVersions::VersionName& lhs,
          StructuredDataVersions::VersionName& rhs) MAIDSAFE_NOEXCEPT;

bool operator==(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs);

bool operator!=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs);

bool operator<(const StructuredDataVersions::VersionName& lhs,
               const StructuredDataVersions::VersionName& rhs);

bool operator>(const StructuredDataVersions::VersionName& lhs,
               const StructuredDataVersions::VersionName& rhs);

bool operator<=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs);

bool operator>=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs);

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_DATA_TYPES_STRUCTURED_DATA_VERSIONS_H_
