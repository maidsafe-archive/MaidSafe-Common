/* Copyright (c) 2012 maidsafe.net limited
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

#include "maidsafe/common/key_value_buffer.h"


namespace maidsafe {

KeyValueBuffer::KeyValueBuffer(MemoryUsage /*max_memory_usage*/, DiskUsage /*max_disk_usage*/) {}

KeyValueBuffer::KeyValueBuffer(MemoryUsage /*max_memory_usage*/,
                               DiskUsage /*max_disk_usage*/,
                               const boost::filesystem::path& /*disk_root*/) {}

KeyValueBuffer::~KeyValueBuffer() {}

void KeyValueBuffer::Store(const Identity& /*key*/, const NonEmptyString& /*value*/) {}

NonEmptyString KeyValueBuffer::Get(const Identity& /*key*/) { return NonEmptyString("r"); }

void KeyValueBuffer::Delete(const Identity& /*key*/) {}

void KeyValueBuffer::SetMaxMemoryUsage(MemoryUsage /*max_memory_usage*/) {}

void KeyValueBuffer::SetMaxDiskUsage(DiskUsage /*max_disk_usage*/) {}

}  // namespace maidsafe
