/*  Copyright 2014 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

/*
  SipHash reference C implementation

  Copyright (c) 2012-2014 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
  Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

  To the extent possible under law, the author(s) have dedicated all copyright
  and related and neighboring rights to this software to the public domain
  worldwide. This software is distributed without any warranty.
  You should have received a copy of the CC0 Public Domain Dedication along with
  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

// Note: The reference version was modified to separate the finalize stage, allowing
// for non-contiguous bytes. The original document describing SipHash was used to
// ensure accuracy.

#include "maidsafe/common/hash/algorithms/siphash.h"

#include <algorithm>
#include <cstring>

// Visual studio warns about the do {} while (0) loop in the macro from SipHash.
// Only change if you are confident in your abilities ...
#ifdef _MSC_VER
#pragma warning(disable : 4127)
#endif

#if defined(ROTL) || defined(U32TO8_LE) || defined(U64TO8_LE) || defined(U8TO64_LE) || \
    defined(SIPROUND)
#error Previous definition for these macros not expected
#endif

#define ROTL(x, b) static_cast<uint64_t>(((x) << (b)) | ((x) >> (64 - (b)))) /*NOLINT*/

#define U32TO8_LE(p, v)                     \
  (p)[0] = static_cast<uint8_t>((v));       \
  (p)[1] = static_cast<uint8_t>((v) >> 8);  \
  (p)[2] = static_cast<uint8_t>((v) >> 16); \
  (p)[3] = static_cast<uint8_t>((v) >> 24);

#define U64TO8_LE(p, v)                                  \
  U32TO8_LE((p), static_cast<uint32_t>((v))); /*NOLINT*/ \
  U32TO8_LE((p) + 4, static_cast<uint32_t>((v) >> 32));

#define U8TO64_LE(p)                                                                   \
  ((static_cast<uint64_t>((p)[0])) | (static_cast<uint64_t>((p)[1]) << 8) | /*NOLINT*/ \
   (static_cast<uint64_t>((p)[2]) << 16) |                                  /*NOLINT*/ \
   (static_cast<uint64_t>((p)[3]) << 24) |                                  /*NOLINT*/ \
   (static_cast<uint64_t>((p)[4]) << 32) |                                  /*NOLINT*/ \
   (static_cast<uint64_t>((p)[5]) << 40) |                                  /*NOLINT*/ \
   (static_cast<uint64_t>((p)[6]) << 48) |                                  /*NOLINT*/ \
   (static_cast<uint64_t>((p)[7]) << 56))                                   /*NOLINT*/

#define SIPROUND       \
  do {                 \
    v0 += v1;          \
    v1 = ROTL(v1, 13); \
    v1 ^= v0;          \
    v0 = ROTL(v0, 32); \
    v2 += v3;          \
    v3 = ROTL(v3, 16); \
    v3 ^= v2;          \
    v0 += v3;          \
    v3 = ROTL(v3, 21); \
    v3 ^= v0;          \
    v2 += v1;          \
    v1 = ROTL(v1, 17); \
    v1 ^= v2;          \
    v2 = ROTL(v2, 32); \
  } while (0)

namespace maidsafe {

namespace {

/* default: SipHash-2-4 */
const unsigned cRounds = 2;
const unsigned dRounds = 4;

std::uint64_t FinalizeInternal(std::uint64_t v0, std::uint64_t v1, std::uint64_t v2,
                               std::uint64_t v3, std::uint8_t b_in, const byte* in,
                               unsigned inlen) MAIDSAFE_NOEXCEPT {
  assert(inlen < 8);

  //
  // Compress remainder bytes
  //
  std::uint64_t b = (static_cast<std::uint64_t>(b_in + inlen)) << 56;

  switch (inlen & 7) {
    case 7:
      b |= static_cast<std::uint64_t>(in[6]) << 48;
    case 6:
      b |= static_cast<std::uint64_t>(in[5]) << 40;
    case 5:
      b |= static_cast<std::uint64_t>(in[4]) << 32;
    case 4:
      b |= static_cast<std::uint64_t>(in[3]) << 24;
    case 3:
      b |= static_cast<std::uint64_t>(in[2]) << 16;
    case 2:
      b |= static_cast<std::uint64_t>(in[1]) << 8;
    case 1:
      b |= static_cast<std::uint64_t>(in[0]);
      break;
    case 0:
      break;
  }

  v3 ^= b;

  for (unsigned i = 0; i < cRounds; ++i)
    SIPROUND;

  v0 ^= b;

  //
  // Finalize
  //
  v2 ^= 0xff;

  for (unsigned i = 0; i < dRounds; ++i)
    SIPROUND;

  return std::uint64_t(v0 ^ v1 ^ v2 ^ v3);
}

}  // namespace

SipHash::SipHash(const std::array<byte, kKeySize>& seed) MAIDSAFE_NOEXCEPT
    : /* "somepseudorandomlygeneratedbytes" */
      v0(0x736f6d6570736575ULL),
      v1(0x646f72616e646f6dULL),
      v2(0x6c7967656e657261ULL),
      v3(0x7465646279746573ULL),
      remainder_length_(0),
      remainder_(),
      b(0) {
  const std::uint64_t k0 = U8TO64_LE(seed.data());
  const std::uint64_t k1 = U8TO64_LE(seed.data() + sizeof(k0));

  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;
}

unsigned SipHash::Compress(const byte* in, std::uint64_t inlen) MAIDSAFE_NOEXCEPT {
  const uint8_t* const end = in + inlen - (inlen % sizeof(uint64_t));
  const unsigned left = inlen & 7;

  for (; in != end; in += 8) {
    const std::uint64_t m = U8TO64_LE(in);
    v3 ^= m;

    for (unsigned i = 0; i < cRounds; ++i)
      SIPROUND;

    v0 ^= m;
  }

  return left;
}

void SipHash::Update(const byte* in, std::uint64_t inlen) MAIDSAFE_NOEXCEPT {
  assert(remainder_length_ < remainder_.size());

  if (remainder_length_ > 0) {
    const std::uint64_t copy_length =
        std::min<std::uint64_t>(inlen, remainder_.size() - remainder_length_);

    std::copy(in, in + copy_length, remainder_.data() + remainder_length_);

    in += copy_length;
    inlen -= copy_length;

    remainder_length_ = unsigned(remainder_length_ + copy_length);
    b = std::uint8_t(b + copy_length);

    remainder_length_ = Compress(remainder_.data(), remainder_length_);
  }

  const unsigned left = Compress(in, inlen);

  assert(left <= inlen);
  assert(remainder_length_ + left < remainder_.size());
  std::copy(in + (inlen - left), in + inlen, remainder_.data() + remainder_length_);

  remainder_length_ += left;
  b = std::uint8_t(b + inlen);
}

std::uint64_t SipHash::Finalize() const MAIDSAFE_NOEXCEPT {
  // Copy state, so this object isn't actually finalized
  assert(remainder_length_ < remainder_.size());
  return FinalizeInternal(v0, v1, v2, v3, b, remainder_.data(), remainder_length_);
}

}  // namespace maidsafe
