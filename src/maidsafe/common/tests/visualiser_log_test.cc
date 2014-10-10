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

#include <mutex>

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/on_scope_exit.h"
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

class VisualiserLogTest : public ::testing::Test {
 protected:
  VisualiserLogTest()
      : kThisVaultId_(InitId()),
        kTestSessionId_("54ca73ce-0c3c-4155-c9e3-c89d74ad5602"),
        kServerName_("visualiser.maidsafe.net"),
        kServerDir_("/testlog"),
        kServerPort_(8080) {}

  std::string GetPostRequestBody(const log::VisualiserLogMessage& vlog) {
    return vlog.GetPostRequestBody();
  }

  void DebugPrint(const log::VisualiserLogMessage& vlog) {
    LOG(kVerbose) << "\tts:                   \"" << vlog.kTimestamp_ << '\"';
    LOG(kVerbose) << "\tvault_id:             \"" << vlog.kVaultId_ << '\"';
    LOG(kVerbose) << "\tsession_id:           \"" << vlog.kSessionId_ << '\"';
    LOG(kVerbose) << "\tpersona_id:           \"" << vlog.kPersonaId_.value << '\"';
    LOG(kVerbose) << "\taction_id:            \"" << vlog.kActionId_.value << '\"';
    if (vlog.kValue1_.size() == crypto::SHA512::DIGESTSIZE)
      LOG(kVerbose) << "\tvalue1 (hex encoded): \"" << HexEncode(vlog.kValue1_) << '\"';
    else
      LOG(kVerbose) << "\tvalue1 (unencoded):   \"" << vlog.kValue1_ << '\"';
    LOG(kVerbose) << "\tvalue2 (hex encoded): \"" << HexEncode(vlog.kValue2_) << "\"\n";
  }

  static Identity InitId() {
    static Identity id{ RandomString(64) };
    return id;
  }

  std::unique_ptr<on_scope_exit> GetScopedSessionIdInvalidator() {
    const std::string kOriginalSessionId{ log::Logging::Instance().VlogSessionId() };
    auto scoped_invalidator(maidsafe::make_unique<on_scope_exit>([kOriginalSessionId] {
      log::Logging::Instance().visualiser_.session_id = kOriginalSessionId;
    }));
    log::Logging::Instance().visualiser_.session_id[0] = '6';
    return std::move(scoped_invalidator);
  }

  const Identity kThisVaultId_;
  const std::string kTestSessionId_, kServerName_, kServerDir_;
  const uint16_t kServerPort_;
};

TEST_F(VisualiserLogTest, BEH_VisualiserLog) {
  EXPECT_TRUE(IsValid(TestPersona::kMaidNode));
  EXPECT_TRUE(IsValid(TestPersona::kDataGetter));
  EXPECT_TRUE(IsValid(TestPersona::kCacheHandler));
  EXPECT_TRUE(!IsValid(static_cast<TestPersona>(-1)));
  EXPECT_TRUE(!IsValid(static_cast<TestPersona>(3)));

#ifdef USE_VLOGGING
  Identity target{ RandomString(64) };
  // Call before VlogPrefix has been set
  EXPECT_THROW(VLOG(TestPersona::kCacheHandler, TestAction::kAccountTransfer, target),
               common_error);

  // Call after VLOG has been initialised
  log::Logging::Instance().InitialiseVlog(DebugId(kThisVaultId_), kTestSessionId_, kServerName_,
                                          kServerPort_, kServerDir_);

  EXPECT_EQ(DebugId(kThisVaultId_), log::Logging::Instance().VlogPrefix());
  EXPECT_EQ(kTestSessionId_, log::Logging::Instance().VlogSessionId());

  VLOG(TestPersona::kDataGetter, TestAction::kGet, target, target);
  VLOG(TestPersona::kDataGetter, TestAction::kGet, target);
  VLOG(TestPersona::kDataGetter, TestAction::kGet, 99);
  VLOG(TestAction::kGet, target, target);
  VLOG(TestAction::kGet, target);
  VLOG(TestAction::kGet, 99);

  EXPECT_THROW(VLOG(TestPersona::kMaidNode, TestAction::kGet, Identity{}), common_error);
  EXPECT_THROW(VLOG(TestPersona::kMaidNode, static_cast<TestAction>(-1), target), common_error);
  EXPECT_THROW(VLOG(TestPersona::kMaidNode, static_cast<TestAction>(-1), 99), common_error);
  EXPECT_THROW(VLOG(static_cast<TestPersona>(-1), TestAction::kGet, target), common_error);
  EXPECT_THROW(VLOG(static_cast<TestPersona>(-1), TestAction::kGet, 99), common_error);
  EXPECT_THROW(VLOG(TestAction::kGet, Identity{}), common_error);
  EXPECT_THROW(VLOG(static_cast<TestAction>(-1), target), common_error);
  EXPECT_THROW(VLOG(static_cast<TestAction>(-1), 99), common_error);

  // Try to initialise again
  EXPECT_THROW(log::Logging::Instance().InitialiseVlog("1", kTestSessionId_, kServerName_,
                                                       kServerPort_, kServerDir_), common_error);
  EXPECT_EQ(DebugId(kThisVaultId_), log::Logging::Instance().VlogPrefix());
  EXPECT_EQ(kTestSessionId_, log::Logging::Instance().VlogSessionId());
  VLOG(TestPersona::kMaidNode, TestAction::kIncrementReferenceCount, target);

  // Emulate VLOG macro being called where an invalid Session ID has been provided to logging.
  {
    auto scoped_session_id_invalidator(GetScopedSessionIdInvalidator());
    ASSERT_NE(kTestSessionId_, log::Logging::Instance().VlogSessionId());
    VLOG(TestPersona::kDataGetter, TestAction::kGet, target, target);  // Should fail
  }

  EXPECT_EQ(kTestSessionId_, log::Logging::Instance().VlogSessionId());
  // Sleep to allow error LOG messages caused by errors returned from the server to execute before
  // the logger's destructor causes them to be ditched.  This is non-critical; just good to see
  // errors where possible.
  Sleep(std::chrono::seconds(8));
#endif
}

