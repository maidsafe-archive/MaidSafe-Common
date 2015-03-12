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

#include "maidsafe/common/data_types/structured_data_versions.h"

#include <algorithm>
#include <limits>
#include <tuple>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"

#include "maidsafe/common/serialisation/serialisation.h"
#include "maidsafe/common/data_types/structured_data_versions_cereal.h"

namespace maidsafe {

StructuredDataVersions::VersionName::VersionName()
    : index(std::numeric_limits<Index>::max()), id(), forking_child_count() {}

StructuredDataVersions::VersionName::VersionName(Index index_in, Id id_in)
    : index(index_in), id(std::move(id_in)), forking_child_count() {}

StructuredDataVersions::VersionName::VersionName(VersionName&& other) MAIDSAFE_NOEXCEPT
    : index(std::move(other.index)),
      id(std::move(other.id)),
      forking_child_count(std::move(other.forking_child_count)) {}

StructuredDataVersions::VersionName& StructuredDataVersions::VersionName::operator=(
    VersionName other) {
  swap(*this, other);
  return *this;
}

void swap(StructuredDataVersions::VersionName& lhs,
          StructuredDataVersions::VersionName& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.index, rhs.index);
  swap(lhs.id, rhs.id);
  swap(lhs.forking_child_count, rhs.forking_child_count);
}

bool operator==(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs) {
  // Don't consider forking_child_count, since that's only used for serialising/parsing
  return std::tie(lhs.index, lhs.id) == std::tie(rhs.index, rhs.id);
}

bool operator!=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs) {
  return !operator==(lhs, rhs);
}

bool operator<(const StructuredDataVersions::VersionName& lhs,
               const StructuredDataVersions::VersionName& rhs) {
  // Don't consider forking_child_count, since that's only used for serialising/parsing
  return std::tie(lhs.index, lhs.id) < std::tie(rhs.index, rhs.id);
}

bool operator>(const StructuredDataVersions::VersionName& lhs,
               const StructuredDataVersions::VersionName& rhs) {
  return operator<(rhs, lhs);
}

bool operator<=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs) {
  return !operator>(lhs, rhs);
}

bool operator>=(const StructuredDataVersions::VersionName& lhs,
                const StructuredDataVersions::VersionName& rhs) {
  return !operator<(lhs, rhs);
}

StructuredDataVersions::Details::Details()
    : parent(), children([](VersionsItr lhs, VersionsItr rhs) { return *lhs < *rhs; }) {}

StructuredDataVersions::Details::Details(VersionsItr parent_in)
    : parent(std::move(parent_in)),
      children([](VersionsItr lhs, VersionsItr rhs) { return *lhs < *rhs; }) {}

StructuredDataVersions::Details::Details(const Details& other)
    : parent(other.parent), children(other.children) {}

StructuredDataVersions::Details::Details(Details&& other)
    : parent(std::move(other.parent)), children(std::move(other.children)) {}

StructuredDataVersions::Details& StructuredDataVersions::Details::operator=(Details other) {
  swap(*this, other);
  return *this;
}

void swap(StructuredDataVersions::Details& lhs,
          StructuredDataVersions::Details& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.parent, rhs.parent);
  swap(lhs.children, rhs.children);
}

StructuredDataVersions::StructuredDataVersions(uint32_t max_versions, uint32_t max_branches)
    : max_versions_(max_versions),
      max_branches_(max_branches),
      versions_(),
      root_(std::make_pair(VersionName(), std::end(versions_))),
      tips_of_trees_([](VersionsItr lhs, VersionsItr rhs) { return *lhs < *rhs; }),
      orphans_() {
  ValidateLimits();
}

StructuredDataVersions& StructuredDataVersions::operator=(StructuredDataVersions other) {
  swap(*this, other);
  return *this;
}

