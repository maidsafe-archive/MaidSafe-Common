/* Copyright 2009 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_CRYPTO_H_
#define MAIDSAFE_COMMON_CRYPTO_H_

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4355 4702)
#endif

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "cryptopp/gzip.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/pssr.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/cryptlib.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "boost/filesystem/path.hpp"

#include "cryptopp/channels.h"
#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include "cryptopp/ida.h"
#include "cryptopp/integer.h"
#include "cryptopp/sha.h"
#include "cryptopp/tiger.h"
#include "cryptopp/aes.h"
#include "cryptopp/osrng.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/types.h"


namespace CryptoPP {
class SHA1;
class SHA256;
class SHA384;
class SHA512;
class Tiger;
}  // namespace CryptoPP

namespace maidsafe {

namespace crypto {

typedef CryptoPP::SHA1 SHA1;
typedef CryptoPP::SHA256 SHA256;
typedef CryptoPP::SHA384 SHA384;
typedef CryptoPP::SHA512 SHA512;
typedef CryptoPP::Tiger Tiger;
typedef CryptoPP::Integer BigInt;

enum { AES256_KeySize = 32 };  // size in bytes.
enum { AES256_IVSize = 16 };  // size in bytes.
extern const uint16_t kMaxCompressionLevel;
extern const std::string kMaidSafeVersionLabel1;
extern const std::string kMaidSafeVersionLabel;

typedef detail::BoundedString<AES256_KeySize> AES256Key;
typedef detail::BoundedString<AES256_IVSize> AES256InitialisationVector;
typedef detail::BoundedString<SHA1::DIGESTSIZE, SHA1::DIGESTSIZE> SHA1Hash;
typedef detail::BoundedString<SHA256::DIGESTSIZE, SHA256::DIGESTSIZE> SHA256Hash;
typedef detail::BoundedString<SHA384::DIGESTSIZE, SHA384::DIGESTSIZE> SHA384Hash;
typedef detail::BoundedString<SHA512::DIGESTSIZE, SHA512::DIGESTSIZE> SHA512Hash;
typedef detail::BoundedString<Tiger::DIGESTSIZE, Tiger::DIGESTSIZE> TigerHash;
typedef NonEmptyString SecurePassword, Salt, PlainText, CipherText,
        CompressedText, UncompressedText;


// Performs a bitwise XOR on each char of first with the corresponding char of second.  If size is
// 0, an empty string is returned.
template<size_t size>
detail::BoundedString<size, size> XOR(const detail::BoundedString<size, size>& first,
                                      const detail::BoundedString<size, size>& second) {
  std::string result(size, 0);
  for (size_t i(0); i != size; ++i)
    result[i] = first.string()[i] ^ second.string()[i];

  return detail::BoundedString<size, size>(result);
}

std::string XOR(const std::string& first, const std::string& second);

// Creates a secure password using the Password-Based Key Derivation Function (PBKDF) version 2
// algorithm.  The number of iterations is derived from "pin".  "label" is additional data to
// provide distinct input data to PBKDF.  The function will throw a std::exception if invalid
// parameters are passed.
template<typename PasswordType>
SecurePassword CreateSecurePassword(const PasswordType& password,
                                    const Salt& salt,
                                    const uint32_t& pin,
                                    const std::string& label = kMaidSafeVersionLabel) {
  if (!password.IsInitialised() || !salt.IsInitialised())
    ThrowError(CommonErrors::uninitialised);
  uint16_t iter = (pin % 10000) + 10000;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
  CryptoPP::SecByteBlock derived(AES256_KeySize + AES256_IVSize);
  byte purpose = 0;  // unused in this pbkdf implementation
  CryptoPP::SecByteBlock context(salt.string().size() + label.size());
  std::copy_n(salt.string().data(), salt.string().size(), &context[0]);
  std::copy_n(label.data(),  label.size(), &context[salt.string().size()]);
  pbkdf.DeriveKey(derived, derived.size(), purpose,
                  reinterpret_cast<const byte*>(password.string().data()),
                  password.string().size(), context.data(), context.size(), iter);
  std::string derived_password;
  CryptoPP::StringSink string_sink(derived_password);
  string_sink.Put(derived, derived.size());
  return SecurePassword(derived_password);
}

// Hash function operating on a string.
template <typename HashType>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE> Hash(const std::string& input) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::StringSource(input, true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing string: " << e.what();
    ThrowError(CommonErrors::hashing_error);
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
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, StringType>
      Hash(const StringType& input) {
  typedef detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE, StringType>
            BoundedString;
  StringType result;
  HashType hash;
  try {
    CryptoPP::StringSource(reinterpret_cast<const byte*>(input.data()), input.length(), true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSinkTemplate<StringType>(result)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing string: " << e.what();
    ThrowError(CommonErrors::hashing_error);
  }
  return BoundedString(result);
}

// Hash function operating on a file.
template <typename HashType>
detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE> HashFile(
    const boost::filesystem::path& file_path) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::FileSource(file_path.c_str(), true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  }
  catch(const CryptoPP::Exception& e) {
    LOG(kError) << "Error hashing file " << file_path << ": " << e.what();
    ThrowError(CommonErrors::hashing_error);
  }
  return detail::BoundedString<HashType::DIGESTSIZE, HashType::DIGESTSIZE>(result);
}

// Performs symmetric encrytion using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
CipherText SymmEncrypt(const PlainText& input,
                       const AES256Key& key,
                       const AES256InitialisationVector& initialisation_vector);

// Performs symmetric decrytion using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
PlainText SymmDecrypt(const CipherText& input,
                      const AES256Key& key,
                      const AES256InitialisationVector& initialisation_vector);

// Compress a string using gzip.  Compression level must be between 0 and 9
// inclusive or function throws a std::exception.
CompressedText Compress(const UncompressedText& input, const uint16_t& compression_level);

// Uncompress a string using gzip.  Will throw a std::exception if uncompression fails.
UncompressedText Uncompress(const CompressedText& input);

std::vector<std::string> SecretShareData(const int32_t& threshold,
                                         const int32_t& number_of_shares,
                                         const std::string& data);

std::string SecretRecoverData(const int32_t& threshold, const std::vector<std::string>& in_strings);

}  // namespace crypto

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_H_
