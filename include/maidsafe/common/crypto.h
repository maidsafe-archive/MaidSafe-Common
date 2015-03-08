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

#ifndef MAIDSAFE_COMMON_CRYPTO_H_
#define MAIDSAFE_COMMON_CRYPTO_H_

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4127 4189 4244 4702 4800)
#endif
#include "cryptopp/algparam.h"
#include "cryptopp/asn.h"
#include "cryptopp/base32.h"
#include "cryptopp/base64.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/default.h"
#include "cryptopp/emsa2.h"
#include "cryptopp/filters.h"
#include "cryptopp/ida.h"
#include "cryptopp/misc.h"
#include "cryptopp/modarith.h"
#include "cryptopp/mqueue.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/pubkey.h"
#include "cryptopp/secblock.h"
#include "cryptopp/simple.h"
#include "cryptopp/strciphr.h"
#include "cryptopp/zdeflate.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "cryptopp/aes.h"
#include "cryptopp/gcm.h"
#include "cryptopp/channels.h"
#include "cryptopp/files.h"
#include "cryptopp/gzip.h"
#include "cryptopp/hex.h"
#include "cryptopp/integer.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pssr.h"
#include "cryptopp/sha.h"
#include "cryptopp/tiger.h"

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/identity.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"

namespace CryptoPP {

class SHA1;
class SHA256;
class SHA384;
class SHA512;

}  // namespace CryptoPP

namespace maidsafe {

namespace crypto {

using SHA1 = CryptoPP::SHA1;
using SHA256 = CryptoPP::SHA256;
using SHA384 = CryptoPP::SHA384;
using SHA512 = CryptoPP::SHA512;
using BigInt = CryptoPP::Integer;

// AES requires a key size of 128, 192, or 256 bits.  We only use 256 bits.
const int AES256_KeySize = 32;  // size in bytes.
// Since the block size for AES is 128 bits, only the first 128 bits of the IV are used.  We force
// the IV to be exactly 128 bits.
const int AES256_IVSize = 16;   // size in bytes.
const std::uint16_t kMaxCompressionLevel = 9;
const std::string kMaidSafeVersionLabel1 = "MaidSafe Version 1 Key Derivation";
const std::string kMaidSafeVersionLabel = kMaidSafeVersionLabel1;

using AES256KeyAndIV =
    detail::BoundedString<AES256_KeySize + AES256_IVSize, AES256_KeySize + AES256_IVSize>;
using SecurePassword = TaggedValue<AES256KeyAndIV, struct SecurePasswordTag>;
using CipherText = TaggedValue<NonEmptyString, struct CipherTextTag>;
using CompressedText = TaggedValue<NonEmptyString, struct CompressedTextTag>;
using Salt = NonEmptyString;
using PlainText = NonEmptyString;
using UncompressedText = NonEmptyString;
using DataParts = std::vector<NonEmptyString>;

// Returns a reference to a static CryptoPP::RandomNumberGenerator held in a
// thread-specific pointer (i.e. it's thread-safe).
CryptoPP::RandomNumberGenerator& random_number_generator();

// Creates a secure password of size AES256_KeySize + AES256_IVSize using the Password-Based Key
// Derivation Function (PBKDF) version 2 algorithm.  The number of iterations is derived from "pin".
// "label" is additional data to provide distinct input data to PBKDF.  The function will throw a
// std::exception if invalid parameters are passed.
template <typename PasswordType>
SecurePassword CreateSecurePassword(const PasswordType& password, const Salt& salt, uint32_t pin,
                                    const std::string& label = kMaidSafeVersionLabel) {
  if (!password.IsInitialised() || !salt.IsInitialised()) {
    LOG(kError) << "CreateSecurePassword password or salt uninitialised";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  uint16_t iter = (pin % 10000) + 10000;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
  CryptoPP::SecByteBlock derived(AES256_KeySize + AES256_IVSize);
  byte purpose = 0;  // unused in this pbkdf implementation
  CryptoPP::SecByteBlock context(salt.string().size() + label.size());
  std::copy_n(salt.string().data(), salt.string().size(), &context[0]);
  std::copy_n(label.data(), label.size(), &context[salt.string().size()]);
  pbkdf.DeriveKey(derived, derived.size(), purpose,
                  reinterpret_cast<const byte*>(password.string().data()), password.string().size(),
                  context.data(), context.size(), iter);
  std::string derived_password;
  CryptoPP::StringSink string_sink(derived_password);
  string_sink.Put(derived, derived.size());
  return SecurePassword(SecurePassword::value_type(derived_password));
}

// Hash function designed to operate on an arbitrary string type, e.g. std::string or
// std::vector<byte>.
template <typename HashType, typename String>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, String> Hash(
    const String& input) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::ArraySource(
        reinterpret_cast<const byte*>(input.data()), input.size(), true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing string: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::hashing_error));
  }
  return detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, String>(result);
}

// Hash function operating on a BoundedString.
template <typename HashType, size_t min, size_t max, typename String>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, String> Hash(
    const detail::BoundedString<min, max, String>& input) {
  return Hash<HashType>(input.string());
}

// Performs symmetric encryption using AES256.
CipherText SymmEncrypt(const PlainText& input, const AES256KeyAndIV& key_and_iv);

// Performs symmetric decryption using AES256.
PlainText SymmDecrypt(const CipherText& input, const AES256KeyAndIV& key_and_iv);

// Compress a string using gzip.  Compression level must be between 0 and 9
// inclusive or function throws a std::exception.
CompressedText Compress(const UncompressedText& input, uint16_t compression_level);

// Uncompress a string using gzip.  Will throw a std::exception if uncompression fails.
UncompressedText Uncompress(const CompressedText& input);

DataParts SecretShareData(int32_t threshold, int32_t number_of_shares, const PlainText& data);

PlainText SecretRecoverData(const DataParts& parts);

DataParts InfoDisperse(int32_t threshold, int32_t number_of_shares, const PlainText& data);

PlainText InfoRetrieve(const DataParts& parts);

CipherText ObfuscateData(const Identity& name, const PlainText& plain_text);

PlainText DeobfuscateData(const Identity& name, const CipherText& cipher_text);

}  // namespace crypto

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_H_
