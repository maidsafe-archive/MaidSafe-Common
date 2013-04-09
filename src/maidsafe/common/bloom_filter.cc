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

#include "maidsafe/common/bloom_filter.h"

#include <cmath>

#include "boost/mpl/at.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"


namespace maidsafe {

namespace {

size_t IdentityHash(const Identity& identity, size_t part) {
  assert((8U * part) < crypto::SHA512::DIGESTSIZE);
  return *static_cast<const size_t*>(
      static_cast<const void*>(identity.string().c_str() + (8 * part)));
}

}  // unnamed namespace


const size_t BloomFilter::kHashFunctionsCount = 7;

BloomFilter::BloomFilter() : bitset_(0) {}

BloomFilter::BloomFilter(size_t bit_capacity) : bitset_(bit_capacity) {}

BloomFilter::BloomFilter(const BloomFilter& other) : bitset_(other.bitset_) {}

BloomFilter::BloomFilter(BloomFilter&& other) : bitset_(std::move(other.bitset_)) {}

BloomFilter& BloomFilter::operator=(BloomFilter other) {
  swap(*this, other);
  return *this;
}

size_t BloomFilter::InsertionCountEstimate() const {
  const double N(static_cast<double>(bitset_.size()));
  const double x(static_cast<double>(bitset_.count()));
  const double k(static_cast<double>(kHashFunctionsCount));
  return static_cast<size_t>(-(N * std::log(1 - (x / N)) / k));
}

double BloomFilter::FalsePositiveRateEstimate() const {
  const double n(static_cast<double>(InsertionCountEstimate()));
  static const double k(static_cast<double>(kHashFunctionsCount));
  const double m(static_cast<double>(bitset_.size()));
  return std::pow(1 - std::pow(1 - 1 / m, k * n), k);
}

void BloomFilter::Insert(const Identity& identity) {
  for (size_t i(0); i != kHashFunctionsCount; ++i) {
    size_t index(IdentityHash(identity, i) % bitset_.size());
    bitset_.set(index, true);
  }
}

bool BloomFilter::ProbablyContains(const Identity& identity) const {
  for (size_t i(0); i != kHashFunctionsCount; ++i) {
    size_t index(IdentityHash(identity, i) % bitset_.size());
    if (!bitset_.test(index))
      return false;
  }
  return true;
}

void BloomFilter::CheckSizesMatch(const BloomFilter& other) const {
  if (bitset_.size() != other.bitset_.size())
    ThrowError(CommonErrors::invalid_parameter);
}

BloomFilter& BloomFilter::operator|=(const BloomFilter& other) {
  CheckSizesMatch(other);
  bitset_ |= other.bitset_;
  return *this;
}

BloomFilter& BloomFilter::operator&=(const BloomFilter& other) {
  CheckSizesMatch(other);
  bitset_ &= other.bitset_;
  return *this;
}

void swap(BloomFilter& lhs, BloomFilter& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.bitset_, rhs.bitset_);
}

}  // namespace maidsafe
