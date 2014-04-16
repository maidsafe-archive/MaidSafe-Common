/*  Copyright 2013 MaidSafe.net limited

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

#include "maidsafe/common/authentication/detail/secure_string.h"

namespace maidsafe {

namespace authentication {

namespace detail {

SecureString::SecureString()
    : phrase_(GetRandomString<SafeString>(crypto::SHA512::DIGESTSIZE)),
      string_(),
      encryptor_(new Encryptor(phrase_.data(), new Encoder(new Sink(string_)))) {}

SecureString::~SecureString() {}

void SecureString::Append(char decrypted_char) { encryptor_->Put(decrypted_char); }

void SecureString::Finalise() { encryptor_->MessageEnd(); }

void SecureString::Clear() {
  string_.clear();
  encryptor_.reset(new Encryptor(phrase_.data(), new Encoder(new Sink(string_))));
}

SafeString SecureString::string() const {
  SafeString decrypted_string;
  Decoder decryptor(new Decryptor(phrase_.data(), new Sink(decrypted_string)));
  decryptor.Put(reinterpret_cast<const byte*>(string_.data()), string_.length());
  decryptor.MessageEnd();
  return decrypted_string;
}

SafeString operator+(const SafeString& first, const SafeString& second) {
  return SafeString(first.begin(), first.end()) + SafeString(second.begin(), second.end());
}

SafeString operator+(const SecureString::Hash& first, const SafeString& second) {
  return SafeString(first.string().begin(), first.string().end()) + second;
}

SafeString operator+(const SafeString& first, const SecureString::Hash& second) {
  return first + SafeString(second.string().begin(), second.string().end());
}

// see safe_allocators.h
LockedPageManager LockedPageManager::instance;

}  // namespace detail

}  // namespace authentication

}  // namespace maidsafe
