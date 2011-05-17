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

#include "maidsafe/common/memory_chunk_store.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

std::string MemoryChunkStore::Get(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return "";
 
  return it->second.second;
}

bool MemoryChunkStore::Get(const std::string &name,
                           const fs::path &sink_file_name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return false;
  
  return WriteFile(sink_file_name, it->second.second);
}

bool MemoryChunkStore::Store(const std::string &name,
                             const std::string &content) {
  if (name.empty())
    return false;

  auto it = chunks_.find(name);
  if (it != chunks_.end()) {
    if (kReferenceCounting)
      ++(it->second.first);
    return true;
  }

  std::uintmax_t chunk_size(content.size());
  if (chunk_size == 0 || !Vacant(chunk_size))
    return false;

  chunks_[name] = ChunkEntry(1, content);
  IncreaseSize(chunk_size);
  return true;
}

bool MemoryChunkStore::Store(const std::string &name,
                             const fs::path &source_file_name,
                             bool delete_source_file) {
  if (name.empty())
    return false;

  boost::system::error_code ec;
  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    std::uintmax_t chunk_size(fs::file_size(source_file_name, ec));
    if (ec || chunk_size == 0 || !Vacant(chunk_size))
      return false;

    std::string content;
    if (!ReadFile(source_file_name, &content) || content.size() != chunk_size)
      return false;

    chunks_[name] = ChunkEntry(1, content);
    IncreaseSize(chunk_size);
  } else if (kReferenceCounting) {
    ++(it->second.first);
  }

  if (delete_source_file)
    fs::remove(source_file_name, ec);

  return true;
}

bool MemoryChunkStore::Delete(const std::string &name) {
  if (name.empty())
    return false;

  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return true;

  if (!kReferenceCounting || --(it->second.first) == 0) {
    DecreaseSize(it->second.second.size());
    chunks_.erase(it);
  }
  return true;
}

bool MemoryChunkStore::MoveTo(const std::string &name,
                              ChunkStore *sink_chunk_store) {
  if (name.empty() || !sink_chunk_store)
    return false;

  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return false;

  if (!sink_chunk_store->Store(name, it->second.second))
    return false;

  if (!kReferenceCounting || --(it->second.first) == 0) {
    DecreaseSize(it->second.second.size());
    chunks_.erase(it);
  }
  return true;
}

bool MemoryChunkStore::Has(const std::string &name) const {
  return chunks_.count(name) > 0;
}

bool MemoryChunkStore::Validate(const std::string &name) const {
  if (!hash_func_)
    return false;

  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return false;

  return name == hash_func_(it->second.second);
}

std::uintmax_t MemoryChunkStore::Size(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return 0;

  return it->second.second.size();
}

std::uintmax_t MemoryChunkStore::Count(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return 0;

  return it->second.first;
}

std::uintmax_t MemoryChunkStore::Count() const {
  return chunks_.size();
}

bool MemoryChunkStore::Empty() const {
  return chunks_.empty();
}

void MemoryChunkStore::Clear() {
  chunks_.clear();
  ChunkStore::Clear();
}

}  // namespace maidsafe
