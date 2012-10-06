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

#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/error.h"


namespace maidsafe {

namespace detail {

namespace test {

TEST(BoundedStringTest, BEH_All) {
  typedef BoundedString<0, 1> ZeroOne;
  typedef BoundedString<0> ZeroMax;
  typedef BoundedString<1, 1> OneOne;
  typedef BoundedString<1, 2> OneTwo;
  typedef BoundedString<1> OneMax;
  typedef BoundedString<2, 2> TwoTwo;
  typedef BoundedString<2> TwoMax;

  ZeroOne a;
  EXPECT_TRUE(a.IsInitialised());

  OneOne b;
  EXPECT_FALSE(b.IsInitialised());

  ZeroOne c("");
  EXPECT_TRUE(a.IsInitialised());

  EXPECT_THROW(OneOne d(""), std::exception);



  b = BoundedString<1, 2>("1");
  EXPECT_THROW(TwoTwo z(b), std::exception);


}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
