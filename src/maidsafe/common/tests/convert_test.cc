/*  Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/test.h"
#include "maidsafe/common/convert.h"

namespace maidsafe {

namespace test {

template <class T>
void ToBoostThenBack(const T& value) {
  auto boost_value = common::convert::ToBoost(value);
  auto second_value = common::convert::ToAsio(boost_value);
  EXPECT_EQ(value, second_value);
}

template <class T>
void ToAsioThenBack(const T& value) {
  auto boost_value = common::convert::ToAsio(value);
  auto second_value = common::convert::ToBoost(boost_value);
  EXPECT_EQ(value, second_value);
}

TEST(Conversions, BEH_Address_v4) {
  auto ip = GetRandomIPv4AddressAsString();
  ToBoostThenBack(asio::ip::address_v4::from_string(ip));
  ToAsioThenBack(boost::asio::ip::address_v4::from_string(ip));
}

TEST(Conversions, BEH_Address_v6) {
  auto ip = GetRandomIPv6AddressAsString();
  ToBoostThenBack(asio::ip::address_v6::from_string(ip));
  ToAsioThenBack(boost::asio::ip::address_v6::from_string(ip));
}

TEST(Conversions, BEH_Address) {
  auto ip_v4 = GetRandomIPv4AddressAsString();
  ToBoostThenBack(asio::ip::address::from_string(ip_v4));
  ToAsioThenBack(boost::asio::ip::address::from_string(ip_v4));
  auto ip_v6 = GetRandomIPv6AddressAsString();
  ToBoostThenBack(asio::ip::address::from_string(ip_v6));
  ToAsioThenBack(boost::asio::ip::address::from_string(ip_v6));
}

TEST(Conversions, BEH_Endpoint) {
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

}  // namespace test

}  // namespace maidsafe

