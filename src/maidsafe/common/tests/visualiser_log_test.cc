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

#include "maidsafe/common/visualiser_log.h"

#include "maidsafe/common/test.h"
#include "maidsafe/common/type_macros.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

DEFINE_OSTREAMABLE_ENUM_VALUES(TestPersona, int32_t,
    (MaidNode)
    (DataGetter)
    (CacheHandler))

DEFINE_OSTREAMABLE_ENUM_VALUES(TestAction, uint64_t,
    (Put)
    (Get)
    (AccountTransfer)
    (IncrementReferenceCount))

TEST_CASE("Visualiser Log", "[Log][Unit]") {
  CHECK(IsValid(TestPersona::kMaidNode));
  CHECK(IsValid(TestPersona::kDataGetter));
  CHECK(IsValid(TestPersona::kCacheHandler));
  CHECK(!IsValid(static_cast<TestPersona>(-1)));
  CHECK(!IsValid(static_cast<TestPersona>(3)));

  Identity target{ RandomString(64) };
  // Call before VlogPrefix has been set
  VLOG(TestPersona::kCacheHandler, TestAction::kAccountTransfer, target);

  // Call after VlogPrefix has been set
  log::Logging::Instance().SetVlogPrefix(DebugId(Identity{ RandomString(64) }));
  VLOG(TestPersona::kDataGetter, TestAction::kGet, target, target);
  VLOG(TestPersona::kDataGetter, TestAction::kGet, target);
  VLOG(TestPersona::kDataGetter, TestAction::kGet, 99);
  VLOG(TestAction::kGet, target, target);
  VLOG(TestAction::kGet, target);
  VLOG(TestAction::kGet, 99);

  CHECK_THROWS_AS(VLOG(TestPersona::kMaidNode, TestAction::kGet, Identity{}), common_error);
  CHECK_THROWS_AS(VLOG(TestPersona::kMaidNode, static_cast<TestAction>(-1), target), common_error);
  CHECK_THROWS_AS(VLOG(TestPersona::kMaidNode, static_cast<TestAction>(-1), 99), common_error);
  CHECK_THROWS_AS(VLOG(static_cast<TestPersona>(-1), TestAction::kGet, target), common_error);
  CHECK_THROWS_AS(VLOG(static_cast<TestPersona>(-1), TestAction::kGet, 99), common_error);
  CHECK_THROWS_AS(VLOG(TestAction::kGet, Identity{}), common_error);
  CHECK_THROWS_AS(VLOG(static_cast<TestAction>(-1), target), common_error);
  CHECK_THROWS_AS(VLOG(static_cast<TestAction>(-1), 99), common_error);

  // Try to set the VlogPrefix again
  CHECK_THROWS_AS(log::Logging::Instance().SetVlogPrefix("1"), common_error);
  VLOG(TestPersona::kMaidNode, TestAction::kIncrementReferenceCount, target);
}

}  // namespace test

}  // namespace maidsafe
