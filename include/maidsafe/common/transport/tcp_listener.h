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

#ifndef MAIDSAFE_COMMON_TRANSPORT_TCP_LISTENER_H_
#define MAIDSAFE_COMMON_TRANSPORT_TCP_LISTENER_H_

#include <functional>
#include <memory>
#include <mutex>

#include "boost/asio/ip/tcp.hpp"

#include "maidsafe/common/asio_service.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace transport {

class TcpListener : public std::enable_shared_from_this<TcpListener> {
 public:
  static TcpListenerPtr MakeShared(AsioService &asio_service,
                                   NewConnectionFunctor on_new_connection, Port desired_port);
  Port ListeningPort() const;
  void StopListening();

 private:
  TcpListener(AsioService &asio_service, NewConnectionFunctor on_new_connection);

  TcpListener(const TcpListener&) = delete;
  TcpListener(TcpListener&&) = delete;
  TcpListener& operator=(TcpListener) = delete;

  void StartListening(Port desired_port);
  void DoStartListening(Port port);
  void HandleAccept(TcpConnectionPtr accepted_connection, const boost::system::error_code& ec);
  void DoStopListening();

  AsioService& asio_service_;
  std::once_flag stop_listening_flag_;
  NewConnectionFunctor on_new_connection_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace transport

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TRANSPORT_TCP_LISTENER_H_