StructuredDataVersions::StructuredDataVersions(const serialised_type& serialised_data_versions)
    : max_versions_(),
      max_branches_(),
      versions_(),
      root_(std::make_pair(VersionName(), std::end(versions_))),
      tips_of_trees_([](VersionsItr lhs, VersionsItr rhs) { return *lhs < *rhs; }),
      orphans_() {
  detail::StructuredDataVersionsCereal serialised_versions;
  try {
    Parse(serialised_data_versions->string(), serialised_versions);
  } catch (...) {
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }

  max_versions_ = serialised_versions.max_versions;
  max_branches_ = serialised_versions.max_branches;
  ValidateLimits();

  std::size_t serialised_branch_index(0);
  while (serialised_branch_index < serialised_versions.branches.size())
    BranchFromCereal(std::end(versions_), serialised_versions, serialised_branch_index);

  if (versions_.size() > max_versions_ || tips_of_trees_.size() > max_branches_)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
}

StructuredDataVersions::serialised_type StructuredDataVersions::Serialise() const {
  detail::StructuredDataVersionsCereal serialised_versions;
  serialised_versions.max_versions = max_versions_;
  serialised_versions.max_branches = max_branches_;

  BranchToCereal(root_.second, serialised_versions, root_.first);
  for (const auto& orphan_set : orphans_) {
    for (const auto& orphan : orphan_set.second)
      BranchToCereal(orphan, serialised_versions, orphan_set.first);
  }

  return serialised_type(NonEmptyString(maidsafe::Serialise(std::move(serialised_versions))));
}

void StructuredDataVersions::ValidateLimits() const {
  if (max_versions_ < 1U || max_branches_ < 1U)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
}

void StructuredDataVersions::BranchFromCereal(
    VersionsItr parent_itr, detail::StructuredDataVersionsCereal& serialised_versions,
    std::size_t& serialised_branch_index) {
  if (serialised_branch_index >= serialised_versions.branches.size())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));

  // Handle first version in branch
  auto& serialised_branch(serialised_versions.branches[serialised_branch_index]);
  auto forking_child_count(serialised_branch.names.front().forking_child_count);
  auto itr(HandleFirstVersionInBranchFromCereal(parent_itr, serialised_branch));

  // Handle other versions in branch
  std::size_t serialised_version_index(1);
  for (; serialised_version_index < serialised_branch.names.size(); ++serialised_version_index) {
    auto previous_itr(itr);
    forking_child_count = serialised_branch.names[serialised_version_index].forking_child_count;
    itr = CheckedInsert(std::move(serialised_branch.names[serialised_version_index]));
    CheckedInsert(previous_itr->second->children, itr);
    itr->second->parent = previous_itr;
  }
  ++serialised_branch_index;

  // Handle continuation forks or mark as tip of tree.
  if (forking_child_count) {
    if (*forking_child_count < 2U)
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    for (uint32_t i(0); i != *forking_child_count; ++i)
      BranchFromCereal(itr, serialised_versions, serialised_branch_index);
  } else {
    CheckedInsert(tips_of_trees_, itr);
  }
}

StructuredDataVersions::VersionsItr StructuredDataVersions::HandleFirstVersionInBranchFromCereal(
    VersionsItr parent_itr, detail::StructuredDataVersionsBranchCereal& serialised_branch) {
  auto itr(CheckedInsert(std::move(serialised_branch.names[0])));
  if (parent_itr == std::end(versions_)) {
    // This is a new branch, so the first element is either root_ or an orphan.
    itr->second->parent = std::end(versions_);
    VersionName absent_parent;
    if (serialised_branch.absent_parent) {
      absent_parent.index = serialised_branch.absent_parent->index;
      absent_parent.id = serialised_branch.absent_parent->id;
    }
    if (root_.second == std::end(versions_)) {
      // Mark as root
      root_.first = absent_parent;
      root_.second = itr;
    } else {
      // Mark as orphan
      if (!absent_parent.id.IsInitialised())
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
      InsertOrphan(absent_parent, itr);
    }
  } else {
    // This is a continuation fork of an existing branch.
    CheckedInsert(parent_itr->second->children, itr);
    itr->second->parent = parent_itr;
  }
  return itr;
}

StructuredDataVersions::VersionsItr StructuredDataVersions::CheckedInsert(VersionName&& version) {
  auto result(versions_.insert(std::make_pair(std::move(version), std::make_shared<Details>())));
  if (!result.second)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  return result.first;
}

