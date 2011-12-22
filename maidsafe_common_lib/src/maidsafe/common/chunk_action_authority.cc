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

#include "maidsafe/common/chunk_action_authority.h"
#include "maidsafe/common/chunk_store.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/return_codes.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

ChunkActionAuthority::ChunkActionAuthority(
    std::shared_ptr<ChunkStore> chunk_store)
        : chunk_store_(chunk_store) {}

ChunkActionAuthority::~ChunkActionAuthority() {}

std::string ChunkActionAuthority::Get(
      const std::string &name,
      const std::string &version,
      const asymm::PublicKey &public_key) const {
  std::string existing_content;
  int result(ValidGet(name, version, public_key, &existing_content));
  if (result != kSuccess) {
    DLOG(WARNING) << "Failed to get " << Base32Substr(name) << ": " << result;
    existing_content.clear();
  }

  return existing_content;
}

bool ChunkActionAuthority::Get(const std::string &name,
                               const fs::path &sink_file_name,
                               const std::string &version,
                               const asymm::PublicKey &public_key) const {
  std::string existing_content;
  int result(ValidGet(name, version, public_key, &existing_content));
  if (result != kSuccess) {
    DLOG(WARNING) << "Failed to get " << Base32Substr(name) << ": " << result;
    return false;
  }

  if (!WriteFile(sink_file_name, existing_content)) {
    DLOG(ERROR) << "Failed to write chunk " << Base32Substr(name) << " to "
                << sink_file_name;
    return false;
  }

  return true;
}

bool ChunkActionAuthority::Store(const std::string &name,
                                 const std::string &content,
                                 const asymm::PublicKey &public_key) {
  int result(ValidStore(name, content, public_key));
  if (result != kSuccess) {
    DLOG(ERROR) << "Invalid request to store " << Base32Substr(name) << ": "
                << result;
    return false;
  }

  if (!chunk_store_->Store(name, content)) {
    DLOG(ERROR) << "Failed to store " << Base32Substr(name);
    return false;
  }

  return true;
}

bool ChunkActionAuthority::Store(const std::string &name,
                                 const fs::path &source_file_name,
                                 bool delete_source_file,
                                 const asymm::PublicKey &public_key) {
  std::string content;
  if (!ReadFile(source_file_name, &content)) {
    DLOG(ERROR) << "Failed to read " << source_file_name;
    return false;
  }

  int result(ValidStore(name, content, public_key));
  if (result != kSuccess) {
    DLOG(ERROR) << "Invalid request to store " << Base32Substr(name) << ": "
                << result;
    return false;
  }

  if (!chunk_store_->Store(name, content)) {
    DLOG(ERROR) << "Failed to store " << Base32Substr(name);
    return false;
  }

  if (delete_source_file) {
    boost::system::error_code error_code;
#ifdef DEBUG
    bool removed(fs::remove(source_file_name, error_code));
    if (!removed) {
      DLOG(WARNING) << "Failed to remove source file " << source_file_name
                    << (error_code ? (": " + error_code.message()) : "");
    }
#else
    fs::remove(source_file_name, error_code);
#endif
  }

  return true;
}

bool ChunkActionAuthority::Delete(const std::string &name,
                                  const std::string &version,
                                  const std::string &ownership_proof,
                                  const asymm::PublicKey &public_key) {
  int result(ValidDelete(name, version, ownership_proof, public_key));
  if (result != kSuccess) {
    DLOG(ERROR) << "Invalid request to delete " << Base32Substr(name) << ": "
                << result;
    return false;
  }

  if (!chunk_store_->Delete(name)) {
    DLOG(ERROR) << "Failed to delete " << Base32Substr(name);
    return false;
  }

  return true;
}

bool ChunkActionAuthority::Modify(const std::string &name,
                                  const std::string &content,
                                  const std::string &version,
                                  const asymm::PublicKey &public_key) {
  std::string new_content;
  int result(ValidModify(name, content, version, public_key, &new_content));
  if (result != kSuccess) {
    DLOG(ERROR) << "Invalid request to modify " << Base32Substr(name) << ": "
                << result;
    return false;
  }

  if (!chunk_store_->Modify(name, new_content)) {
    DLOG(ERROR) << "Failed to modify " << Base32Substr(name);
    return false;
  }

  return true;
}

bool ChunkActionAuthority::Modify(const std::string &name,
                                  const fs::path &source_file_name,
                                  bool delete_source_file,
                                  const std::string &version,
                                  const asymm::PublicKey &public_key) {
  std::string content;
  if (!ReadFile(source_file_name, &content)) {
    DLOG(ERROR) << "Failed to read " << source_file_name;
    return false;
  }

  std::string new_content;
  int result(ValidModify(name, content, version, public_key, &new_content));
  if (result != kSuccess) {
    DLOG(ERROR) << "Invalid request to modify " << Base32Substr(name) << ": "
                << result;
    return false;
  }

  if (!chunk_store_->Modify(name, new_content)) {
    DLOG(ERROR) << "Failed to modify " << Base32Substr(name);
    return false;
  }

  if (delete_source_file) {
    boost::system::error_code error_code;
#ifdef DEBUG
    bool removed(fs::remove(source_file_name, error_code));
    if (!removed) {
      DLOG(WARNING) << "Failed to remove source file " << source_file_name
                    << (error_code ? (": " + error_code.message()) : "");
    }
#else
    fs::remove(source_file_name, error_code);
#endif
  }

  return true;
}

bool ChunkActionAuthority::Has(const std::string &name,
                               const std::string &version,
                               const asymm::PublicKey &public_key) const {
  int result(ValidHas(name, version, public_key));
  if (result != kSuccess) {
    DLOG(WARNING) << "Invalid request or doesn't have " << Base32Substr(name)
                  << ": " << result;
    return false;
  }

  return true;
}

}  // namespace maidsafe
