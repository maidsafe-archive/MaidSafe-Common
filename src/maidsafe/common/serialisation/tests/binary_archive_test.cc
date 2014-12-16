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

#include "maidsafe/common/serialisation/binary_archive.h"

#include <limits>
#include <type_traits>

#include "cereal/types/string.hpp"

#include "maidsafe/common/serialisation/compile_time_mapper.h"
#include "maidsafe/common/serialisation/serialisation.h"
#include "maidsafe/common/test.h"

namespace maidsafe {

namespace test {

enum class MessageTypeTag : SerialisableTypeTag { kPing, kPingResponse };

struct Ping {
  static const SerialisableTypeTag kSerialisableTypeTag =
      static_cast<SerialisableTypeTag>(MessageTypeTag::kPing);
  std::string data = "Ping";
  template <typename Archive>
  void serialize(Archive& archive) {
    archive(data);
  }
};

const SerialisableTypeTag Ping::kSerialisableTypeTag;

struct PingResponse {
  static const SerialisableTypeTag kSerialisableTypeTag =
      static_cast<SerialisableTypeTag>(MessageTypeTag::kPingResponse);
  std::string data = "PingResponse";
  template <typename Archive>
  void serialize(Archive& archive) {
    archive(data);
  }
};

const SerialisableTypeTag PingResponse::kSerialisableTypeTag;

using MessageMap = GetMap<Ping, PingResponse>::Map;

template <SerialisableTypeTag Tag>
using Message = typename Find<MessageMap, Tag>::ResultCustomType;

TEST(BinaryArchiveTest, BEH_Basic) {
  // std::vector<byte> functions
  auto serialised_message = SerialiseMappedType(Ping());

  InputVectorStream binary_stream{serialised_message};
  auto tag = static_cast<MessageTypeTag>(TypeFromStream(binary_stream));
  ASSERT_EQ(MessageTypeTag::kPing, tag);
  auto parsed_ping = Parse<Ping>(binary_stream);
  EXPECT_EQ("Ping", parsed_ping.data);

  serialised_message = SerialiseMappedType(PingResponse());
  binary_stream.swap_vector(serialised_message);
  tag = static_cast<MessageTypeTag>(TypeFromStream(binary_stream));
  ASSERT_EQ(MessageTypeTag::kPingResponse, tag);
  auto parsed_ping_response = Parse<PingResponse>(binary_stream);
  EXPECT_EQ("PingResponse", parsed_ping_response.data);

  // std::string functions
  auto serialised_to_string_message = SerialiseMappedTypeToString(Ping());

  std::istringstream binary_string_stream{serialised_to_string_message};
  tag = static_cast<MessageTypeTag>(TypeFromStringStream(binary_string_stream));
  ASSERT_EQ(MessageTypeTag::kPing, tag);
  parsed_ping = ParseFromStringStream<Ping>(binary_string_stream);
  EXPECT_EQ("Ping", parsed_ping.data);

  serialised_to_string_message = SerialiseMappedTypeToString(PingResponse());
  binary_string_stream.str(serialised_to_string_message);
  tag = static_cast<MessageTypeTag>(TypeFromStringStream(binary_string_stream));
  ASSERT_EQ(MessageTypeTag::kPingResponse, tag);
  parsed_ping_response = ParseFromStringStream<PingResponse>(binary_string_stream);
  EXPECT_EQ("PingResponse", parsed_ping_response.data);
}

}  // namespace test

}  // namespace maidsafe
