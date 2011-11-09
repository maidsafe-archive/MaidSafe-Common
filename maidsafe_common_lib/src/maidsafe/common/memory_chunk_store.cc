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
#include "maidsafe/common/utils.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

std::string MemoryChunkStore::Get(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    DLOG(WARNING) << "Can't get chunk " << HexSubstr(name);
    return "";
  }

  return (*it).second.second;
}

bool MemoryChunkStore::Get(const std::string &name,
                           const fs::path &sink_file_name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    DLOG(WARNING) << "Can't get chunk " << HexSubstr(name);
    return false;
  }

  return WriteFile(sink_file_name, (*it).second.second);
}

bool MemoryChunkStore::Store(const std::string &name,
                             const std::string &content) {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name)) {
    DLOG(ERROR) << "Failed to validate chunk " << HexSubstr(name);
    return false;
  }

  auto it = chunks_.find(name);
  if (it != chunks_.end()) {
    if (kReferenceCounting) {
      ++(*it).second.first;
      DLOG(INFO) << "Increased count of chunk " << HexSubstr(name) << " to "
                 << (*it).second.first;
    } else {
      DLOG(INFO) << "Already stored chunk " << HexSubstr(name);
    }
    return true;
  }

  std::uintmax_t chunk_size(content.size());
  if (chunk_size == 0) {
    DLOG(ERROR) << "Chunk " << HexSubstr(name) << " has size 0";
    return false;
  }

  if (!Vacant(chunk_size)) {
    DLOG(ERROR) << "Chunk " << HexSubstr(name) << " has size " << chunk_size
                << " > vacant space";
    return false;
  }

  chunks_[name] = ChunkEntry(1, content);
  IncreaseSize(chunk_size);
  DLOG(INFO) << "Stored chunk " << HexSubstr(name);
  return true;
}

bool MemoryChunkStore::Store(const std::string &name,
                             const fs::path &source_file_name,
                             bool delete_source_file) {
  if (!chunk_validation_ || !chunk_validation_->ValidName(name)) {
    DLOG(ERROR) << "Failed to validate chunk " << HexSubstr(name);
    return false;
  }

  boost::system::error_code ec;
  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    std::uintmax_t chunk_size(fs::file_size(source_file_name, ec));
    if (ec) {
      DLOG(ERROR) << "Failed to caclulate size for chunk " << HexSubstr(name)
                  << ": " << ec.message();
      return false;
    }

    if (chunk_size == 0) {
      DLOG(ERROR) << "Chunk " << HexSubstr(name) << " has size 0";
      return false;
    }

    if (!Vacant(chunk_size)) {
      DLOG(ERROR) << "Chunk " << HexSubstr(name) << " has size " << chunk_size
                  << " > vacant space.";
      return false;
    }

    std::string content;
    if (!ReadFile(source_file_name, &content)) {
      DLOG(ERROR) << "Failed to read file for chunk " << HexSubstr(name);
      return false;
    }

    if (content.size() != chunk_size) {
      DLOG(ERROR) << "File content size " << content.size() << " != chunk_size "
                  << chunk_size << " for chunk " << HexSubstr(name);
      return false;
    }

    chunks_[name] = ChunkEntry(1, content);
    IncreaseSize(chunk_size);
    DLOG(INFO) << "Stored chunk " << HexSubstr(name);
  } else if (kReferenceCounting) {
    ++(*it).second.first;
    DLOG(INFO) << "Increased count of chunk " << HexSubstr(name) << " to "
               << (*it).second.first;
  }

  if (delete_source_file)
    fs::remove(source_file_name, ec);

  return true;
}

bool MemoryChunkStore::Delete(const std::string &name) {
  if (name.empty()) {
    DLOG(ERROR) << "Name empty";
    return false;
  }

  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    DLOG(INFO) << "Chunk " << HexSubstr(name) << " already deleted";
    return true;
  }

  if (!kReferenceCounting || --(*it).second.first == 0) {
    DecreaseSize((*it).second.second.size());
    chunks_.erase(it);
    return true;
  }

#ifdef DEBUG
  if (kReferenceCounting && (*it).second.first != 0) {
    DLOG(INFO) << "Decreased count of chunk " << HexSubstr(name) << " to "
               << (*it).second.first << " via deletion";
  }
#endif

  return true;
}

bool MemoryChunkStore::Modify(const std::string &name,
                              const std::string &content) {
  if (name.empty())
    return false;
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return false;
  if (!chunk_validation_ || !chunk_validation_->ValidName(name))
    return false;

  std::string current_content((*it).second.second);
  std::uintmax_t content_size_difference(
      content.size() - current_content.size());
  if (!Vacant(content_size_difference))
    return false;

  chunks_[name] = ChunkEntry((*it).second.first, content);

  if (content_size_difference > 0)
    IncreaseSize(content_size_difference);
  else if (content_size_difference < 0)
    DecreaseSize(content_size_difference);
  return true;
}

bool MemoryChunkStore::Modify(const std::string &name,
                              const fs::path &source_file_name) {
  if (source_file_name.empty())
    return false;
  std::string content;
  if (!ReadFile(source_file_name, &content))
    return false;
  return Modify(name, content);
}

bool MemoryChunkStore::MoveTo(const std::string &name,
                              ChunkStore *sink_chunk_store) {
  if (!sink_chunk_store) {
    DLOG(ERROR) << "NULL sink passed for chunk " << HexSubstr(name);
    return false;
  }

  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    DLOG(WARNING) << "Failed to find chunk " << HexSubstr(name);
    return false;
  }

  if (!sink_chunk_store->Store(name, (*it).second.second)) {
    DLOG(ERROR) << "Failed to store chunk " << HexSubstr(name) << " in sink";
    return false;
  }

  if (!kReferenceCounting || --(*it).second.first == 0) {
    DecreaseSize((*it).second.second.size());
    chunks_.erase(it);
    DLOG(INFO) << "Moved chunk " << HexSubstr(name);
  }

#ifdef DEBUG
  if (kReferenceCounting && (*it).second.first != 0) {
    DLOG(INFO) << "Decreased count of chunk " << HexSubstr(name) << " to "
               << (*it).second.first << " via move";
  }
#endif

  return true;
}

bool MemoryChunkStore::Has(const std::string &name) const {
  bool found(chunks_.find(name) != chunks_.end());
  DLOG(INFO) << (found ? "Have chunk " : "Do not have chunk ")
             << HexSubstr(name);
  return found;
}

bool MemoryChunkStore::Validate(const std::string &name) const {
  if (!chunk_validation_) {
    DLOG(ERROR) << "No validation available for chunk " << HexSubstr(name);
    return false;
  }

  auto it = chunks_.find(name);
  if (it == chunks_.end()) {
    DLOG(WARNING) << "Failed to find chunk " << HexSubstr(name);
    return false;
  }

  bool valid(chunk_validation_->ValidChunk(name, (*it).second.second));
  DLOG(INFO) << "Validation result for chunk " << HexSubstr(name) << ": "
             << std::boolalpha << valid;
  return valid;
}

std::uintmax_t MemoryChunkStore::Size(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return 0;

  return (*it).second.second.size();
}

std::uintmax_t MemoryChunkStore::Count(const std::string &name) const {
  auto it = chunks_.find(name);
  if (it == chunks_.end())
    return 0;

  return (*it).second.first;
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
