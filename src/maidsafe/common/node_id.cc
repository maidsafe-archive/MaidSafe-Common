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

#include "maidsafe/common/node_id.h"

#include <algorithm>
#include <bitset>

#include "maidsafe/common/error_categories.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/node_id_common_bits_matrix.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const size_t NodeId::kSize;
#endif

#ifdef NDEBUG
#define INIT_DEBUG_NODE_ID
#else
#define INIT_DEBUG_NODE_ID , debug_id_(HexSubstr(raw_id_))
#endif

#ifdef NDEBUG
NodeId::NodeId() : raw_id_() {}
#else
NodeId::NodeId() : raw_id_(), debug_id_("Invalid ID") {}
#endif

NodeId::NodeId(const NodeId& other) : raw_id_(other.raw_id_) INIT_DEBUG_NODE_ID {}

NodeId::NodeId(NodeId&& other) MAIDSAFE_NOEXCEPT
    : raw_id_(std::move(other.raw_id_)) INIT_DEBUG_NODE_ID {}

NodeId& NodeId::operator=(NodeId&& other) MAIDSAFE_NOEXCEPT {
  raw_id_ = std::move(other.raw_id_);
#ifndef NDEBUG
  debug_id_ = std::move(other.debug_id_);
#endif
  return *this;
}

NodeId::NodeId(std::string id) : raw_id_(std::move(id)) INIT_DEBUG_NODE_ID {
  if (raw_id_.size() != kSize)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
}

NodeId::NodeId(const crypto::SHA512Hash& id) : raw_id_(id.string()) INIT_DEBUG_NODE_ID {}

NodeId::NodeId(const std::string& id, NodeId::EncodingType encoding_type) : raw_id_() {
  try {
    switch (encoding_type) {
      case EncodingType::kBinary:
        DecodeFromBinary(id);
        break;
      case EncodingType::kHex:
        raw_id_ = HexDecode(id);
        break;
      case EncodingType::kBase64:
        raw_id_ = Base64Decode(id);
        break;
      default:
        raw_id_ = id;
    }
  } catch (const std::exception& e) {
    LOG(kError) << "NodeId Ctor: " << boost::diagnostic_information(e);
    raw_id_.clear();
  }
  if (raw_id_.size() != kSize)
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
#ifndef NDEBUG
  debug_id_ = HexSubstr(raw_id_);
#endif
}

std::string NodeId::EncodeToBinary() const {
  std::string binary;
  binary.reserve(kSize);
  for (size_t i = 0; i < kSize; ++i) {
    std::bitset<8> temp(static_cast<int>(raw_id_[i]));
    binary += temp.to_string();
  }
  return binary;
}

void NodeId::DecodeFromBinary(const std::string& binary_id) {
  raw_id_.assign(kSize, 0);
  for (size_t i = 0; i < kSize; ++i) {
    std::bitset<8> temp(binary_id.substr(i * 8, 8));
    raw_id_[i] = static_cast<char>(temp.to_ulong());
  }
}

bool NodeId::CloserToTarget(const NodeId& id1, const NodeId& id2, const NodeId& target_id) {
  if (!id1.IsValid() || !id2.IsValid() || !target_id.IsValid())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
  for (uint16_t i = 0; i < kSize; ++i) {
    unsigned char result1 = id1.raw_id_[i] ^ target_id.raw_id_[i];
    unsigned char result2 = id2.raw_id_[i] ^ target_id.raw_id_[i];
    if (result1 != result2)
      return result1 < result2;
  }
  return false;
}

std::string NodeId::string() const {
  if (!IsValid())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
  return raw_id_;
}

std::string NodeId::ToStringEncoded(const EncodingType& encoding_type) const {
  if (!IsValid())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
  switch (encoding_type) {
    case EncodingType::kBinary:
      return EncodeToBinary();
    case EncodingType::kHex:
      return HexEncode(raw_id_);
    case EncodingType::kBase64:
      return Base64Encode(raw_id_);
    default:
      return raw_id_;
  }
}

bool NodeId::IsValid() const { return !raw_id_.empty(); }

int NodeId::CommonLeadingBits(const NodeId& other) const {
  if (!IsValid() || !other.IsValid())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));

  // Find first mismatching char between the two IDs
  auto mismatch(std::mismatch(std::begin(raw_id_), std::end(raw_id_), std::begin(other.raw_id_)));

  // If there's no mismatch, the IDs are equal
  if (mismatch.first == std::end(raw_id_))
    return 8 * kSize;

  int common_bits{detail::kCommonBits[static_cast<unsigned char>(
      *mismatch.first)][static_cast<unsigned char>(*mismatch.second)]};
  return static_cast<int>(8 * std::distance(std::begin(raw_id_), mismatch.first)) + common_bits;
}

NodeId& NodeId::operator^=(const NodeId& other) {
  if (!IsValid() || !other.IsValid())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));

  for (uint16_t i(0); i != kSize; ++i)
    raw_id_[i] ^= other.raw_id_[i];
#ifndef NDEBUG
  debug_id_ = HexSubstr(raw_id_);
#endif
  return *this;
}

std::string DebugId(const NodeId& node_id) {
#ifdef NDEBUG
  return node_id.IsValid() ? HexSubstr(node_id.raw_id_) : "Invalid ID";
#else
  return node_id.debug_id_;
#endif
}

void swap(NodeId& lhs, NodeId& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.raw_id_, rhs.raw_id_);
#ifndef NDEBUG
  swap(lhs.debug_id_, rhs.debug_id_);
#endif
}

}  // namespace maidsafe
