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
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace detail {

namespace test {

TEST(BoundedStringTest, BEH_All) {
  BoundedString<1, 3> one("1");
  BoundedString<1, 2> two(one);
  typedef BoundedString<2, 2> Three;
  EXPECT_THROW(Three three(one), std::exception);
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
