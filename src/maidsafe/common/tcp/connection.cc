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

#include "maidsafe/common/tcp/connection.h"

#include <condition_variable>

#include "boost/asio/error.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/utils.h"

namespace Asio = boost::asio;
namespace ip = Asio::ip;

namespace maidsafe {

namespace tcp {

Connection::Connection(AsioService& asio_service)
    : io_service_(asio_service.service()),
      start_flag_(),
      socket_close_flag_(),
      socket_(io_service_),
      on_message_received_(),
      on_connection_closed_(),
      receiving_message_(),
      send_queue_() {
  static_assert((sizeof(DataSize)) == 4, "DataSize must be 4 bytes.");
  assert(!socket_.is_open());
  if (asio_service.ThreadCount() != 1U) {
    LOG(kError) << "This must be a single-threaded io_service, or an asio strand will be required.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
}

Connection::Connection(AsioService& asio_service, Port remote_port)
    : io_service_(asio_service.service()),
      start_flag_(),
      socket_close_flag_(),
      socket_(io_service_),
      on_message_received_(),
      on_connection_closed_(),
      receiving_message_(),
      send_queue_() {
  if (asio_service.ThreadCount() != 1U) {
    LOG(kError) << "This must be a single-threaded io_service, or an asio strand will be required.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }

  boost::system::error_code connect_error;
  // Try IPv6 first.
  socket_.connect(ip::tcp::endpoint{ip::address_v6::loopback(), remote_port}, connect_error);
  if (connect_error &&
      connect_error == Asio::error::make_error_code(Asio::error::address_family_not_supported)) {
    // Try IPv4 now.
    socket_.connect(ip::tcp::endpoint{ip::address_v4::loopback(), remote_port}, connect_error);
  }
  boost::system::error_code remote_endpoint_error;
  socket_.remote_endpoint(remote_endpoint_error);
  if (connect_error || remote_endpoint_error) {
    LOG(kError) << "Failed to connect to " << remote_port << ": " << connect_error.message();
    BOOST_THROW_EXCEPTION(MakeError(VaultManagerErrors::failed_to_connect));
  }
}

ConnectionPtr Connection::MakeShared(AsioService& asio_service) {
  return ConnectionPtr{new Connection{asio_service}};
}

ConnectionPtr Connection::MakeShared(AsioService& asio_service, Port remote_port) {
  return ConnectionPtr{new Connection{asio_service, remote_port}};
}

void Connection::Start(MessageReceivedFunctor on_message_received,
                       ConnectionClosedFunctor on_connection_closed) {
  std::call_once(start_flag_, [=] {
    on_message_received_ = on_message_received;
    on_connection_closed_ = on_connection_closed;
    ConnectionPtr this_ptr{shared_from_this()};
    io_service_.dispatch([this_ptr] { this_ptr->ReadSize(); });
  });
}

void Connection::Close() {
  ConnectionPtr this_ptr{shared_from_this()};
  io_service_.post([this_ptr] { this_ptr->DoClose(); });
}

void Connection::DoClose() {
  std::call_once(socket_close_flag_, [this] {
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ignored_ec);
    socket_.close(ignored_ec);
    if (on_connection_closed_)
      on_connection_closed_();
  });
}

void Connection::ReadSize() {
  ConnectionPtr this_ptr{shared_from_this()};
  Asio::async_read(socket_, Asio::buffer(receiving_message_.size_buffer),
                   [this_ptr](const boost::system::error_code& ec, size_t bytes_transferred) {
    if (ec) {
      LOG(kInfo) << ec.message();
      return this_ptr->DoClose();
    }
    assert(bytes_transferred == 4U);
    static_cast<void>(bytes_transferred);

    DataSize data_size;
    data_size = (((((this_ptr->receiving_message_.size_buffer[0] << 8) |
                    this_ptr->receiving_message_.size_buffer[1])
                   << 8) |
                  this_ptr->receiving_message_.size_buffer[2])
                 << 8) |
                this_ptr->receiving_message_.size_buffer[3];
    if (data_size > MaxMessageSize()) {
      LOG(kError) << "Incoming message size of " << data_size
                  << " bytes exceeds maximum allowed of " << MaxMessageSize() << " bytes.";
      this_ptr->receiving_message_.data_buffer.clear();
      return this_ptr->DoClose();
    }

    this_ptr->receiving_message_.data_buffer.resize(data_size);
    this_ptr->ReadData();
  });
}

void Connection::ReadData() {
  ConnectionPtr this_ptr{shared_from_this()};
  Asio::async_read(
      socket_, Asio::buffer(receiving_message_.data_buffer),
      io_service_.wrap([this_ptr](const boost::system::error_code& ec, size_t bytes_transferred) {
        if (ec) {
          LOG(kError) << "Failed to read message body: " << ec.message();
          return this_ptr->DoClose();
        }
        assert(bytes_transferred == this_ptr->receiving_message_.data_buffer.size());
        static_cast<void>(bytes_transferred);

        // Dispatch the message outside the strand.
        std::string data{std::begin(this_ptr->receiving_message_.data_buffer),
                         std::end(this_ptr->receiving_message_.data_buffer)};
        this_ptr->io_service_.post([=] { this_ptr->on_message_received_(std::move(data)); });
        this_ptr->io_service_.dispatch([this_ptr] { this_ptr->ReadSize(); });
      }));
}

void Connection::Send(std::string data) {
  SendingMessage message(EncodeData(std::move(data)));
  ConnectionPtr this_ptr{shared_from_this()};
  io_service_.post([this_ptr, message] {
    bool currently_sending{!this_ptr->send_queue_.empty()};
    this_ptr->send_queue_.emplace_back(std::move(message));
    if (!currently_sending)
      this_ptr->DoSend();
  });
}

void Connection::DoSend() {
  std::array<Asio::const_buffer, 2> buffers;
  buffers[0] = Asio::buffer(send_queue_.front().size_buffer);
  buffers[1] = Asio::buffer(send_queue_.front().data.data(), send_queue_.front().data.size());
  ConnectionPtr this_ptr{shared_from_this()};
  Asio::async_write(
      socket_, buffers,
      io_service_.wrap([this_ptr](const boost::system::error_code& ec, size_t bytes_transferred) {
        if (ec) {
          LOG(kError) << "Failed to send message: " << ec.message();
          return this_ptr->DoClose();
        }
        assert(bytes_transferred ==
               this_ptr->send_queue_.front().size_buffer.size() +
                   this_ptr->send_queue_.front().data.size());
        static_cast<void>(bytes_transferred);

        this_ptr->send_queue_.pop_front();
        if (!this_ptr->send_queue_.empty())
          this_ptr->DoSend();
      }));
}

Connection::SendingMessage Connection::EncodeData(std::string data) const {
  if (data.empty())
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_string_size));
  if (data.size() > MaxMessageSize())
    BOOST_THROW_EXCEPTION(MakeError(VaultManagerErrors::ipc_message_too_large));

  SendingMessage message;
  for (int i = 0; i != 4; ++i)
    message.size_buffer[i] = static_cast<char>(data.size() >> (8 * (3 - i)));
  message.data = std::move(data);

  return message;
}

}  // namespace tcp

}  // namespace maidsafe
