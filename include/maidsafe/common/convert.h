/*  Copyright 2012 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_CONVERT_H_
#define MAIDSAFE_COMMON_CONVERT_H_

#include "asio/ip/address.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/address_v6.hpp"
#include "asio/ip/udp.hpp"

#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/address_v4.hpp"
#include "boost/asio/ip/address_v6.hpp"
#include "boost/asio/ip/udp.hpp"

namespace maidsafe { namespace common { namespace convert {

// Warning: Will map error from any error_category to an error
// in the generic_category. Even boost asio category maps to
// the std generic category so there probably isn't a bijection
// between the std::error_code and the boost::system::error_code.
inline std::error_code ToStd(const boost::system::error_code& ec) {
  using errc_t = std::errc;
  return std::make_error_code(static_cast<errc_t>(ec.value()));
}

inline boost::asio::ip::address_v4 ToBoost(const asio::ip::address_v4& addr) {
  return boost::asio::ip::address_v4(addr.to_ulong());
}

inline asio::ip::address_v4 ToAsio(const boost::asio::ip::address_v4& addr) {
  return asio::ip::address_v4(addr.to_ulong());
}

inline boost::asio::ip::address_v6 ToBoost(const asio::ip::address_v6& addr) {
  return boost::asio::ip::address_v6(addr.to_bytes(), addr.scope_id());
}

inline asio::ip::address_v6 ToAsio(const boost::asio::ip::address_v6& addr) {
  return asio::ip::address_v6(addr.to_bytes(), addr.scope_id());
}

inline boost::asio::ip::address ToBoost(const asio::ip::address& addr) {
  if (addr.is_v4()) {
    return boost::asio::ip::address(ToBoost(addr.to_v4()));
  } else if (addr.is_v6()) {
    return boost::asio::ip::address(ToBoost(addr.to_v6()));
  } else {
    assert(0 && "Unknown IP version");
    return boost::asio::ip::address();
  }
}

inline asio::ip::address ToAsio(const boost::asio::ip::address& addr) {
  if (addr.is_v4()) {
    return asio::ip::address(ToAsio(addr.to_v4()));
  } else if (addr.is_v6()) {
    return asio::ip::address(ToAsio(addr.to_v6()));
  } else {
    assert(0 && "Unknown IP version");
    return asio::ip::address();
  }
}

inline boost::asio::ip::udp::endpoint ToBoost(const asio::ip::udp::endpoint& e) {
  return boost::asio::ip::udp::endpoint(ToBoost(e.address()), e.port());
}

inline asio::ip::udp::endpoint ToAsio(const boost::asio::ip::udp::endpoint& e) {
  return asio::ip::udp::endpoint(ToAsio(e.address()), e.port());
}

}  // namespace convert
}  // namespace common
}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CONVERT_H_
