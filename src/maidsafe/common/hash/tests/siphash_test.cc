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
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/hash/algorithms/siphash.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

// Visual studio warns about the do {} while (0) loop in the macro from SipHash.
// Do not change the reference implementation for testing.
#ifdef _MSC_VER
#pragma warning (disable: 4127)
#endif

namespace maidsafe {
namespace test {

namespace {

//
// NOTE: The liberal usage of NOLINT was done to perserve the reference
// SipHash code, even it the change was merely style.
//

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

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

/* default: SipHash-2-4 */
#define cROUNDS 2
#define dROUNDS 4

#define ROTL(x,b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) ) /*NOLINT*/

#define U32TO8_LE(p, v)                                         \
  (p)[0] = (uint8_t)((v)      ); (p)[1] = (uint8_t)((v) >>  8); \
  (p)[2] = (uint8_t)((v) >> 16); (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                        \
  U32TO8_LE((p),     (uint32_t)((v)      ));   \
  U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));


#define U8TO64_LE(p)            \
  (((uint64_t)((p)[0])      ) | \
   ((uint64_t)((p)[1]) <<  8) | /*NOLINT*/ \
   ((uint64_t)((p)[2]) << 16) | /*NOLINT*/ \
   ((uint64_t)((p)[3]) << 24) | /*NOLINT*/ \
   ((uint64_t)((p)[4]) << 32) | /*NOLINT*/ \
   ((uint64_t)((p)[5]) << 40) | /*NOLINT*/ \
   ((uint64_t)((p)[6]) << 48) | /*NOLINT*/ \
   ((uint64_t)((p)[7]) << 56))  /*NOLINT*/

#define SIPROUND                                                   \
  do {                                                             \
    v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); /*NOLINT*/ \
    v2 += v3; v3=ROTL(v3,16); v3 ^= v2;                 /*NOLINT*/ \
    v0 += v3; v3=ROTL(v3,21); v3 ^= v0;                 /*NOLINT*/ \
    v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); /*NOLINT*/ \
  } while(0)                                            /*NOLINT*/

#ifdef DEBUG
#define TRACE                                                                \
    do {                                                                     \
    printf( "(%3d) v0 %08x %08x\n",                               /*NOLINT*/ \
        ( int )inlen, ( uint32_t )( v0 >> 32 ), ( uint32_t )v0 );            \
    printf( "(%3d) v1 %08x %08x\n",                               /*NOLINT*/ \
        ( int )inlen, ( uint32_t )( v1 >> 32 ), ( uint32_t )v1 );            \
    printf( "(%3d) v2 %08x %08x\n",                               /*NOLINT*/ \
        ( int )inlen, ( uint32_t )( v2 >> 32 ), ( uint32_t )v2 );            \
    printf( "(%3d) v3 %08x %08x\n",                               /*NOLINT*/ \
        ( int )inlen, ( uint32_t )( v3 >> 32 ), ( uint32_t )v3 );            \
    } while(0)  /*NOLINT*/
#else
#define TRACE
#endif

int siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t *k) {
  /* "somepseudorandomlygeneratedbytes" */
  uint64_t v0 = 0x736f6d6570736575ULL;
  uint64_t v1 = 0x646f72616e646f6dULL;
  uint64_t v2 = 0x6c7967656e657261ULL;
  uint64_t v3 = 0x7465646279746573ULL;
  uint64_t b;
  uint64_t k0 = U8TO64_LE( k );  //NOLINT
  uint64_t k1 = U8TO64_LE( k + 8 );  //NOLINT
  uint64_t m;
  int i;
  const uint8_t *end = in + inlen - ( inlen % sizeof( uint64_t ) ); //NOLINT
  const int left = inlen & 7;
  b = ( ( uint64_t )inlen ) << 56;  //NOLINT
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

#ifdef DOUBLE
  v1 ^= 0xee;
#endif

  for (; in != end; in += 8) {
    m = U8TO64_LE(in);
    v3 ^= m;

    TRACE;
    for (i = 0; i < cROUNDS; ++i) SIPROUND;

    v0 ^= m;
  }

  switch (left) {
  case 7: b |= ( ( uint64_t )in[ 6] )  << 48;  //NOLINT
  case 6: b |= ( ( uint64_t )in[ 5] )  << 40;  //NOLINT
  case 5: b |= ( ( uint64_t )in[ 4] )  << 32;  //NOLINT
  case 4: b |= ( ( uint64_t )in[ 3] )  << 24;  //NOLINT
  case 3: b |= ( ( uint64_t )in[ 2] )  << 16;  //NOLINT
  case 2: b |= ( ( uint64_t )in[ 1] )  <<  8;  //NOLINT
  case 1: b |= ( ( uint64_t )in[ 0] ); break;  //NOLINT
  case 0: break;
  }


  v3 ^= b;

  TRACE;
  for (i = 0; i < cROUNDS; ++i) SIPROUND;

  v0 ^= b;

#ifndef DOUBLE
  v2 ^= 0xff;
#else
  v2 ^= 0xee;
#endif

  TRACE;
  for (i = 0; i < dROUNDS; ++i) SIPROUND;

  b = v0 ^ v1 ^ v2  ^ v3;
  U64TO8_LE(out, b);

#ifdef DOUBLE
  v1 ^= 0xdd;

  TRACE;
  for (i = 0; i < dROUNDS; ++i) SIPROUND;

  b = v0 ^ v1 ^ v2  ^ v3;
  U64TO8_LE(out + 8, b);
#endif

  return 0;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

std::array<byte, 16> GetRandomSeed() {
  std::array<byte, 16> seed{{}};
  CryptoPP::RandomNumberGenerator& random = maidsafe::crypto::random_number_generator();
  random.GenerateBlock(seed.data(), seed.size());
  return seed;
}

std::uint64_t SiphashReference(
    const std::array<byte, 16>& seed,
    const char* in,
    const std::uint64_t inlen) {
  std::uint64_t out{};
  siphash(
      reinterpret_cast<byte*>(&out),
      reinterpret_cast<const byte*>(in),
      inlen,
      seed.data());
  return out;
}

}  // namespace

TEST(SipHash, BEH_FixedString) {
  const char test_string[] = "hash this string";
  const unsigned string_size = sizeof(test_string) - 1;
  const auto random_seed(GetRandomSeed());
  const auto reference_hash(SiphashReference(random_seed, test_string, string_size));

  // Split the string at every possible point
  for (unsigned count = 0; count <= string_size; ++count) {
    maidsafe::SipHash hash(random_seed);
    hash.Update(reinterpret_cast<const byte*>(test_string), count);
    hash.Update(
        reinterpret_cast<const byte*>(test_string) + count, string_size - count);
    EXPECT_EQ(reference_hash, hash.Finalize()) << "Failed on count " << count;
  }

  // Try byte at a time
  {
    maidsafe::SipHash hash(random_seed);
    for (unsigned count = 0; count < string_size; ++count) {
      hash.Update(reinterpret_cast<const byte*>(&test_string[count]), 1);
    }
    EXPECT_EQ(reference_hash, hash.Finalize());
  }
}

TEST(SipHash, BEH_RandomString) {
  const std::string test_string(RandomString(1000));
  const auto random_seed(GetRandomSeed());
  const auto reference_hash(
      SiphashReference(random_seed, test_string.data(), test_string.size()));

  // Split the string at every possible point
  for (unsigned count = 0; count <= test_string.size(); ++count) {
    maidsafe::SipHash hash(random_seed);
    hash.Update(reinterpret_cast<const byte*>(test_string.data()), count);
    hash.Update(
        reinterpret_cast<const byte*>(test_string.data()) + count,
        test_string.size() - count);
    EXPECT_EQ(reference_hash, hash.Finalize()) << "Failed on count " << count;
  }

  // Try byte at a time
  {
    maidsafe::SipHash hash(random_seed);
    for (unsigned count = 0; count < test_string.size(); ++count) {
      hash.Update(reinterpret_cast<const byte*>(test_string.data() + count), 1);
    }
    EXPECT_EQ(reference_hash, hash.Finalize());
  }
}

}  // namespace test
}  // namespace maidsafe
