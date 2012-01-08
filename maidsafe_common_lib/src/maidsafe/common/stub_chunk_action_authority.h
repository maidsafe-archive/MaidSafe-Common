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

#ifndef MAIDSAFE_COMMON_STUB_CHUNK_ACTION_AUTHORITY_H_
#define MAIDSAFE_COMMON_STUB_CHUNK_ACTION_AUTHORITY_H_

#include <string>

#include "maidsafe/common/chunk_action_authority.h"
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1005
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace maidsafe {

class StubChunkActionAuthority : public ChunkActionAuthority {
 public:
  explicit StubChunkActionAuthority(std::shared_ptr<ChunkStore> chunk_store)
      : ChunkActionAuthority(chunk_store) {}
  virtual ~StubChunkActionAuthority() {}
  virtual bool ValidName(const std::string &name) const;
  virtual bool Cacheable(const std::string &name) const;
  virtual bool ValidChunk(const std::string &name) const;
  virtual std::string Version(const std::string &name) const;

 protected:
  virtual int ValidGet(const std::string &name,
                       const std::string &version,
                       const asymm::PublicKey &public_key,
                       std::string *existing_content = NULL) const;
  virtual int ValidStore(const std::string &name,
                         const std::string &content,
                         const asymm::PublicKey &public_key) const;
  virtual int ValidDelete(const std::string &name,
                          const std::string &version,
                          const std::string &ownership_proof,
                          const asymm::PublicKey &public_key) const;
  virtual int ValidModify(const std::string &name,
                          const std::string &content,
                          const std::string &version,
                          const asymm::PublicKey &public_key,
                          std::string *new_content = NULL) const;
  virtual int ValidHas(const std::string &name,
                       const std::string &version,
                       const asymm::PublicKey &public_key) const;

 private:
  StubChunkActionAuthority(const StubChunkActionAuthority&);
  StubChunkActionAuthority& operator=(const StubChunkActionAuthority&);
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_STUB_CHUNK_ACTION_AUTHORITY_H_
