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

#include "maidsafe/common/data_types/tests/test_utils.h"

#include "maidsafe/common/data_types/data.h"

namespace maidsafe {

namespace test {

testing::AssertionResult Equal(const Data* const lhs, const Data* const rhs) {
  if (!lhs->IsInitialised() && !rhs->IsInitialised())
    return testing::AssertionSuccess();

  if (!lhs->IsInitialised())
    return testing::AssertionFailure() << "lhs is not initialised.";

  if (!rhs->IsInitialised())
    return testing::AssertionFailure() << "rhs is not initialised.";

  if (lhs->NameAndType() != rhs->NameAndType())
    return testing::AssertionFailure() << "lhs->NameAndType() [" << lhs->NameAndType().name << ":"
                                       << lhs->NameAndType().type_id << "] != rhs->NameAndType() ["
                                       << rhs->NameAndType().name << ":"
                                       << rhs->NameAndType().type_id << "].";

  return testing::AssertionSuccess();
}

}  // namespace test

}  // namespace maidsafe
