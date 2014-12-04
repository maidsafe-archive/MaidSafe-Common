/*  Copyright 2008 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_NODE_ID_H_
#define MAIDSAFE_COMMON_NODE_ID_H_

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/error.h"

namespace maidsafe {

class NodeId {
 public:
  enum class EncodingType { kBinary = 0, kHex, kBase64 };
  static const size_t kSize = crypto::SHA512::DIGESTSIZE;

  // Creates an ID for which IsValid() returns false.
  NodeId();

  NodeId(const NodeId& other);
  NodeId(NodeId&& other) MAIDSAFE_NOEXCEPT;
  NodeId& operator=(const NodeId& other) = default;
  NodeId& operator=(NodeId&& other) MAIDSAFE_NOEXCEPT;

  // Creates a NodeId from a raw (decoded) string.  Will throw if 'id' is invalid.
  explicit NodeId(std::string id);

  // Creates a NodeId from a SHA512 hash.
  explicit NodeId(const crypto::SHA512Hash& id);

  // Creates a NodeId from an encoded string.  Will throw if 'id' is invalid.
  NodeId(const std::string& id, EncodingType encoding_type);

  // Checks if 'id1' is closer in XOR distance to 'target_id' than 'id2'.  Will throw if IsValid()
  // is false for any of the args.
  static bool CloserToTarget(const NodeId& id1, const NodeId& id2, const NodeId& target_id);

  // Decoded representation of the ID.  Will throw if IsValid() is false.
  std::string string() const;

  // Encoded representation of the ID.  Will throw if IsValid() is false.
  std::string ToStringEncoded(const EncodingType& encoding_type) const;

  // False for default constructed NodeId, otherwise true.
  bool IsValid() const;

  // DEPRECATED - Use 'IsValid' instead.  True for default constructed NodeId, otherwise false.
  bool IsZero() const { return !IsValid(); }

  // Number of most significant bits which are common to this ID and 'other'.  Will throw if
  // IsValid() is false for '*this' or 'other'.
  int CommonLeadingBits(const NodeId& other) const;

  // Bitwise XOR distance between two IDs.  Will throw if IsValid() is false for '*this' or 'other'.
  NodeId& operator^=(const NodeId& other);

  // Required by Cereal
  template <typename Archive>
  Archive& load(Archive& ref_archive) {
    std::string temp;
    auto& archive = ref_archive(temp);
    *this = NodeId{std::move(temp)};
    return archive;
  }

  // Required by Cereal
  template <typename Archive>
  Archive& save(Archive& ref_archive) const {
    return ref_archive(raw_id_);
  }

  // Returns true if both 'raw_id_'s are equal or if IsValid() is false for both.  Doesn't throw.
  friend bool operator==(const NodeId& lhs, const NodeId& rhs);

  // Returns true if 'lhs.raw_id_' is less than 'rhs.raw_id_' or if IsValid() is false for 'lhs' and
  // true for 'rhs'.  Doesn't throw.
  friend bool operator<(const NodeId& lhs, const NodeId& rhs);

  // Returns an abbreviated hex representation of node_id.
  friend std::string DebugId(const NodeId& node_id);

  friend void swap(NodeId& lhs, NodeId& rhs) MAIDSAFE_NOEXCEPT;

 private:
  std::string EncodeToBinary() const;
  void DecodeFromBinary(const std::string& binary_id);

  std::string raw_id_;
#ifndef NDEBUG
  std::string debug_id_;
#endif
};

inline bool operator==(const NodeId& lhs, const NodeId& rhs) { return lhs.raw_id_ == rhs.raw_id_; }
inline bool operator!=(const NodeId& lhs, const NodeId& rhs) { return !operator==(lhs, rhs); }
inline bool operator<(const NodeId& lhs, const NodeId& rhs) { return lhs.raw_id_ < rhs.raw_id_; }
inline bool operator>(const NodeId& lhs, const NodeId& rhs) { return operator<(rhs, lhs); }
inline bool operator<=(const NodeId& lhs, const NodeId& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const NodeId& lhs, const NodeId& rhs) { return !operator<(lhs, rhs); }

// Bitwise XOR distance between two IDs.  Will throw if IsValid() is false for either arg.
inline NodeId operator^(NodeId lhs, const NodeId& rhs) {
  lhs ^= rhs;
  return lhs;
}

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,
                                             const NodeId& node_id) {
  ostream << DebugId(node_id);
  return ostream;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_NODE_ID_H_
