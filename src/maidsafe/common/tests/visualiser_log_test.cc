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
  VisualiserLogTest() : this_vault_id_(InitId()) {}
  std::string GetPostRequestBody(const log::VisualiserLogMessage& vlog) {
    return vlog.GetPostRequestBody();
  }
  void DebugPrint(const log::VisualiserLogMessage& vlog) {
    LOG(kVerbose) << "\tts:                   \"" << vlog.kTimestamp_ << '\"';
    LOG(kVerbose) << "\tvault_id:             \"" << vlog.kVaultId_ << '\"';
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

  Identity this_vault_id_;
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
  CHECK_THROWS_AS(VLOG(TestPersona::kCacheHandler, TestAction::kAccountTransfer, target),
                  common_error);

  // Call after VLOG has been initialised
  log::Logging::Instance().InitialiseVlog("Visualiser log test", "128.199.223.97", 8080, "/log");
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

  // Try to initialise again
  CHECK_THROWS_AS(log::Logging::Instance().InitialiseVlog("1", "128.199.223.97", 8080, "/log"),
                  common_error);
  VLOG(TestPersona::kMaidNode, TestAction::kIncrementReferenceCount, target);
#endif
}

// This test outputs the URL-encoded version of VLOG messages along with the string representation
// of each decoded VLOG element to allow (currently manual) checking of server-side visualiser code.
TEST_F(VisualiserLogTest, BEH_VisualiserLogCheckUrlEncode) {
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
    log::Logging::Instance().InitialiseVlog(DebugId(this_vault_id_), "128.199.223.97", 8080,
                                            "/log");
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

  Sleep(std::chrono::milliseconds(100));  // To avoid Catch output getting mixed in with TLOGs.
#endif
}

}  // namespace test

}  // namespace maidsafe
