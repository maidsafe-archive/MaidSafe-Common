/*  Copyright 2009 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_RSA_H_
#define MAIDSAFE_COMMON_RSA_H_

#include <string>

#ifdef __MSVC__
#pragma warning(push, 1)
#pragma warning(disable : 4702)
#endif
#include "cryptopp/channels.h"
#include "cryptopp/ida.h"
#include "cryptopp/rsa.h"
#ifdef __MSVC__
#pragma warning(pop)
#endif

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/node_id.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace rsa {

typedef CryptoPP::RSA::PrivateKey PrivateKey;
typedef CryptoPP::RSA::PublicKey PublicKey;
struct Keys {
  // The signature will be the same size as the key size in bytes
  // http://stackoverflow.com/questions/5403808/private-key-length-bytes
  // http://stackoverflow.com/questions/6658728/rsa-signature-size
  enum {
    kKeyBitSize = 2048,
    kSignatureByteSize = kKeyBitSize / 8
  };
  Keys() : private_key(), public_key() {}
  PrivateKey private_key;
  PublicKey public_key;
};

// TODO(Fraser#5#): 2012-10-02 - Calculate reliable lower and upper bounds for the following 2 types
typedef detail::BoundedString<2> EncodedPublicKey;
typedef detail::BoundedString<3> EncodedPrivateKey;

typedef NonEmptyString PlainText, CipherText;
typedef detail::BoundedString<Keys::kSignatureByteSize> Signature;

Keys GenerateKeyPair();

CipherText Encrypt(const PlainText& data, const PublicKey& public_key);

PlainText Decrypt(const CipherText& data, const PrivateKey& private_key);

Signature Sign(const PlainText& data, const PrivateKey& private_key);

Signature SignFile(const boost::filesystem::path& filename, const PrivateKey& private_key);

bool CheckSignature(const PlainText& data, const Signature& signature, const PublicKey& public_key);

bool CheckFileSignature(const boost::filesystem::path& filename, const Signature& signature,
                        const PublicKey& public_key);

EncodedPrivateKey EncodeKey(const PrivateKey& private_key);

EncodedPublicKey EncodeKey(const PublicKey& public_key);

PrivateKey DecodeKey(const EncodedPrivateKey& private_key);

PublicKey DecodeKey(const EncodedPublicKey& public_key);

// TODO(Fraser#5#): 2012-10-07 - This is used in ProcessStore of signature_packet_rules.cc.  Remove
//                  This function once we have a proper BoundedString typedef for EncodedPublicKey.
bool ValidateKey(const PublicKey& public_key);

bool MatchingKeys(const PrivateKey& private_key1, const PrivateKey& private_key2);

bool MatchingKeys(const PublicKey& public_key1, const PublicKey& public_key2);

}  // namespace rsa

namespace asymm = rsa;

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_RSA_H_
