/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/on_scope_exit.h"

#include <vector>

#include "maidsafe/common/error.h"
#include "maidsafe/common/test.h"


namespace maidsafe {

namespace test {

void Increment(std::vector<int32_t>& data) {
  data.push_back(static_cast<int32_t>(data.size()));
}

void IncrementAndThrow(std::vector<int32_t>& data) {
  Increment(data);
  ThrowError(CommonErrors::invalid_parameter);
}

TEST(OnScopeExit, BEH_RevertValue) {
  std::vector<int32_t> before;
  for (int i(0); i != 100; ++i)
    before.push_back(i);
  {
    on_scope_exit strong_guarantee(on_scope_exit::RevertValue(before));
    Increment(before);
    EXPECT_EQ(before.size(), 101);
    strong_guarantee.Release();
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);

  try {
    on_scope_exit strong_guarantee(on_scope_exit::RevertValue(before));
    IncrementAndThrow(before);
  }
  catch(const maidsafe_error&) {
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);
}

TEST(OnScopeExit, BEH_SetAction) {
  std::vector<int32_t> before;
  for (int i(0); i != 100; ++i)
    before.push_back(i);
  {
    on_scope_exit strong_guarantee([&before]() { before.clear(); });
    Increment(before);
    EXPECT_EQ(before.size(), 101);
    strong_guarantee.Release();
    EXPECT_EQ(before.size(), 101);
  }
  EXPECT_EQ(before.size(), 101);

  try {
    on_scope_exit strong_guarantee([&before]() { before.clear(); });
    IncrementAndThrow(before);
  }
  catch(const maidsafe_error&) {
    EXPECT_TRUE(before.empty());
  }
  EXPECT_TRUE(before.empty());
}

}  // namespace test

}  // namespace maidsafe
