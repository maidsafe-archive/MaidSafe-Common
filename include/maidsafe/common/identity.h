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

#ifndef MAIDSAFE_COMMON_IDENTITY_H_
#define MAIDSAFE_COMMON_IDENTITY_H_

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/tagged_value.h"

namespace maidsafe {

const std::size_t identity_size = 64;

using Identity = detail::BoundedString<identity_size, identity_size>;

// Checks if 'id1' is closer in XOR distance to 'target_id' than 'id2'.  Will throw if
// IsInitialised() is false for any of the args.
bool CloserToTarget(const Identity& id1, const Identity& id2, const Identity& target_id);

// Number of most significant bits which are common to 'id1' and 'id2'.  Will throw if
// IsInitialised() is false for either of the args.
int CommonLeadingBits(const Identity& id1, const Identity& id2);



namespace binary {

using String = TaggedValue<std::string, struct BinaryTag>;

// Encoded representation of the ID.  Will throw if id.IsInitialised() is false.
std::string Encode(const Identity& id);

}  // namespace binary



namespace hex {

using String = TaggedValue<std::string, struct BinaryTag>;

// Encoded representation of the ID.  Will throw if id.IsInitialised() is false.
std::string Encode(const Identity& id);

}  // namespace hex



namespace base64 {

using String = TaggedValue<std::string, struct BinaryTag>;

// Encoded representation of the ID.  Will throw if id.IsInitialised() is false.
std::string Encode(const Identity& id);

}  // namespace base64



// Creates an Identity from an encoded string.  Will throw if 'id.size()' is not 'identity_size'.
Identity MakeIdentity(const binary::String& id);
Identity MakeIdentity(const hex::String& id);
Identity MakeIdentity(const base64::String& id);

// Creates a random Identity (mainly useful for testing).
Identity MakeIdentity();

// Returns an abbreviated hex representation of id.  Throws if IsInitialised() is false for 'id'.
std::string DebugId(const Identity& id);

}  // namespace maidsafe

// Prints DebugId of 'id'.
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,
                                             const maidsafe::Identity& id) {
  ostream << maidsafe::DebugId(id);
  return ostream;
}

#endif  // MAIDSAFE_COMMON_NODE_ID_H_
