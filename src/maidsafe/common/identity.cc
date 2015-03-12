/*  Copyright 2015 MaidSafe.net limited

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

#include "maidsafe/common/identity.h"

#include <algorithm>
#include <bitset>

#include "maidsafe/common/convert.h"
#include "maidsafe/common/error_categories.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/identity_common_bits_matrix.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

bool CloserToTarget(const Identity& id1, const Identity& id2, const Identity& target_id) {
  if (!id1.IsInitialised() || !id2.IsInitialised() || !target_id.IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));
  for (uint16_t i = 0; i < identity_size; ++i) {
    unsigned char result1 = id1.string()[i] ^ target_id.string()[i];
    unsigned char result2 = id2.string()[i] ^ target_id.string()[i];
    if (result1 != result2)
      return result1 < result2;
  }
  return false;
}

int CommonLeadingBits(const Identity& id1, const Identity& id2) {
  if (!id1.IsInitialised() || !id2.IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));

  // Find first mismatching char between the two IDs
  const std::vector<unsigned char>& raw_id1(id1.string());
  const std::vector<unsigned char>& raw_id2(id2.string());
  auto mismatch(std::mismatch(raw_id1.begin(), raw_id1.end(), raw_id2.begin()));

  // If there's no mismatch, the IDs are equal
  if (mismatch.first == raw_id1.end())
    return 8 * identity_size;

  int common_bits(detail::kCommonBits[*mismatch.first][*mismatch.second]);
  return static_cast<int>(8 * std::distance(raw_id1.begin(), mismatch.first)) + common_bits;
}



namespace binary {

std::string Encode(const Identity& id) {
  if (!id.IsInitialised())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));
  std::string binary;
  binary.reserve(identity_size);
  for (size_t i = 0; i < identity_size; ++i) {
    std::bitset<8> temp(static_cast<int>(id.string()[i]));
    binary += temp.to_string();
  }
  return binary;
}

}  // namespace binary



namespace hex {

std::string Encode(const Identity& id) { return Encode(id.string()); }

}  // namespace hex



namespace base64 {

std::string Encode(const Identity& id) { return Encode(id.string()); }

}  // namespace base64



Identity MakeIdentity(const binary::String& id) {
  try {
    std::vector<byte> raw_id(identity_size, 0);
    for (size_t i = 0; i < identity_size; ++i) {
      std::bitset<8> temp(id->substr(i * 8, 8));
      raw_id[i] = static_cast<unsigned char>(temp.to_ulong());
    }
    return Identity(raw_id);
  } catch (const std::exception& e) {
    LOG(kError) << "Identity factory: " << boost::diagnostic_information(e);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));
  }
}

Identity MakeIdentity(const hex::String& id) {
  try {
    return Identity(hex::DecodeToBytes(id.data));
  } catch (const std::exception& e) {
    LOG(kError) << "Identity factory: " << boost::diagnostic_information(e);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));
  }
}

Identity MakeIdentity(const base64::String& id) {
  try {
    return Identity(base64::DecodeToBytes(id.data));
  } catch (const std::exception& e) {
    LOG(kError) << "Identity factory: " << boost::diagnostic_information(e);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_identity));
  }
}

Identity MakeIdentity() { return Identity(RandomBytes(identity_size)); }

}  // namespace maidsafe
