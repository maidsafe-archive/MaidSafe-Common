/*  Copyright 2015 MaidSafe.net limited

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

#include "maidsafe/common/serialisation/types/boost_filesystem.h"

#include "boost/filesystem.hpp"

#include "maidsafe/common/utils.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace test {

TEST(PathSerialisationTest, BEH_SaveAndLoad) {
  fs::path original_path;
  ASSERT_TRUE(original_path.empty());

  auto serialised_path(Serialise(original_path));
  auto parsed_path(Parse<fs::path>(serialised_path));
  EXPECT_EQ(original_path, parsed_path);

  original_path = RandomAlphaNumericString(10);
  serialised_path = Serialise(original_path);
  parsed_path = Parse<fs::path>(serialised_path);
  EXPECT_EQ(original_path, parsed_path);

  original_path /= std::wstring(1, 0xa74e);
  serialised_path = Serialise(original_path);
  parsed_path = Parse<fs::path>(serialised_path);
  EXPECT_EQ(original_path, parsed_path);
}

}  // namespace test

}  // namespace maidsafe
