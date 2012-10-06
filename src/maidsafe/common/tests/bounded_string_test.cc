/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/common/bounded_string.h"

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace detail {

namespace test {

typedef BoundedString<1, 1> OneOne;
typedef BoundedString<1, 2> OneTwo;
typedef BoundedString<1> OneMax;
typedef BoundedString<2, 2> TwoTwo;
typedef BoundedString<2> TwoMax;

TEST(BoundedStringTest, BEH_DefaultConstructor) {
  OneOne a;
  EXPECT_FALSE(a.IsInitialised());
  OneMax b;
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_Getters) {
  TwoTwo a;
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_THROW(a.string(), std::exception);

  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax b(random);
  EXPECT_TRUE(b.IsInitialised());
  EXPECT_EQ(random, b.string());
}

TEST(BoundedStringTest, BEH_StringConstructor) {
  // Empty (invalid)
  EXPECT_THROW(OneOne a(""), std::exception);
  // Valid
  std::string random(RandomString(1));
  OneOne b(random);
  EXPECT_EQ(random, b.string());
  // Too big
  EXPECT_THROW(OneOne c(RandomString(2)), std::exception);

  // Empty (invalid)
  EXPECT_THROW(OneMax d(""), std::exception);
  // Valid
  random = RandomString((RandomUint32() % 1024) + 1);
  OneMax e(random);
  EXPECT_EQ(random, e.string());
}

TEST(BoundedStringTest, BEH_Swap) {
  // Swap with initialised
  std::string random1(RandomString(1));
  std::string random2(RandomString(2));
  OneTwo a(random1);
  OneTwo b(random2);
  swap(a, b);
  EXPECT_EQ(random2, a.string());
  EXPECT_EQ(random1, b.string());

  // Swap with uninitialised
  OneTwo c;
  swap(a, c);
  EXPECT_FALSE(a.IsInitialised());
  EXPECT_EQ(random2, c.string());
}

TEST(BoundedStringTest, BEH_Copy) {
  // Copy from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b(a);
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Copy from uninitialised
  OneMax c;
  OneMax d(c);
  EXPECT_FALSE(d.IsInitialised());
}

TEST(BoundedStringTest, BEH_Move) {
  // Move from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(std::move(OneMax(random)));
  EXPECT_EQ(random, a.string());

  // Move from uninitialised
  OneMax b(std::move(OneMax()));
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_Assignment) {
  // Assign from initialised
  std::string random(RandomString((RandomUint32() % 1024) + 1));
  OneMax a(random);
  OneMax b("1");
  b = a;
  EXPECT_EQ(random, a.string());
  EXPECT_EQ(random, b.string());

  // Assign from self
  b = b;
  EXPECT_EQ(random, b.string());

  // Assign from uninitialised
  OneMax c;
  b = c;
  EXPECT_FALSE(b.IsInitialised());
}

TEST(BoundedStringTest, BEH_OtherBoundedStringConstructor) {
  // Copy from too small
  OneTwo a(RandomString(1));
  EXPECT_THROW(TwoTwo b(a), std::exception);
  EXPECT_THROW(TwoMax c(a), std::exception);

  // Copy from too big
  OneMax d(RandomString(3));
  EXPECT_THROW(OneTwo e(d), std::exception);
  EXPECT_THROW(TwoTwo f(d), std::exception);

  // Valid copy
  std::string random(RandomString(2));
  OneTwo g(random);
  OneMax h(g);
  EXPECT_EQ(random, h.string());
  TwoTwo i(g);
  EXPECT_EQ(random, i.string());

  // Copy from uninitialised
  OneOne j;
  OneTwo k(j);
  EXPECT_FALSE(k.IsInitialised());
  TwoTwo l(j);
  EXPECT_FALSE(l.IsInitialised());
}

TEST(BoundedStringTest, BEH_OtherBoundedStringAssignment) {
  // Assign from too small
  OneTwo a(RandomString(1));
  std::string before_throw(RandomString(2));
  TwoTwo b(before_throw);
  EXPECT_THROW(b = a, std::exception);
  EXPECT_EQ(before_throw, b.string());
  TwoMax c;
  EXPECT_THROW(c = a, std::exception);
  EXPECT_FALSE(c.IsInitialised());

  // Assign from too big
  OneMax d(RandomString(3));
  OneTwo e;
  EXPECT_THROW(e = d, std::exception);
  EXPECT_FALSE(e.IsInitialised());
  TwoTwo f(before_throw);
  EXPECT_THROW(f = d, std::exception);
  EXPECT_EQ(before_throw, f.string());

  // Valid assignment
  std::string random(RandomString(2));
  OneTwo g(random);
  OneMax h("1");
  h = g;
  EXPECT_EQ(random, h.string());
  TwoTwo i("02");
  i = g;
  EXPECT_EQ(random, i.string());

  // Assign from uninitialised
  OneOne j;
  OneTwo k("1");
  k = j;
  EXPECT_FALSE(k.IsInitialised());
  TwoTwo l("02");
  l = j;
  EXPECT_FALSE(l.IsInitialised());
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
