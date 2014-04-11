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

#ifndef MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIALS_H_
#define MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIALS_H_

#include <functional>
#include <memory>

#include "maidsafe/common/authentication/detail/secure_string.h"

namespace maidsafe {

namespace authentication {

struct UserCredentials {
  typedef detail::SecureInputString<
      std::greater_equal<detail::SecureString::size_type>, 1, detail::KeywordTag> Keyword;
  typedef detail::SecureInputString<
      std::greater_equal<detail::SecureString::size_type>, 1, detail::PasswordTag> Password;
  typedef detail::SecureInputString<
      std::greater_equal<detail::SecureString::size_type>, 1, detail::PinTag> Pin;

  UserCredentials() : keyword(), pin(), password() {}

  UserCredentials(UserCredentials&& other) : keyword(std::move(other.keyword)),
      pin(std::move(other.pin)), password(std::move(other.password)) {}

  UserCredentials& operator=(UserCredentials&& other) {
    keyword = std::move(other.keyword);
    pin = std::move(other.pin);
    password = std::move(other.password);
    return *this;
  }

  std::unique_ptr<Keyword> keyword;
  std::unique_ptr<Pin> pin;
  std::unique_ptr<Password> password;
};

}  // namespace authentication

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_AUTHENTICATION_USER_CREDENTIALS_H_
