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

typedef CryptoPP::SHA1 SHA1;
typedef CryptoPP::SHA256 SHA256;
typedef CryptoPP::SHA384 SHA384;
typedef CryptoPP::SHA512 SHA512;
typedef CryptoPP::Integer BigInt;

enum { AES256_KeySize = 32 };  // size in bytes.
enum { AES256_IVSize = 16 };   // size in bytes.
extern const uint16_t kMaxCompressionLevel;
extern const std::string kMaidSafeVersionLabel1;
extern const std::string kMaidSafeVersionLabel;

// AES requires a key size of 128, 192, or 256 bits.  We only use 256 bits.
typedef detail::BoundedString<AES256_KeySize, AES256_KeySize> AES256Key;
// Since the block size for AES is 128 bits, only the first 128 bits of the IV are used.  We force
// the IV to be exactly 128 bits.
typedef detail::BoundedString<AES256_IVSize, AES256_IVSize> AES256InitialisationVector;
typedef detail::BoundedString<SHA1::DIGESTSIZE, SHA1::DIGESTSIZE> SHA1Hash;
typedef detail::BoundedString<SHA256::DIGESTSIZE, SHA256::DIGESTSIZE> SHA256Hash;
typedef detail::BoundedString<SHA384::DIGESTSIZE, SHA384::DIGESTSIZE> SHA384Hash;
typedef detail::BoundedString<SHA512::DIGESTSIZE, SHA512::DIGESTSIZE> SHA512Hash;
typedef TaggedValue<
    detail::BoundedString<AES256_KeySize + AES256_IVSize, AES256_KeySize + AES256_IVSize>,
    struct SecurePasswordTag> SecurePassword;
typedef TaggedValue<NonEmptyString, struct CipherTextTag> CipherText;
typedef TaggedValue<NonEmptyString, struct CompressedTextTag> CompressedText;
typedef NonEmptyString Salt, PlainText, UncompressedText;

// Returns a reference to a static CryptoPP::RandomNumberGenerator held in a
// thread-specific pointer (i.e. it's thread-safe).
CryptoPP::RandomNumberGenerator& random_number_generator();

// Performs a bitwise XOR on each char of first with the corresponding char of second.
template <size_t size>
detail::BoundedString<size, size> XOR(const detail::BoundedString<size, size>& first,
                                      const detail::BoundedString<size, size>& second) {
  std::string result(size, 0);
  auto first_itr(std::begin(first.string()));
  auto second_itr(std::begin(second.string()));
  auto result_itr(std::begin(result));
  while (result_itr != std::end(result)) {
    *result_itr = *first_itr ^ *second_itr;
    ++first_itr;
    ++second_itr;
    ++result_itr;
  }

  return detail::BoundedString<size, size>(result);
}

// Performs a bitwise XOR on each char of first with the corresponding char of second.  If the
// the strings are different lengths or are empty, the function throws a common_error.
std::string XOR(const std::string& first, const std::string& second);

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

// Hash function operating on a string.
template <typename HashType>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE> Hash(const std::string& input) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::StringSource(input, true,
                           new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing string: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::hashing_error));
  }
  return detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE>(result);
}

// Hash function operating on a vector<byte>.
template <typename HashType>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE> Hash(
    const std::vector<byte>& input) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::ArraySource(input.data(), input.size(), true,
                          new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing array: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::hashing_error));
  }
  return detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE>(result);
}

// Hash function operating on a BoundedString.
template <typename HashType, size_t min, size_t max>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE> Hash(
    const detail::BoundedString<min, max>& input) {
  return Hash<HashType>(input.string());
}

// Hash function operating on an arbitrary string type.
template <typename HashType, typename StringType>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, StringType> Hash(
    const StringType& input) {
  typedef detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, StringType>
      BoundedString;
  StringType result;
  HashType hash;
  try {
    static_cast<void>(CryptoPP::StringSource(
        reinterpret_cast<const byte*>(input.data()), input.length(), true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSinkTemplate<StringType>(result))));
  } catch (const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing string: " << e.what();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::hashing_error));
  }
  return BoundedString(result);
}

// Performs symmetric encryption using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
CipherText SymmEncrypt(const PlainText& input, const AES256Key& key,
                       const AES256InitialisationVector& initialisation_vector);

// Performs symmetric decryption using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
PlainText SymmDecrypt(const CipherText& input, const AES256Key& key,
                      const AES256InitialisationVector& initialisation_vector);

// Compress a string using gzip.  Compression level must be between 0 and 9
// inclusive or function throws a std::exception.
CompressedText Compress(const UncompressedText& input, uint16_t compression_level);

// Uncompress a string using gzip.  Will throw a std::exception if uncompression fails.
UncompressedText Uncompress(const CompressedText& input);

std::vector<std::string> SecretShareData(int32_t threshold, int32_t number_of_shares,
                                         const std::string& data);

std::vector<std::vector<byte>> SecretShareData(int32_t threshold, int32_t number_of_shares,
                                               const std::vector<byte>& data);

std::string SecretRecoverData(const std::vector<std::string>& in_strings);

std::vector<byte> SecretRecoverData(const std::vector<std::vector<byte>>& in_arrays);

std::vector<std::string> InfoDisperse(int32_t threshold, int32_t number_of_shares,
                                      const std::string& data);

std::vector<std::vector<byte>> InfoDisperse(int32_t threshold, int32_t number_of_shares,
                                            const std::vector<byte>& data);

std::string InfoRetrieve(const std::vector<std::string>& in_strings);

std::vector<byte> InfoRetrieve(const std::vector<std::vector<byte>>& in_arrays);

CipherText ObfuscateData(const Identity& name, const PlainText& plain_text);

PlainText DeobfuscateData(const Identity& name, const CipherText& cipher_text);

}  // namespace crypto

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_H_
