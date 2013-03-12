/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   *  Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
   *  Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
   *  Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_COMMON_RSA_H_
#define MAIDSAFE_COMMON_RSA_H_

#include <string>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif
#include "cryptopp/channels.h"
#include "cryptopp/ida.h"
#include "cryptopp/rsa.h"
#ifdef __MSVC__
#  pragma warning(pop)
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
  enum { kKeyBitSize = 2048, kSignatureByteSize = kKeyBitSize / 8 };
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

bool CheckFileSignature(const boost::filesystem::path& filename,
                        const Signature& signature,
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
