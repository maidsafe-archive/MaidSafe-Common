/* Copyright (c) 2012 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   *  Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
   *  Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
   *  Neither the name of the maidsafe.net limited nor the names of its
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

#ifndef MAIDSAFE_COMMON_TYPES_H_
#define MAIDSAFE_COMMON_TYPES_H_

#include <cstdint>
#include <type_traits>
#include <utility>

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/tagged_value.h"


namespace maidsafe {

typedef detail::BoundedString<1> NonEmptyString;
typedef detail::BoundedString<64, 64> Identity;
typedef NonEmptyString UserPassword;

typedef TaggedValue<uint64_t, struct MemoryUsageTag> MemoryUsage;
typedef TaggedValue<uint64_t, struct DiskUsageTag> DiskUsage;

template<typename T>
struct is_long_term_cacheable : public std::false_type {};

template<typename T>
struct is_short_term_cacheable : public std::false_type {};

template<typename T>
struct is_payable : public std::true_type {};

template<typename T>
struct is_unique_on_network : public std::true_type {};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TYPES_H_
