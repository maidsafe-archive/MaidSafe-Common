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

#include "maidsafe/common/chunk_store_disk.h"

namespace maidsafe {

std::string ChunkStoreDisk::Get(const std::string &name) {
  std::string chunk_content;

  return chunk_content;
}

bool ChunkStoreDisk::Get(const std::string &name,
                         const fs::path &sink_file_name) {
  bool success(false);

  return success;
}

bool ChunkStoreDisk::Store(const std::string &name,
                           const std::string &content) {
  bool success(false);

  return success;
}

bool ChunkStore::Store(const std::string &name,
                       const fs::path &source_file_name,
                       bool delete_source_file) {
  bool success(false);

  return success;
}

bool ChunkStoreDisk::Delete(const std::string &name) {
  bool success(false);

  return success;
}

bool ChunkStoreDisk::MoveTo(const std::string &name,
                            ChunkStore *sink_chunk_store) {
  bool success(false);

  return success;
}

bool ChunkStoreDisk::Has(const std::string &name) {
  bool found(false);

  return found;
}

std::uintmax_t ChunkStoreDisk::Size(const std::string &name) {
  std::uintmax_t size_of_chunk(0);

  return size_of_chunk;
}

bool ChunkStoreDisk::Validate(const std::string &name) {
  bool valid(false);

  return valid;
}

std::uintmax_t ChunkStoreDisk::Count() {
  std::uintmax_t chunk_count(0);

  return chunk_count;
}

bool ChunkStoreDisk::Empty() {
  bool is_empty(false);

  return is_empty;
}

void ChunkStoreDisk::Clear() {
}

}  // namespace maidsafe
