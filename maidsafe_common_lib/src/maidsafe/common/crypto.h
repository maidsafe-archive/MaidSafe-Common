/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
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

#ifndef MAIDSAFE_COMMON_CRYPTO_H_
#define MAIDSAFE_COMMON_CRYPTO_H_

#include <cstdint>
#include <string>
#include <algorithm>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "boost/filesystem/path.hpp"

#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include "cryptopp/integer.h"
#include "cryptopp/pubkey.h"
#include "cryptopp/rsa.h"
#include "cryptopp/sha.h"
#include "cryptopp/tiger.h"
#include "cryptopp/aes.h"
#include "cryptopp/osrng.h"

#ifdef __MSVC__
#  pragma warning(pop)
#  pragma warning(disable: 4505)
#endif

#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


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


const uint16_t AES256_KeySize = 32;  /**< size in bytes. */
const uint16_t AES256_IVSize = 16;  /**< size in bytes. */
const uint16_t kMaxCompressionLevel = 9;

static const std::string kMaidSafeVersionLabel1 =
    "MaidSafe Version 1 Key Derivation";
static const std::string kMaidSafeVersionLabel = kMaidSafeVersionLabel1;

/** XOR one string with another.
 *  The function performs an bitwise XOR on each char of first with the
 *  corresponding char of second.  first and second must have identical size.
 *  @param first string to be obfuscated.
 *  @param second string used to obfuscate the first one.
 *  @return The obfuscated string. */
std::string XOR(const std::string &first, const std::string &second);

/** Creates a secure password.
 *  Creates a secure password using the Password-Based Key Derivation Function
 *  (PBKDF) version 2 algorithm.
 *  @param password password.
 *  @param salt salt.
 *  @param pin PIN from which the number of iterations is derived.
 *  @param label additional data to provide distinct input data to PBKDF
 *  @return CommonReturnCode */
int SecurePassword(const std::string &password,
                           const std::string &salt,
                           const uint32_t &pin,
                           std::string *derived_password,
                           const std::string &label = kMaidSafeVersionLabel);

/** Hash function operating on a string.
 *  @tparam HashType type of hash function to use (e.g. SHA512)
 *  @param input string that is to be hashed.
 *  @return the result of the hash function. */
template <class HashType>
std::string Hash(const std::string &input) {
  std::string result;
  HashType hash;
  CryptoPP::StringSource(input, true,
      new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  return result;
}

/** Hash function operating on a file.
 *  @tparam HashType type of hash function to use (e.g. SHA512)
 *  @param file_path path to file that is to be hashed.
 *  @return the result of the hash function, or empty string if the file could
 *  not be read. */
template <class HashType>
std::string HashFile(const boost::filesystem::path &file_path) {
  std::string result;
  HashType hash;
  try {
    CryptoPP::FileSource(file_path.c_str(), true,
        new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(result)));
  }
  catch(...) {
    // DLOG(ERROR) << e.what();
    result.clear();
  }
  return result;
}

/** Symmetrically encrypt a string.
 *  Performs symmetric encrytion using AES256. It returns an empty string if the
 *  key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
 *  @param input string to be encrypted.
 *  @param key key used to encrypt.  Size must be >= AES256_KeySize.
 *  @param initialisation_vector initialisation vector used to encrypt.  Size
 *  must be >= AES256_IVSize.
 *  @return the encrypted data or an empty string. */
std::string SymmEncrypt(const std::string &input,
                        const std::string &key,
                        const std::string &initialisation_vector);

/** Symmetrically decrypt a string.
 *  Performs symmetric decrytion using AES256. It returns an empty string if the
 *  key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
 *  @param input string to be decrypted.
 *  @param key key used to encrypt.  Size must be >= AES256_KeySize.
 *  @param initialisation_vector initialisation vector used to encrypt.  Size
 *  must be >= AES256_IVSize.
 *  @return the decrypted data or an empty string. */
std::string SymmDecrypt(const std::string &input,
                        const std::string &key,
                        const std::string &initialisation_vector);


/** Compress a string.
 *  Compress a string using gzip.  Compression level must be between 0 and 9
 *  inclusive or function returns an empty string.
 *  @param input string to be compressed.
 *  @param compression_level level of compression.
 *  @return the compressed data or an empty string. */
std::string Compress(const std::string &input,
                     const uint16_t &compression_level);

/** Uncompress a string.
 *  Uncompress a string using gzip.
 *  @param input string to be uncompressed.
 *  @return the uncompressed data or an empty string. */
std::string Uncompress(const std::string &input);

}  // namespace crypto

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_H_
