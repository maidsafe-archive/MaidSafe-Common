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

#include "maidsafe/common/convert.h"

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

template <class T>
void ToBoostThenBack(const T& value) {
  auto boost_value = convert::ToBoost(value);
  auto second_value = convert::ToAsio(boost_value);
  EXPECT_EQ(value, second_value);
}

template <class T>
void ToAsioThenBack(const T& value) {
  auto boost_value = convert::ToAsio(value);
  auto second_value = convert::ToBoost(boost_value);
  EXPECT_EQ(value, second_value);
}

TEST(ConversionsTest, BEH_Address_v4) {
  auto ip = GetRandomIPv4AddressAsString();
  ToBoostThenBack(asio::ip::address_v4::from_string(ip));
  ToAsioThenBack(boost::asio::ip::address_v4::from_string(ip));
}

TEST(ConversionsTest, BEH_Address_v6) {
  const unsigned long scope_ids[] = {0x0, 0x1, 0x2, 0x4, 0x5, 0x8, 0xe, 0xf};  // NOLINT

  for (auto scope_id : scope_ids) {
    auto ip = GetRandomIPv6AddressAsString();

    auto asio_addr = asio::ip::address_v6::from_string(ip);
    asio_addr.scope_id(scope_id);
    ToBoostThenBack(asio_addr);

    auto boost_addr = boost::asio::ip::address_v6::from_string(ip);
    boost_addr.scope_id(scope_id);
    ToAsioThenBack(boost_addr);
  }
}

TEST(ConversionsTest, BEH_Address) {
  auto ip_v4 = GetRandomIPv4AddressAsString();
  ToBoostThenBack(asio::ip::address::from_string(ip_v4));
  ToAsioThenBack(boost::asio::ip::address::from_string(ip_v4));

  auto ip_v6 = GetRandomIPv6AddressAsString();
  ToBoostThenBack(asio::ip::address::from_string(ip_v6));
  ToAsioThenBack(boost::asio::ip::address::from_string(ip_v6));
}

TEST(ConversionsTest, BEH_Endpoint) {
  using asio_endpoint = asio::ip::udp::endpoint;
  using boost_endpoint = boost::asio::ip::udp::endpoint;
  auto port = GetRandomPort();

  auto ip_v4 = GetRandomIPv4AddressAsString();
  ToBoostThenBack(asio_endpoint(asio::ip::address::from_string(ip_v4), port));
  ToAsioThenBack(boost_endpoint(boost::asio::ip::address::from_string(ip_v4), port));

  auto ip_v6 = GetRandomIPv6AddressAsString();
  ToBoostThenBack(asio_endpoint(asio::ip::address::from_string(ip_v6), port));
  ToAsioThenBack(boost_endpoint(boost::asio::ip::address::from_string(ip_v6), port));
}

TEST(ConversionsTest, BEH_ByteVectorAndString) {
  const std::string input(RandomString(RandomUint32() % 1000));

  const std::vector<unsigned char> bytes(convert::ToByteVector(input));
  ASSERT_EQ(input.size(), bytes.size());
  for (std::size_t i(0); i < input.size(); ++i)
    EXPECT_EQ(input[i], static_cast<char>(bytes[i]));

  const std::string string(convert::ToString(bytes));
  EXPECT_EQ(input, string);
}

}  // namespace test

}  // namespace maidsafe