void StructuredDataVersions::BranchToCereal(
    VersionsItr itr, detail::StructuredDataVersionsCereal& serialised_versions,
    const VersionName& absent_parent) const {
  if (itr == std::end(versions_))
    return;
  serialised_versions.branches.emplace_back();
  auto serialised_branch(&serialised_versions.branches.back());
  if (absent_parent.id.IsInitialised())
    serialised_branch->absent_parent = absent_parent;

  BranchToCereal(itr, serialised_versions, serialised_branch);
}

void StructuredDataVersions::BranchToCereal(
    VersionsItr itr, detail::StructuredDataVersionsCereal& serialised_versions,
    detail::StructuredDataVersionsBranchCereal* serialised_branch) const {
  for (;;) {
    if (itr == std::end(versions_))
      return;
    serialised_branch->names.emplace_back(itr->first.index, itr->first.id);
    if (itr->second->children.empty())
      return;


    if (itr->second->children.size() == 1U) {
      itr = *std::begin(itr->second->children);
    } else {
      auto added_version(&serialised_branch->names.back());
      added_version->forking_child_count = static_cast<uint32_t>(itr->second->children.size());
      for (auto child : itr->second->children) {
        serialised_versions.branches.emplace_back();
        BranchToCereal(child, serialised_versions, &serialised_versions.branches.back());
      }
      return;
    }
  }
}

void StructuredDataVersions::ApplySerialised(const serialised_type& serialised_data_versions) {
  StructuredDataVersions new_info(serialised_data_versions);
  ApplyBranch(root_.first, root_.second, new_info);
  for (const auto& orphan_set : orphans_) {
    for (const auto& orphan : orphan_set.second)
      ApplyBranch(orphan_set.first, orphan, new_info);
  }
  swap(*this, new_info);
}

void StructuredDataVersions::ApplyBranch(VersionName parent, VersionsItr itr,
                                         StructuredDataVersions& new_versions) const {
  for (;;) {
    new_versions.Put(parent, itr->first);
    if (itr->second->children.empty())
      return;
    parent = itr->first;
    if (itr->second->children.size() == 1U) {
      itr = *std::begin(itr->second->children);
    } else {
      for (auto child : itr->second->children)
        ApplyBranch(parent, child, new_versions);
      return;
    }
  }
}

boost::optional<StructuredDataVersions::VersionName> StructuredDataVersions::Put(
    const VersionName& old_version, const VersionName& new_version) {
  if (!new_version.id.IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));

  if (NewVersionPreExists(old_version, new_version))
    return boost::none;

  // Check we've not been asked to store two roots.
  bool is_root(!old_version.id.IsInitialised() || versions_.empty() || new_version.index == 0);
  if (is_root && root_.second != std::end(versions_) && !RootParentName().id.IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));

  // Construct temp objects before modifying members in case exception is thrown.
  Version version(std::make_pair(
      new_version,
      std::make_shared<Details>(is_root ? std::end(versions_) : versions_.find(old_version))));
  bool is_orphan(version.second->parent == std::end(versions_) && !is_root);
  bool unorphans_existing_root(false);
  size_t unorphan_count;
  CheckForUnorphaning(version, unorphans_existing_root, unorphan_count);

  // If there's a root version with index of 0 and this has passed 'old_version' with index of 0,
  // check this call isn't implying two different roots
  if (is_orphan && root_.second->first.index == 0 && old_version.index == 0 &&
      root_.second->first.id != old_version.id) {
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }

  // Handle case where we're about to exceed 'max_versions_'.
  bool erase_existing_root(false);
  if (AtVersionsLimit()) {
    if (unorphans_existing_root || is_root)
      // This new version would become 'root_', only to be immediately erased to bring version count
      // back down to 'max_versions_'.
      return boost::make_optional(new_version);
    erase_existing_root = true;
  }

  // Handle case where we're about to exceed 'max_branches_'.
  CheckBranchCount(version, is_orphan, unorphan_count, erase_existing_root);

  // Finally, safe to now add details
  return Insert(version, is_root, is_orphan, old_version, unorphans_existing_root, unorphan_count,
                erase_existing_root);
}

