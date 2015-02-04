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

/*

This serialises an IPv4 address to 5 bytes and an IPv6 address to 17 bytes.  It may be more
efficient depending on the ratio of IPv4 to IPv6 addresses being handled to always serialise using
IPv6, in which case the serialised data can be 16 bytes.

To do this, the following replacements for detail::Save and detail::Load can be used:

template <typename Archive, typename Address>
void Save(Archive& archive, const Address& address) {
  using AddressV6 = typename AddressTypes<Address>::V6;
  AddressV6 ip_v6_address;
  if (address.is_v4())
    ip_v6_address = AddressV6::v4_mapped(address.to_v4());
  else
    ip_v6_address = address.to_v6();
  typename AddressV6::bytes_type bytes = ip_v6_address.to_bytes();
  archive(cereal::binary_data(bytes.data(), sizeof(bytes)));
}

template <typename Archive, typename Address>
void Load(Archive& archive, Address& address) {
  using AddressV6 = typename AddressTypes<Address>::V6;
  typename AddressV6::bytes_type bytes;
  archive(cereal::binary_data(bytes.data(), sizeof(bytes)));
  AddressV6 ip_v6_address(bytes);
  if (ip_v6_address.is_v4_mapped())
    address = ip_v6_address.to_v4();
  else
    address = ip_v6_address;
}

*/


#ifndef MAIDSAFE_COMMON_SERIALISATION_TYPES_ASIO_AND_BOOST_ASIO_H_
#define MAIDSAFE_COMMON_SERIALISATION_TYPES_ASIO_AND_BOOST_ASIO_H_

#include "asio/ip/address.hpp"
#include "boost/asio/ip/address.hpp"
#include "cereal/cereal.hpp"

#include "maidsafe/common/error.h"

namespace maidsafe {

namespace detail {

template <typename T>
struct AddressTypes;

template <>
struct AddressTypes<asio::ip::address> {
  using V4 = asio::ip::address_v4;
  using V6 = asio::ip::address_v6;
};

template <>
struct AddressTypes<boost::asio::ip::address> {
  using V4 = boost::asio::ip::address_v4;
  using V6 = boost::asio::ip::address_v6;
};

template <typename Archive, typename AddressV4orV6>
void SaveV4orV6Address(Archive& archive, const AddressV4orV6& address) {
  typename AddressV4orV6::bytes_type bytes(address.to_bytes());
  archive(cereal::make_size_tag(static_cast<uint8_t>(sizeof(bytes))));
  archive(cereal::binary_data(bytes.data(), sizeof(bytes)));
}

template <typename Archive, typename AddressV4orV6>
void LoadV4orV6Address(Archive& archive, AddressV4orV6& address) {
  typename AddressV4orV6::bytes_type bytes;
  archive(cereal::binary_data(bytes.data(), sizeof(bytes)));
  address = AddressV4orV6(bytes);
}

template <typename Archive, typename Address>
void Save(Archive& archive, const Address& address) {
  if (address.is_v4())
    SaveV4orV6Address(archive, address.to_v4());
  else
    SaveV4orV6Address(archive, address.to_v6());
}

template <typename Archive, typename Address>
void Load(Archive& archive, Address& address) {
  using AddressV4 = typename AddressTypes<Address>::V4;
  using AddressV6 = typename AddressTypes<Address>::V6;

  uint8_t size;
  archive(cereal::make_size_tag(size));
  if (size == std::tuple_size<typename AddressV4::bytes_type>::value) {
    AddressV4 address_v4;
    LoadV4orV6Address(archive, address_v4);
    address = address_v4;
  } else if (size == std::tuple_size<typename AddressV6::bytes_type>::value) {
    AddressV6 address_v6;
    LoadV4orV6Address(archive, address_v6);
    address = address_v6;
  } else {
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
}

}  // namespace detail

}  // namespace maidsafe

namespace cereal {

template <typename Archive>
void save(Archive& archive, const asio::ip::address& address) {
  maidsafe::detail::Save<Archive, asio::ip::address>(archive, address);
}

template <typename Archive>
void load(Archive& archive, asio::ip::address& address) {
  maidsafe::detail::Load<Archive, asio::ip::address>(archive, address);
}

template <typename Archive>
void save(Archive& archive, const boost::asio::ip::address& address) {
  maidsafe::detail::Save<Archive, boost::asio::ip::address>(archive, address);
}

template <typename Archive>
void load(Archive& archive, boost::asio::ip::address& address) {
  maidsafe::detail::Load<Archive, boost::asio::ip::address>(archive, address);
}

}  // namespace cereal

#endif  // MAIDSAFE_COMMON_SERIALISATION_TYPES_ASIO_AND_BOOST_ASIO_H_
