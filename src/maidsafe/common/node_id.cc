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

#include <bitset>

#include "maidsafe/common/error_categories.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

NodeId::NodeId() : raw_id_(kSize, 0) {}

NodeId::NodeId(const NodeId& other) : raw_id_(other.raw_id_) {}

NodeId::NodeId(NodeId&& other) : raw_id_(std::move(other.raw_id_)) {}

NodeId& NodeId::operator=(NodeId other) {
  swap(*this, other);
  return *this;
}

NodeId::NodeId(IdType type)
    : raw_id_([type]()->std::string {
        switch (type) {
          case kMaxId:
            return std::string(kSize, -1);
          case kRandomId:
            return RandomString(kSize);
          default:
            BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
        }
      }()) {}

NodeId::NodeId(std::string id) : raw_id_(std::move(id)) {
  if (raw_id_.size() != kSize) {
    raw_id_.clear();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
  }
}

NodeId::NodeId(const crypto::SHA512Hash& id) : raw_id_(id.string()) {}

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
  }
  catch (const std::exception& e) {
    LOG(kError) << "NodeId Ctor: " << e.what();
    raw_id_.clear();
  }
  if (encoding_type == EncodingType::kBase64) {
    Base64Decode(id);
  } else if (raw_id_.size() != kSize) {
    raw_id_.clear();
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_node_id));
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

const std::string NodeId::string() const { return raw_id_; }

const std::string NodeId::ToStringEncoded(const EncodingType& encoding_type) const {
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

bool NodeId::IsZero() const {
  for (auto i : raw_id_) {
    if (i != 0)
      return false;
  }
  return true;
}

NodeId& NodeId::operator^=(const NodeId& rhs) {
  for (uint16_t i(0); i != kSize; ++i)
    raw_id_[i] ^= rhs.raw_id_[i];
  return *this;
}

std::string DebugId(const NodeId& node_id) { return HexSubstr(node_id.raw_id_); }

void swap(NodeId& lhs, NodeId& rhs) MAIDSAFE_NOEXCEPT {
  using std::swap;
  swap(lhs.raw_id_, rhs.raw_id_);
}

}  // namespace maidsafe
