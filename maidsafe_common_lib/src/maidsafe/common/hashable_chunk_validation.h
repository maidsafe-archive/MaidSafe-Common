/* Copyright (c) 2011 maidsafe.net limited
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

/**
 * @file hashable_chunk_validation.h
 * @brief Implementation of HashableChunkValidation.
 */

#ifndef MAIDSAFE_COMMON_HASHABLE_CHUNK_VALIDATION_H_
#define MAIDSAFE_COMMON_HASHABLE_CHUNK_VALIDATION_H_

#include <string>
#include "boost/filesystem/path.hpp"
#include "maidsafe/common/chunk_validation.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1005
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

/**
 * Class to validate chunks based on their name.
 *
 * @tparam ValidationType Type of hashing algo used for validity checks
 * @tparam VersionType Type of hashing algo used for versioning
 */
template <class ValidationType, class VersionType>
class HashableChunkValidation : public ChunkValidation {
 public:
  HashableChunkValidation() : ChunkValidation() {}
  ~HashableChunkValidation() {}

  /**
   * Checks if a chunk's name is in a valid, known format.
   * @param name Chunk name
   * @return Whether chunk name is valid.
   */
  bool ValidName(const std::string &name) {
    return !name.empty();
  }

  /**
   * Checks if the hash of a chunk's content is supposed to match its name.
   * @param name Chunk name
   * @return Whether chunk is hashable.
   */
  bool Hashable(const std::string &name) {
    return name.size() == size_t(ValidationType::DIGESTSIZE);
  }

  /**
   * Checks if a chunk's content can be changed after it's been created.
   * @param name Chunk name
   * @return Whether chunk is modifiable.
   */
  bool Modifiable(const std::string &name) {
    return ValidName(name) && !Hashable(name);
  }

  /**
   * Checks if a chunk is valid.
   * @param name Chunk name
   * @param content Chunk's content as string.
   * @return Whether chunk is valid.
   */
  bool ValidChunk(const std::string &name, const std::string &content) {
    if (!Hashable(name))
      return ValidName(name);

    return name == crypto::Hash<ValidationType>(content);
  }

  /**
   * Checks if a chunk is valid.
   * @param name Chunk name
   * @param path Path to chunk's content.
   * @return Whether chunk is valid.
   */
  bool ValidChunk(const std::string &name, const fs::path &path) {
    if (!Hashable(name))
      return ValidName(name);

    return name == crypto::HashFile<ValidationType>(path);
  }

  /**
   * Returns the version of a chunk's contents.
   * @param name Chunk name
   * @param content Chunk's content as string
   * @return The chunk version.
   */
  std::string Version(const std::string &name, const std::string &content) {
    if (Hashable(name))
      return name;

    if (!ValidName(name))
      return "";

    return crypto::Hash<VersionType>(content);
  }

  /**
   * Returns the version of a chunk's contents.
   * @param name Chunk name
   * @param path Path to chunk's content
   * @return The chunk version.
   */
  std::string Version(const std::string &name, const fs::path &path) {
    if (Hashable(name))
      return name;

    if (!ValidName(name))
      return "";

    return crypto::HashFile<VersionType>(path);
  }

 private:
  HashableChunkValidation(const HashableChunkValidation&);
  HashableChunkValidation& operator=(const HashableChunkValidation&);
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASHABLE_CHUNK_VALIDATION_H_
