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

#include "maidsafe/common/transport/tcp_listener.h"

#include <condition_variable>
#include <limits>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"

#include "maidsafe/common/transport/tcp_connection.h"

namespace asio = boost::asio;

namespace maidsafe {

namespace transport {

TcpListener::TcpListener(AsioService &asio_service, NewConnectionFunctor on_new_connection)
    : asio_service_(asio_service),
      stop_listening_flag_(),
      on_new_connection_(on_new_connection),
      acceptor_(asio_service_.service()) {
  if (asio_service.ThreadCount() != 1U) {
    LOG(kError) << "This must be a single-threaded io_service, or an asio strand will be required.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
  }
}

TcpListenerPtr TcpListener::MakeShared(
    AsioService &asio_service,
    NewConnectionFunctor on_new_connection, Port desired_port) {
  std::shared_ptr<TcpListener> listener{ new TcpListener{ asio_service, on_new_connection } };
  listener->StartListening(desired_port);
  return listener;
}

Port TcpListener::ListeningPort() const {
  return acceptor_.local_endpoint().port();
}

void TcpListener::StartListening(Port desired_port) {
  unsigned attempts{ 0 };
  while (attempts <= kMaxRangeAboveDefaultPort &&
    desired_port + attempts <= std::numeric_limits<Port>::max() && !acceptor_.is_open()) {
    try {
      DoStartListening(static_cast<Port>(desired_port + attempts));
    }
    catch (const std::exception& e) {
      LOG(kWarning) << "Failed to start listening on port " << desired_port + attempts << ": "
        << boost::diagnostic_information(e);
      ++attempts;
    }
  }
  if (!acceptor_.is_open()) {
    LOG(kError) << "Failed to start listening on any port in the range [" << desired_port
      << ", " << desired_port + attempts << "]";
    BOOST_THROW_EXCEPTION(MakeError(VaultManagerErrors::failed_to_listen));
  }
}

void TcpListener::DoStartListening(Port port) {
  // Try IPv6 first.
  asio::ip::tcp::endpoint endpoint{ asio::ip::address_v6::loopback(), port };
  on_scope_exit cleanup_on_error([&] {
    boost::system::error_code ec;
    acceptor_.close(ec);
  });

  try {
    acceptor_.open(endpoint.protocol());
  }
  catch (const boost::system::system_error& error) {
    if (error.code() == asio::error::make_error_code(asio::error::address_family_not_supported)) {
      // Try IPv4 now.
      endpoint = asio::ip::tcp::endpoint{ asio::ip::address_v4::loopback(), port };
      acceptor_.open(endpoint.protocol());
    } else {
      throw;
    }
  }

  // Below option is interpreted differently by Windows and shouldn't be used.  On, Windows, this
  // will allow two processes to listen on the same port.  On a POSIX-compliant OS, this option
  // tells the kernel that even if given port is busy (only TIME_WAIT state), go ahead and reuse it
  // anyway.  If it's busy, but with a different state, you will still get an 'address already in
  // use' error.  For further info, see:
  // http://msdn.microsoft.com/en-us/library/ms740621(VS.85).aspx
  // http://www.unixguide.net/network/socketfaq/4.5.shtml
  // http://old.nabble.com/Port-allocation-problem-on-windows-(incl.-patch)-td28241079.html
#ifndef MAIDSAFE_WIN32
  acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
#endif
  acceptor_.bind(endpoint);
  acceptor_.listen(asio::socket_base::max_connections);

  // The connection object is kept alive in the acceptor handler until HandleAccept() is called.
  TcpConnectionPtr connection{ TcpConnection::MakeShared(asio_service_) };
  TcpListenerPtr this_ptr{ shared_from_this() };
  acceptor_.async_accept(connection->Socket(), asio_service_.service().wrap(
      [this_ptr, connection](const boost::system::error_code& error) {
        this_ptr->HandleAccept(connection, error);
      }));

  cleanup_on_error.Release();
}

void TcpListener::HandleAccept(TcpConnectionPtr accepted_connection,
                               const boost::system::error_code& ec) {
  if (!acceptor_.is_open() || asio_service_.service().stopped())
    return;

  if (ec)
    LOG(kWarning) << "Error while accepting connection: " << ec.message();
  else
    on_new_connection_(accepted_connection);

  // The connection object is kept alive in the acceptor handler until HandleAccept() is called.
  TcpConnectionPtr connection{ TcpConnection::MakeShared(asio_service_) };
  TcpListenerPtr this_ptr{ shared_from_this() };
  acceptor_.async_accept(connection->Socket(), asio_service_.service().wrap(
      [this_ptr, connection](const boost::system::error_code& error) {
        this_ptr->HandleAccept(connection, error);
      }));
}

void TcpListener::StopListening() {
  asio_service_.service().post([this] { DoStopListening(); });
}

void TcpListener::DoStopListening() {
  std::call_once(stop_listening_flag_, [this] {
    boost::system::error_code ec;
    if (acceptor_.is_open())
      acceptor_.close(ec);
    if (ec.value() != 0)
      LOG(kError) << "Acceptor close error: " << ec.message();
  });
}

}  // namespace transport

}  // namespace maidsafe
