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

#ifndef MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIAL_UTILS_H_
#define MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIAL_UTILS_H_

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/authentication/user_credentials.h"

namespace maidsafe {

namespace authentication {

// Uses PBKDFv2 function to generate a secure password from the user's password and pin.  The user's
// keyword is not used in this function.  Throws if the password or pin is null.
crypto::SecurePassword CreateSecurePassword(const UserCredentials& user_credentials);

// Uses PBKDFv2 function to generate a secure password from the user's keyword, password and pin,
// then performs a bitwise XOR on the secure password and data.  Throws if any user credential is
// null.
NonEmptyString Obfuscate(const UserCredentials& user_credentials, const NonEmptyString& data);

}  // namespace authentication

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIAL_UTILS_H_
