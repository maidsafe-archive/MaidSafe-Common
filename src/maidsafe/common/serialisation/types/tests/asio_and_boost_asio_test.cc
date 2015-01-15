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

#include "maidsafe/common/serialisation/types/asio_and_boost_asio.h"

#include "maidsafe/common/test.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace maidsafe {

namespace test {

template <typename T>
class IpAddressSerialisationTest : public testing::Test {
 protected:
  typename maidsafe::detail::AddressTypes<T>::V4 GetRandomIPv4Address();
  typename maidsafe::detail::AddressTypes<T>::V6 GetRandomIPv6Address();
};

template <>
asio::ip::address_v4 IpAddressSerialisationTest<asio::ip::address>::GetRandomIPv4Address() {
  return asio::ip::make_address_v4(GetRandomIPv4AddressAsString());
}

template <>
asio::ip::address_v6 IpAddressSerialisationTest<asio::ip::address>::GetRandomIPv6Address() {
  return asio::ip::make_address_v6(GetRandomIPv6AddressAsString());
}

template <>
boost::asio::ip::address_v4
    IpAddressSerialisationTest<boost::asio::ip::address>::GetRandomIPv4Address() {
  return boost::asio::ip::address_v4::from_string(GetRandomIPv4AddressAsString());
}

template <>
boost::asio::ip::address_v6
    IpAddressSerialisationTest<boost::asio::ip::address>::GetRandomIPv6Address() {
  return boost::asio::ip::address_v6::from_string(GetRandomIPv6AddressAsString());
}

using TestTypes = testing::Types<asio::ip::address, boost::asio::ip::address>;

TYPED_TEST_CASE(IpAddressSerialisationTest, TestTypes);

TYPED_TEST(IpAddressSerialisationTest, BEH_SaveAndLoad) {
  using Address = TypeParam;
  using AddressV4 = typename maidsafe::detail::AddressTypes<Address>::V4;
  using AddressV6 = typename maidsafe::detail::AddressTypes<Address>::V6;

  Address address = this->GetRandomIPv4Address();
  auto serialised_address_v4(Serialise(address));
  auto parsed_address(Parse<Address>(serialised_address_v4));
  EXPECT_EQ(address, parsed_address);

  address = this->GetRandomIPv6Address();
  auto serialised_address_v6(Serialise(address));
  parsed_address = Parse<Address>(serialised_address_v6);
  EXPECT_EQ(address, parsed_address);

  EXPECT_LT(serialised_address_v4.size(), serialised_address_v6.size());
}

}  // namespace test

}  // namespace maidsafe