// This test outputs the JSON version of VLOG messages along with the string representation
// of each decoded VLOG element to allow (currently manual) checking of server-side visualiser code.
TEST_F(VisualiserLogTest, BEH_VisualiserLogCheckJson) {
  std::vector<Identity> identities;
  for (int i(0); i < 4; ++i) {
    std::string id;
    for (int j(0); j < 64; ++j)
      id += static_cast<char>((i * 64) + j);
    identities.emplace_back(id);
  }

#ifdef USE_VLOGGING
  // Set VLOG prefix in case this test isn't run after the previous one.
  try {
    log::Logging::Instance().InitialiseVlog(DebugId(kThisVaultId_), kTestSessionId_, kServerName_,
                                            kServerPort_, kServerDir_);
  }
  catch (const std::exception&) {}

  auto vlog0 = VLOG(TestPersona::kDataGetter, TestAction::kGet, identities[0], identities[1]);
  LOG(kVerbose) << '\t' << GetPostRequestBody(vlog0);
  DebugPrint(vlog0);

  auto vlog1 = VLOG(TestPersona::kDataGetter, TestAction::kPut, identities[2]);
  LOG(kVerbose) << '\t' << GetPostRequestBody(vlog1);
  DebugPrint(vlog1);

  auto vlog2 = VLOG(TestPersona::kMaidNode, TestAction::kPut, std::numeric_limits<uint64_t>::max());
  LOG(kVerbose) << '\t' << GetPostRequestBody(vlog2);
  DebugPrint(vlog2);

  auto vlog3 = VLOG(TestAction::kGet, identities[3]);
  LOG(kVerbose) << '\t' << GetPostRequestBody(vlog3);
  DebugPrint(vlog3);

  std::vector<std::string> closest_ids;
  for (int i(0); i != 16; ++i)
    closest_ids.emplace_back(std::to_string(i * 4 + i));

  std::stringstream stringstream;
  cereal::JSONOutputArchive archive{ stringstream };

  archive(cereal::make_nvp("oldId", 123));
  archive(cereal::make_nvp("newId", 234));
  archive(cereal::make_nvp("closestIds", closest_ids));

  auto vlog4 = VLOG(TestAction::kPut, stringstream.str());
  LOG(kVerbose) << '\t' << GetPostRequestBody(vlog4);
  DebugPrint(vlog4);
  // Sleep to allow error LOG messages caused by errors returned from the server to execute before
  // the logger's destructor causes them to be ditched.  This is non-critical; just good to see
  // errors where possible.
  Sleep(std::chrono::seconds(8));
#endif
}

}  // namespace test

}  // namespace maidsafe
