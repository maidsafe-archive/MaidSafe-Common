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

#ifndef MAIDSAFE_COMMON_TCP_LISTENER_H_
#define MAIDSAFE_COMMON_TCP_LISTENER_H_

#include <functional>
#include <memory>
#include <mutex>

#include "asio/ip/tcp.hpp"
#include "asio/io_service.hpp"
#include "asio/strand.hpp"

#include "maidsafe/common/types.h"

namespace maidsafe {

namespace tcp {

class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(const Listener&) = delete;
  Listener(Listener&&) = delete;
  Listener& operator=(Listener) = delete;

  static ListenerPtr MakeShared(asio::io_service::strand& strand,
                                NewConnectionFunctor on_new_connection, Port desired_port);
  Port ListeningPort() const;
  void StopListening();

 private:
  Listener(asio::io_service::strand& strand, NewConnectionFunctor on_new_connection);

  void StartListening(Port desired_port);
  void DoStartListening(Port port);
  void HandleAccept(ConnectionPtr accepted_connection, const std::error_code& ec);
  void DoStopListening();

  asio::io_service::strand& strand_;
  std::once_flag stop_listening_flag_;
  NewConnectionFunctor on_new_connection_;
  asio::ip::tcp::acceptor acceptor_;
};

}  // namespace tcp

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TCP_LISTENER_H_
