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
 * @file chunk_action_authority.h
 * @brief Declaration of ChunkActionAuthority interface.
 */

#ifndef MAIDSAFE_COMMON_CHUNK_ACTION_AUTHORITY_H_
#define MAIDSAFE_COMMON_CHUNK_ACTION_AUTHORITY_H_

#include <memory>
#include <string>
#include "boost/filesystem/path.hpp"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1005
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace fs = boost::filesystem;

namespace maidsafe {

class ChunkStore;


/**
 * Abstract class to validate chunks, and requested actions on chunks.
 *
 * Implementations will need to be aware of different chunk types and their
 * inherent validity.
 */
class ChunkActionAuthority {
 public:
  enum OperationType { kStore, kDelete, kUpdate, kGet, kHas };

  explicit ChunkActionAuthority(std::shared_ptr<ChunkStore> chunk_store)
      : chunk_store_(chunk_store) {}
  virtual ~ChunkActionAuthority() {}

  /**
   * Checks if op_type is valid for the given chunk content.
   * @param op_type Ennumeration indicating the requested operation type
   * @param name Chunk name
   * @param content Chunk's content as string
   * @param public_key Public key used to validate reauest against existing data
   * @param chunk_store ChunkStore used to retrieve existing data if required
   * @param new_content Chunk's new (possibly modified) content to replace /
   * be stored under "name".  A NULL pointer may be passed here.
   * @return kSuccess where the requested operation is valid.  Any other value
   * indicates an invalid request.
   */
  virtual int ValidOperation(const int &op_type,
                             const std::string &name,
                             const std::string &content,
                             const std::string &version,
                             const asymm::PublicKey &public_key,
                             std::string *new_content = NULL) const = 0;

  /**
   * Checks if op_type is valid for the given chunk held as a file.
   * @param op_type Ennumeration indicating the requested operation type
   * @param name Chunk name
   * @param path Path to chunk's content
   * @param public_key Public key used to validate reauest against existing data
   * @param chunk_store ChunkStore used to retrieve existing data if required
   * @param new_content Chunk's new (possibly modified) content to replace /
   * be stored under "name".  A NULL pointer may be passed here.
   * @return kSuccess where the requested operation is valid.  Any other value
   * indicates an invalid request.
   */
  virtual int ValidOperationOnFile(const int &op_type,
                                   const std::string &name,
                                   const fs::path &path,
                                   const std::string &version,
                                   const asymm::PublicKey &public_key,
                                   std::string *new_content = NULL) const = 0;

  /**
  * Checks if a chunk's name is in a valid, known format.
  * @param name Chunk name
  * @return Whether chunk name is valid.
  */
  virtual bool ValidName(const std::string &name) const = 0;

  /**
   * Checks if a chunk is suitable for caching based on its type.
   * @param name Chunk name
   * @return Whether chunk is casheable.
   */
  virtual bool Cacheable(const std::string &name) const = 0;

  /**
   * Checks if a chunk is valid.
   * @param name Chunk name
   * @return Whether chunk is valid.
   */
  virtual bool ValidChunk(const std::string &name) const = 0;

  /**
   * Returns the version of a chunk's contents.
   * @param name Chunk name
   * @return The chunk version.
   */
  virtual std::string Version(const std::string &name) const = 0;

 protected:
  std::shared_ptr<ChunkStore> chunk_store_;

 private:
  ChunkActionAuthority(const ChunkActionAuthority&);
  ChunkActionAuthority& operator=(const ChunkActionAuthority&);
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CHUNK_ACTION_AUTHORITY_H_
