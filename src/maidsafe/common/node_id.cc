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

#include "maidsafe/common/node_id.h"

#include <bitset>

#include "maidsafe/common/error_categories.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

NodeId::NodeId() : raw_id_(kSize, 0) {}

NodeId::NodeId(const NodeId& other) : raw_id_(other.raw_id_) {}

NodeId::NodeId(const IdType& type) : raw_id_(kSize, -1) {
  switch (type) {
    case kMaxId :
      break;  // already set
    case kRandomId :
      std::generate(raw_id_.begin(), raw_id_.end(), std::bind(&RandomUint32));
      break;
    default :
      break;
  }
}

NodeId::NodeId(const std::string& id) : raw_id_(id) {
  if (raw_id_.size() != kSize) {
    raw_id_.clear();
    ThrowError(CommonErrors::invalid_node_id);
  }
}

NodeId::NodeId(const crypto::SHA512Hash& id) : raw_id_(id.string()) {}

NodeId::NodeId(const std::string& id, const EncodingType& encoding_type) : raw_id_() {
  try {
    switch (encoding_type) {
      case kBinary: DecodeFromBinary(id);
        break;
      case kHex: raw_id_ = DecodeFromHex(id);
        break;
      case kBase32: raw_id_ = DecodeFromBase32(id);
        break;
      case kBase64: raw_id_ = DecodeFromBase64(id);
        break;
      default: raw_id_ = id;
    }
  }
  catch(const std::exception& e) {
    LOG(kError) << "NodeId Ctor: " << e.what();
    raw_id_.clear();
  }
  if (raw_id_.size() != kSize) {
    raw_id_.clear();
    ThrowError(CommonErrors::invalid_node_id);
  }
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
  std::bitset<8 * kSize> binary_bitset(binary_id);
  if (raw_id_.size() != kSize)
    raw_id_.assign(kSize, 0);
  for (size_t i = 0; i < kSize; ++i) {
    std::bitset<8> temp(binary_id.substr(i * 8, 8));
    raw_id_[i] = static_cast<char>(temp.to_ulong());
  }
}

bool NodeId::CloserToTarget(const NodeId& id1, const NodeId& id2, const NodeId& target_id) {
  for (uint16_t i = 0; i < kSize; ++i) {
    unsigned char result1 = id1.raw_id_[i] ^ target_id.raw_id_[i];
    unsigned char result2 = id2.raw_id_[i] ^ target_id.raw_id_[i];
    if (result1 != result2)
      return result1 < result2;
  }
  return false;
}

const std::string NodeId::string() const {
  return raw_id_;
}

const std::string NodeId::ToStringEncoded(const EncodingType& encoding_type) const {
  switch (encoding_type) {
    case kBinary:
      return EncodeToBinary();
    case kHex:
      return EncodeToHex(raw_id_);
    case kBase32:
      return EncodeToBase32(raw_id_);
    case kBase64:
      return EncodeToBase64(raw_id_);
    default:
      return raw_id_;
  }
}

bool NodeId::IsZero() const {
  static const std::string kZeroId(kSize, 0);
  return raw_id_ == kZeroId;
}

// TODO(Fraser#5#): 2012-09-28 - Check if required - probably used before in multi-index.
// bool NodeId::operator()(const NodeId& lhs, const NodeId& rhs) const {
//   return lhs.raw_id_ < rhs.raw_id_;
// }

bool NodeId::operator==(const NodeId& rhs) const {
  return raw_id_ == rhs.raw_id_;
}

bool NodeId::operator!=(const NodeId& rhs) const {
  return raw_id_ != rhs.raw_id_;
}

bool NodeId::operator<(const NodeId& rhs) const {
  return raw_id_ < rhs.raw_id_;
}

bool NodeId::operator>(const NodeId& rhs) const {
  return raw_id_ > rhs.raw_id_;
}

bool NodeId::operator<=(const NodeId& rhs) const {
  return raw_id_ <= rhs.raw_id_;
}

bool NodeId::operator>=(const NodeId& rhs) const {
  return raw_id_ >= rhs.raw_id_;
}

NodeId& NodeId::operator=(const NodeId& rhs) {
  if (this != &rhs)
    this->raw_id_ = rhs.raw_id_;
  return* this;
}

const NodeId NodeId::operator^(const NodeId& rhs) const {
  NodeId result;
  for (uint16_t i(0); i != kSize; ++i)
    result.raw_id_[i] = raw_id_[i] ^ rhs.raw_id_[i];
  return result;
}

std::string DebugId(const NodeId& node_id) {
  std::string hex(node_id.ToStringEncoded(NodeId::kHex));
  return hex.substr(0, 7) + ".." + hex.substr(hex.size() - 7);
}

}  // namespace maidsafe
