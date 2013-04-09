/* Copyright (c) 2013 maidsafe.net limited
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

#ifndef MAIDSAFE_COMMON_BLOOM_FILTER_H_
#define MAIDSAFE_COMMON_BLOOM_FILTER_H_

#include "boost/dynamic_bitset.hpp"

#include "maidsafe/common/types.h"


namespace maidsafe {

class BloomFilter {
 public:
  static const size_t kHashFunctionsCount;

  BloomFilter();
  explicit BloomFilter(size_t bit_capacity);
  BloomFilter(const BloomFilter& other);
  BloomFilter(BloomFilter&& other);
  BloomFilter& operator=(BloomFilter other);

  size_t InsertionCountEstimate() const;
  double FalsePositiveRateEstimate() const;
  size_t BitsetCount() const { return bitset_.count(); }
  size_t BitCapacity() const { return bitset_.size(); }

  void Insert(const Identity& identity);
  template<typename InputIterator>
  void Insert(InputIterator start, InputIterator end) {
    while (start != end)
      Insert(*start++);
  }

  bool ProbablyContains(const Identity& identity) const;

  void Clear() { bitset_.reset(); }

  BloomFilter& operator|=(const BloomFilter& other);
  BloomFilter& operator&=(const BloomFilter& other);

  friend bool operator==(const BloomFilter& lhs, const BloomFilter& rhs);
  friend void swap(BloomFilter& lhs, BloomFilter& rhs) MAIDSAFE_NOEXCEPT;  // NOLINT (Fraser)

 private:
  void CheckSizesMatch(const BloomFilter& other) const;

  boost::dynamic_bitset<size_t> bitset_;
};


inline BloomFilter operator|(BloomFilter lhs, const BloomFilter& rhs) {
  lhs |= rhs;
  return lhs;
}

inline BloomFilter operator&(BloomFilter lhs, const BloomFilter& rhs) {
  lhs &= rhs;
  return lhs;
}

inline bool operator==(const BloomFilter& lhs, const BloomFilter& rhs) {
  return lhs.bitset_ == rhs.bitset_;
}

inline bool operator!=(const BloomFilter& lhs, const BloomFilter& rhs) {
  return !operator==(lhs, rhs);
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BLOOM_FILTER_H_
