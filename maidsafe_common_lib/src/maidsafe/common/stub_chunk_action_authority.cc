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

#include "maidsafe/common/stub_chunk_action_authority.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/return_codes.h"


namespace maidsafe {

int StubChunkActionAuthority::ValidOperation(
    const int &/*op_type*/,
    const std::string &/*name*/,
    const std::string &/*content*/,
    const std::string &/*version*/,
    const asymm::PublicKey &/*public_key*/,
    std::string * /*new_content*/) const {
  return kSuccess;
}

int StubChunkActionAuthority::ValidOperationOnFile(
    const int &/*op_type*/,
    const std::string &/*name*/,
    const fs::path &/*path*/,
    const std::string &/*version*/,
    const asymm::PublicKey &/*public_key*/,
    std::string * /*new_content*/) const {
  return kSuccess;
}

bool StubChunkActionAuthority::ValidName(const std::string &name) const {
  return !name.empty();
}

bool StubChunkActionAuthority::Cacheable(const std::string &/*name*/) const {
  return true;
}

bool StubChunkActionAuthority::ValidChunk(const std::string &/*name*/) const {
  return true;
}

std::string StubChunkActionAuthority::Version(const std::string &name) const {
  return name;
}


}  // namespace maidsafe
