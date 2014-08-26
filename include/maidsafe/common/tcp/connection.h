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

#ifndef MAIDSAFE_COMMON_TCP_CONNECTION_H_
#define MAIDSAFE_COMMON_TCP_CONNECTION_H_

#include <array>
#include <cstdint>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio/buffer.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "maidsafe/common/asio_service.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace tcp {

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  typedef uint32_t DataSize;

  Connection(const Connection&) = delete;
  Connection(Connection&&) = delete;
  Connection& operator=(Connection) = delete;

  // Used when accepting an incoming connection.
  static ConnectionPtr MakeShared(AsioService &asio_service);
  // Used to attempt to connect to 'remote_port' on loopback address.
  static ConnectionPtr MakeShared(AsioService &asio_service, Port remote_port);

  void Start(MessageReceivedFunctor on_message_received,
             ConnectionClosedFunctor on_connection_closed);

  void Close();

  void Send(std::string data);

  boost::asio::ip::tcp::socket& Socket() { return socket_; }

  static size_t MaxMessageSize() { return 1024 * 1024; }  // bytes

 private:
  explicit Connection(AsioService &asio_service);
  Connection(AsioService &asio_service, Port remote_port);

  struct ReceivingMessage {
    std::array<unsigned char, 4> size_buffer;
    std::vector<unsigned char> data_buffer;
  };

  struct SendingMessage {
    std::array<unsigned char, 4> size_buffer;
    std::string data;
  };

  void DoClose();

  void ReadSize();
  void ReadData();

  void DoSend();
  SendingMessage EncodeData(std::string data) const;

  boost::asio::io_service& io_service_;
  std::once_flag start_flag_, socket_close_flag_;
  boost::asio::ip::tcp::socket socket_;
  MessageReceivedFunctor on_message_received_;
  ConnectionClosedFunctor on_connection_closed_;
  ReceivingMessage receiving_message_;
  std::deque<SendingMessage> send_queue_;
};

}  // namespace tcp

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TCP_CONNECTION_H_