StructuredDataVersions::VersionName StructuredDataVersions::ParentName(VersionsItr itr) const {
  return itr->second->parent->first;
}

StructuredDataVersions::VersionName StructuredDataVersions::ParentName(
    Versions::const_iterator itr) const {
  return itr->second->parent->first;
}

StructuredDataVersions::VersionName StructuredDataVersions::RootParentName() const {
  return root_.first;
}

bool StructuredDataVersions::NewVersionPreExists(const VersionName& old_version,
                                                 const VersionName& new_version) const {
  auto existing_itr(versions_.find(new_version));
  if (existing_itr != std::end(versions_)) {
    if (existing_itr->second->parent == std::end(versions_)) {
      // This is root or an orphan
      if (new_version == root_.second->first) {
        if (root_.first == old_version)
          return true;
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
      }

      auto orphan_itr(FindOrphan(new_version));
      assert(orphan_itr.first != std::end(orphans_) &&
             orphan_itr.second != std::end(orphan_itr.first->second));
      if (orphan_itr.first != std::end(orphans_) && orphan_itr.first->first == old_version)
        return true;
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
    } else {
      if (ParentName(existing_itr) == old_version)
        return true;
      else
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
    }
  }
  return false;
}

void StructuredDataVersions::CheckForUnorphaning(Version& version, bool& unorphans_existing_root,
                                                 size_t& unorphan_count) const {
  auto orphans_itr(orphans_.find(version.first));
  unorphan_count = (orphans_itr == std::end(orphans_) ? 0 : orphans_itr->second.size());
  std::vector<std::future<void>> check_futures;
  const VersionName* const version_parent(version.second->parent != std::end(versions_) &&
                                                  version.second->parent->first.id.IsInitialised()
                                              ? &version.second->parent->first
                                              : nullptr);
  if (unorphan_count) {
    for (auto orphan_itr(std::begin(orphans_itr->second));
         orphan_itr != std::end(orphans_itr->second); ++orphan_itr) {
      // Check we can't iterate back to ourself (avoid circular parent-child chain)
      if (version_parent)
        check_futures.push_back(CheckVersionNotInBranch(*orphan_itr, *version_parent));
      CheckedInsert(version.second->children, *orphan_itr);
    }
  }
  unorphans_existing_root = (root_.first.id.IsInitialised() && RootParentName() == version.first);
  if (unorphans_existing_root) {
    if (version_parent)
      check_futures.push_back(CheckVersionNotInBranch(root_.second, *version_parent));
    CheckedInsert(version.second->children, root_.second);
  }
  for (auto& check_future : check_futures)
    check_future.get();
}

std::future<void> StructuredDataVersions::CheckVersionNotInBranch(
    VersionsItr itr, const VersionName& version) const {
  return std::async([this, itr, &version]() {
    VersionsItr versions_itr(itr);
    while (versions_itr != std::end(versions_)) {
      auto child_itr(std::begin(versions_itr->second->children));
      if (child_itr == std::end(versions_itr->second->children))
        return;
      if ((*child_itr)->first == version)
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
      std::vector<std::future<void>> check_futures;
      while (++child_itr != std::end(versions_itr->second->children))
        check_futures.emplace_back(CheckVersionNotInBranch(*child_itr, version));
      versions_itr = *std::begin(versions_itr->second->children);
      for (auto& check_future : check_futures)
        check_future.get();
    }
  });
}

void StructuredDataVersions::CheckBranchCount(const Version& version, bool is_orphan,
                                              size_t unorphaned_count,
                                              bool& erase_existing_root) const {
  if (AtBranchesLimit() && unorphaned_count == 0) {
    if (is_orphan || !version.second->parent->second->children.empty()) {
      // We're going to exceed limit - see if deleting 'root_' helps
      bool root_is_tip_of_tree(root_.second != std::end(versions_) &&
                               root_.second->second->children.empty());
      if (root_is_tip_of_tree)
        erase_existing_root = true;
      else
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::cannot_exceed_limit));
    }
  }
}

