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
#include <vector>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4355 4702)
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

#include "maidsafe/common/log.h"
#include "maidsafe/common/rsa.h"


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


extern const uint16_t AES256_KeySize;
extern const uint16_t AES256_IVSize;
extern const uint16_t kMaxCompressionLevel;

extern const std::string kMaidSafeVersionLabel1;
extern const std::string kMaidSafeVersionLabel;

// Performs a bitwise XOR on each char of first with the corresponding char of second.  first and
// second must have identical non-zero size or a std::exception will be thrown.
std::string XOR(const std::string& first, const std::string& second);

// Creates a secure password using the Password-Based Key Derivation Function (PBKDF) version 2
// algorithm.  The number of iterations is derived from "pin".  "label" is additional data to
// provide distinct input data to PBKDF.  The function will throw a std::exception if invalid
// parameters are passed.
std::string SecurePassword(const std::string& password,
                           const std::string& salt,
                           const uint32_t& pin,
                           const std::string& label = kMaidSafeVersionLabel);

// Hash function operating on a string.
template <class HashType>
std::string Hash(const std::string& input) {
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
  return result;
}

// Hash function operating on a file.
template <class HashType>
std::string HashFile(const boost::filesystem::path& file_path) {
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
  return result;
}

// Performs symmetric encrytion using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
std::string SymmEncrypt(const std::string& input,
                        const std::string& key,
                        const std::string& initialisation_vector);

// Performs symmetric decrytion using AES256. It throws a std::exception if the
// key size < AES256_KeySize or if initialisation_vector size < AES256_IVSize.
std::string SymmDecrypt(const std::string& input,
                        const std::string& key,
                        const std::string& initialisation_vector);

// Compress a string using gzip.  Compression level must be between 0 and 9
// inclusive or function throws a std::exception.
std::string Compress(const std::string& input,
                     const uint16_t& compression_level);

// Uncompress a string using gzip.  Will throw a std::exception if uncompression fails.
std::string Uncompress(const std::string& input);

std::vector<std::string> SecretShareData(const int32_t& threshold,
                                         const int32_t& number_of_shares,
                                         const std::string& data);

std::string SecretRecoverData(const int32_t& threshold, const std::vector<std::string>& in_strings);

}  // namespace crypto

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CRYPTO_H_
