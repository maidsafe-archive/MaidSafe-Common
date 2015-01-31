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
#include <memory>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4520)
#endif
#include "boost/flyweight.hpp"
#include "boost/flyweight/no_locking.hpp"
#include "boost/flyweight/refcounted.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"

#include "maidsafe/common/serialisation/types/boost_flyweight.h"

#include "maidsafe/common/utils.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {
namespace test {

TEST(FlyweightSerialisationTest, BEH_OneArgument) {
  using FlyString = boost::flyweight<std::string>;

  const auto string_one = "this is one string";
  const auto string_two = "this is two string";

  std::vector<byte> serialised;
  {
    const FlyString one{string_one};
    const FlyString two{string_one};
    const FlyString three{string_two};
    const FlyString four{string_one};
    const FlyString five{string_two};

    EXPECT_EQ(std::addressof(one.get()), std::addressof(two.get()));
    EXPECT_EQ(std::addressof(one.get()), std::addressof(four.get()));
    EXPECT_EQ(std::addressof(three.get()), std::addressof(five.get()));

    serialised = Serialise(one, two, three, four, five);

    const auto larger = Serialise(one.get(), two.get(), three.get(), four.get(), five.get());
    EXPECT_LT(serialised.size(), larger.size());
  }
  {
    FlyString one, two, three, four, five;
    Parse(serialised, one, two, three, four, five);

    EXPECT_EQ(std::addressof(one.get()), std::addressof(two.get()));
    EXPECT_EQ(std::addressof(one.get()), std::addressof(four.get()));
    EXPECT_EQ(std::addressof(three.get()), std::addressof(five.get()));

    EXPECT_STREQ(string_one, one.get().c_str());
    EXPECT_STREQ(string_one, two.get().c_str());
    EXPECT_STREQ(string_two, three.get().c_str());
    EXPECT_STREQ(string_one, four.get().c_str());
    EXPECT_STREQ(string_two, five.get().c_str());
  }
}

TEST(FlyweightSerialisationTest, BEH_MultipleArguments) {
  namespace fly = boost::flyweights;
  using FlyString = boost::flyweight<std::string, fly::no_locking, fly::refcounted>;

  const auto string_one = "this is one string";
  const auto string_two = "this is two string";

  std::vector<byte> serialised;
  {
    const FlyString one{string_one};
    const FlyString two{string_one};
    const FlyString three{string_two};
    const FlyString four{string_one};
    const FlyString five{string_two};

    EXPECT_EQ(std::addressof(one.get()), std::addressof(two.get()));
    EXPECT_EQ(std::addressof(one.get()), std::addressof(four.get()));
    EXPECT_EQ(std::addressof(three.get()), std::addressof(five.get()));

    serialised = Serialise(one, two, three, four, five);

    const auto larger = Serialise(one.get(), two.get(), three.get(), four.get(), five.get());
    EXPECT_LT(serialised.size(), larger.size());
  }
  {
    FlyString one, two, three, four, five;
    Parse(serialised, one, two, three, four, five);

    EXPECT_EQ(std::addressof(one.get()), std::addressof(two.get()));
    EXPECT_EQ(std::addressof(one.get()), std::addressof(four.get()));
    EXPECT_EQ(std::addressof(three.get()), std::addressof(five.get()));

    EXPECT_STREQ(string_one, one.get().c_str());
    EXPECT_STREQ(string_one, two.get().c_str());
    EXPECT_STREQ(string_two, three.get().c_str());
    EXPECT_STREQ(string_one, four.get().c_str());
    EXPECT_STREQ(string_two, five.get().c_str());
  }
}

}  // namespace test
}  // namespace maidsafe