boost::optional<StructuredDataVersions::VersionName> StructuredDataVersions::Insert(
    const Version& version, bool is_root, bool is_orphan, const VersionName& old_version,
    bool unorphans_existing_root, size_t unorphan_count, bool erase_existing_root) {
  assert(!((is_root || unorphans_existing_root) && erase_existing_root));

  auto inserted_itr(versions_.insert(version).first);
  if (unorphan_count)
    Unorphan(inserted_itr);

  if (!is_root && !is_orphan)
    SetVersionAsChildOfItsParent(inserted_itr);

  if (is_orphan && !unorphans_existing_root)
    InsertOrphan(old_version, inserted_itr);

  if (is_root && root_.first.id.IsInitialised() && !unorphans_existing_root) {
    // The new root is replacing a temporary old root which would have been an orphan had the real
    // root existed at that time.  Move the old root to 'orphans_'.
    InsertOrphan(root_.first, root_.second);
  }

  boost::optional<VersionName> removed_version;
  if (is_root) {
    if (unorphans_existing_root)
      UnorphanRoot(inserted_itr, true, old_version);
    else
      root_ = std::make_pair(old_version, inserted_itr);
  } else if (unorphans_existing_root) {
    UnorphanRoot(inserted_itr, is_orphan, old_version);
  } else if (erase_existing_root) {
    removed_version = root_.second->first;
    ReplaceRoot();
  }

  if (version.second->children.empty())
    CheckedInsert(tips_of_trees_, inserted_itr);

  assert(versions_.size() <= max_versions_ && tips_of_trees_.size() <= max_branches_);
  return removed_version;
}

void StructuredDataVersions::SetVersionAsChildOfItsParent(VersionsItr versions_itr) {
  auto& parents_children(versions_itr->second->parent->second->children);
  if (parents_children.empty()) {
    auto tip_of_tree_itr(std::find_if(
        std::begin(tips_of_trees_), std::end(tips_of_trees_),
        [this, versions_itr](VersionsItr tot) { return tot->first == ParentName(versions_itr); }));
    assert(tip_of_tree_itr != std::end(tips_of_trees_));
    tips_of_trees_.erase(tip_of_tree_itr);
  }
  CheckedInsert(parents_children, versions_itr);
}

void StructuredDataVersions::UnorphanRoot(VersionsItr parent, bool is_root_or_orphan,
                                          const VersionName& old_version) {
  root_.second->second->parent = parent;
  if (is_root_or_orphan) {
    root_ = std::make_pair(old_version, parent);
  } else {
    // Find the start of the current branch - must be an orphan.
    auto new_root(parent);
    while (new_root->second->parent != std::end(versions_))
      new_root = new_root->second->parent;
    auto orphan_itr(FindOrphan(new_root->first));
    assert(orphan_itr.first != std::end(orphans_));
    if (orphan_itr.first == std::end(orphans_))
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));
    // Move from orphans to root_
    root_.first = orphan_itr.first->first;
    root_.second = *orphan_itr.second;
    EraseOrphan(orphan_itr);
  }
}

void StructuredDataVersions::Unorphan(VersionsItr parent) {
  auto orphans_itr(orphans_.find(parent->first));
  assert(orphans_itr != std::end(orphans_));
  if (orphans_itr == std::end(orphans_))
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unknown));

  for (auto orphan : orphans_itr->second)
    orphan->second->parent = parent;

  orphans_.erase(orphans_itr);
}

void StructuredDataVersions::ReplaceRoot() {
  // Remove current root from 'tips_of_trees_'.
  auto tip_of_tree_itr(FindBranchTip(root_.second->first));
  if (tip_of_tree_itr != std::end(tips_of_trees_))
    tips_of_trees_.erase(tip_of_tree_itr);

  if (root_.second->second->children.empty())
    ReplaceRootFromOrphans();
  else
    ReplaceRootFromChildren();
}

void StructuredDataVersions::ReplaceRootFromOrphans() {
  assert(!orphans_.empty());
  OrphanItr orphan_itr(
      std::make_pair(std::begin(orphans_), std::begin(std::begin(orphans_)->second)));
  versions_.erase(root_.second);
  root_.first = orphan_itr.first->first;
  root_.second = *orphan_itr.second;
  EraseOrphan(orphan_itr);
}

