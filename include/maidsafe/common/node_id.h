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

#ifndef MAIDSAFE_COMMON_NODE_ID_H_
#define MAIDSAFE_COMMON_NODE_ID_H_

#include <cstdint>
#include <string>
#include <vector>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"


namespace maidsafe {

class NodeId {
 public:
  enum IdType { kMaxId, kRandomId };
  enum EncodingType { kBinary, kHex, kBase32, kBase64 };
  enum { kSize = 64 };

  // Creates an ID equal to 0.
  NodeId();

  NodeId(const NodeId& other);

  // Creates an ID = (2 ^ kKeySizeBits) - 1 or a random ID in the interval [0, 2 ^ kKeySizeBits).
  explicit NodeId(const IdType& type);

  // Creates a NodeId from a raw (decoded) string.  Will throw if id is invalid.
  explicit NodeId(const std::string& id);

  // Creates a NodeId from a SHA512 hash.
  explicit NodeId(const crypto::SHA512Hash& id);

  // Creates a NodeId from an encoded string.  Will throw if id is invalid.
  NodeId(const std::string& id, const EncodingType& encoding_type);

//  // Creates a NodeId equal to 2 ^ power.
//  explicit NodeId(const uint16_t& power);

//  // Creates a random NodeId in range [lower ID, higher ID].  Prefer to pass lower ID as id1.
//  NodeId(const NodeId& id1, const NodeId& id2);

  // Checks if id1 is closer in XOR distance to target_id than id2.
  static bool CloserToTarget(const NodeId& id1, const NodeId& id2, const NodeId& target_id);

  // Decoded representation of the ID.
  const std::string string() const;

  // Encoded representation of the ID.
  const std::string ToStringEncoded(const EncodingType& encoding_type) const;

  bool IsZero() const;

//  bool operator()(const NodeId& lhs, const NodeId& rhs) const;
  bool operator==(const NodeId& rhs) const;
  bool operator!=(const NodeId& rhs) const;
  bool operator<(const NodeId& rhs) const;
  bool operator>(const NodeId& rhs) const;
  bool operator<=(const NodeId& rhs) const;
  bool operator>=(const NodeId& rhs) const;
  NodeId& operator=(const NodeId& rhs);

  // XOR distance between two IDs.  XOR bit to bit.
  const NodeId operator^(const NodeId& rhs) const;

  friend std::string DebugId(const NodeId& node_id);

 private:
  std::string EncodeToBinary() const;
  void DecodeFromBinary(const std::string& binary_id);
  std::string raw_id_;
};

// Returns an abbreviated hex representation of node_id.
std::string DebugId(const NodeId& node_id);

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_NODE_ID_H_