void StructuredDataVersions::ReplaceRootFromChildren() {
  // Create orphans and find replacement from current root's children.
  auto child_itr(std::begin(root_.second->second->children));
  auto current_root_name(root_.second->first);
  auto replacement_root(std::make_pair(current_root_name, *child_itr));
  replacement_root.second->second->parent = std::end(versions_);

  while (++child_itr != std::end(root_.second->second->children)) {
    (*child_itr)->second->parent = std::end(versions_);
    InsertOrphan(current_root_name, *child_itr);
  }

  versions_.erase(root_.second);
  root_ = replacement_root;
}

StructuredDataVersions::SortedVersionsItrs::const_iterator StructuredDataVersions::FindBranchTip(
    const VersionName& name) const {
  return std::find_if(std::begin(tips_of_trees_), std::end(tips_of_trees_),
                      [&name](VersionsItr branch_tip) { return branch_tip->first == name; });
}

StructuredDataVersions::OrphanItr StructuredDataVersions::FindOrphan(const VersionName& name) {
  OrphanItr orphan_itr;
  orphan_itr.first = std::begin(orphans_);
  while (orphan_itr.first != std::end(orphans_)) {
    orphan_itr.second = std::find_if(
        std::begin(orphan_itr.first->second), std::end(orphan_itr.first->second),
        [&name](SortedVersionsItrs::value_type version) { return version->first == name; });
    if (orphan_itr.second != std::end(orphan_itr.first->second))
      break;
    ++orphan_itr.first;
  }
  return orphan_itr;
}

StructuredDataVersions::OrphanConstItr StructuredDataVersions::FindOrphan(
    const VersionName& name) const {
  OrphanConstItr orphan_itr;
  orphan_itr.first = std::begin(orphans_);
  while (orphan_itr.first != std::end(orphans_)) {
    orphan_itr.second = std::find_if(
        std::begin(orphan_itr.first->second), std::end(orphan_itr.first->second),
        [&name](SortedVersionsItrs::value_type version) { return version->first == name; });
    if (orphan_itr.second != std::end(orphan_itr.first->second))
      break;
    ++orphan_itr.first;
  }
  return orphan_itr;
}

void StructuredDataVersions::InsertOrphan(const VersionName& absent_parent_name,
                                          VersionsItr orphan) {
  // This either creates a new entry in the orphans map, or gives the existing one for parent.
  auto orphans_itr(
      orphans_.insert(std::make_pair(absent_parent_name,
                                     SortedVersionsItrs([](VersionsItr lhs, VersionsItr rhs) {
                                       return *lhs < *rhs;
                                     }))).first);
  orphans_itr->second.insert(orphan);
}

void StructuredDataVersions::EraseOrphan(OrphanItr orphan_itr) {
  assert(orphan_itr.first != std::end(orphans_) &&
         orphan_itr.second != std::end(orphan_itr.first->second));
  orphan_itr.first->second.erase(orphan_itr.second);
  if (orphan_itr.first->second.empty())
    orphans_.erase(orphan_itr.first);
}

std::vector<StructuredDataVersions::VersionName> StructuredDataVersions::Get() const {
  std::vector<StructuredDataVersions::VersionName> result;
  for (const auto& tot : tips_of_trees_) {
    assert(tot->second->children.empty());
    result.push_back(tot->first);
  }
  std::reverse(std::begin(result), std::end(result));
  return result;
}

std::vector<StructuredDataVersions::VersionName> StructuredDataVersions::GetBranch(
    const VersionName& branch_tip) const {
  auto branch_tip_itr(FindBranchTip(branch_tip));
  CheckBranchTipIterator(branch_tip, branch_tip_itr);
  auto itr(*branch_tip_itr);
  std::vector<StructuredDataVersions::VersionName> result;
  while (itr != std::end(versions_)) {
    result.push_back(itr->first);
    itr = itr->second->parent;
  }
  return result;
}

void StructuredDataVersions::CheckBranchTipIterator(
    const VersionName& name, SortedVersionsItrs::const_iterator branch_tip_itr) const {
  if (branch_tip_itr == std::end(tips_of_trees_)) {
    if (versions_.find(name) == std::end(versions_))
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::no_such_element));
    else
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_argument));
  }
}

void StructuredDataVersions::DeleteBranchUntilFork(const VersionName& branch_tip) {
  auto branch_tip_itr(FindBranchTip(branch_tip));
  CheckBranchTipIterator(branch_tip, branch_tip_itr);
  auto itr(*branch_tip_itr);
  tips_of_trees_.erase(branch_tip_itr);

  for (;;) {
    auto parent_itr = itr->second->parent;
    if (parent_itr == std::end(versions_))  // Found root or orphan.
      return EraseFrontOfBranch(itr);
    auto parents_child_itr(std::find_if(
        std::begin(parent_itr->second->children), std::end(parent_itr->second->children),
        [itr](VersionsItr child_itr) { return itr->first == child_itr->first; }));
    assert(parents_child_itr != std::end(parent_itr->second->children));
    parent_itr->second->children.erase(parents_child_itr);
    versions_.erase(itr);
    if (!parent_itr->second->children.empty())  // Found fork.
      return;

    itr = parent_itr;
  }
}

void StructuredDataVersions::EraseFrontOfBranch(VersionsItr front_of_branch) {
  if (root_.second == front_of_branch) {  // Front of branch is 'root_'.
    if (orphans_.empty()) {
      versions_.erase(front_of_branch);
      root_ = std::make_pair(VersionName(), std::end(versions_));
      assert(versions_.empty() && tips_of_trees_.empty());
    } else {
      ReplaceRootFromOrphans();
    }
  } else {  // Front of branch is an orphan.
    EraseOrphan(FindOrphan(front_of_branch->first));
    versions_.erase(front_of_branch);
  }
}

void StructuredDataVersions::clear() {
  versions_.clear();
  root_ = std::make_pair(VersionName(), std::end(versions_));
  tips_of_trees_.clear();
  orphans_.clear();
}

bool StructuredDataVersions::AtVersionsLimit() const {
  assert(versions_.size() <= max_versions_);
  return versions_.size() == max_versions_;
}

bool StructuredDataVersions::AtBranchesLimit() const {
  assert(tips_of_trees_.size() <= max_branches_);
  return tips_of_trees_.size() == max_branches_;
}

void StructuredDataVersions::CheckedInsert(SortedVersionsItrs& container,
                                           VersionsItr element) const {
  auto result(container.insert(element));
  assert(result.second);
  static_cast<void>(result);
}

void swap(StructuredDataVersions& lhs, StructuredDataVersions& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.max_versions_, rhs.max_versions_);
  swap(lhs.max_branches_, rhs.max_branches_);
  // std::map::swap invalidates end iterators.  root_ and orphans_ have parents which are
  // end iterators - these need reset after the call to swap(lhs.versions_, rhs.versions_).  Also,
  // root_ may not yet have been set, in which case root_.second is an end iterator.
  bool lhs_root_previously_not_set(lhs.root_.second == std::end(lhs.versions_));
  bool rhs_root_previously_not_set(rhs.root_.second == std::end(rhs.versions_));
  swap(lhs.versions_, rhs.versions_);
  swap(lhs.root_, rhs.root_);

  if (lhs_root_previously_not_set)
    rhs.root_.second = std::end(rhs.versions_);
  else
    rhs.root_.second->second->parent = std::end(rhs.versions_);
  if (rhs_root_previously_not_set)
    lhs.root_.second = std::end(lhs.versions_);
  else
    lhs.root_.second->second->parent = std::end(lhs.versions_);

  swap(lhs.tips_of_trees_, rhs.tips_of_trees_);

  swap(lhs.orphans_, rhs.orphans_);
  for (const auto& lhs_orphans : lhs.orphans_) {
    for (auto lhs_orphan : lhs_orphans.second)
      lhs_orphan->second->parent == std::end(lhs.versions_);
  }
  for (const auto& rhs_orphans : rhs.orphans_) {
    for (auto rhs_orphan : rhs_orphans.second)
      rhs_orphan->second->parent == std::end(rhs.versions_);
  }
}

}  // namespace maidsafe
