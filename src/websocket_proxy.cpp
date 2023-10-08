#include "websocket_proxy_cpp/websocket_proxy.hpp"

#include <boost/asio/co_spawn.hpp>
#include <iostream>

namespace http = beast::http;

net::awaitable<void> do_session(stream websocket)
{
  // Set suggested timeout settings for the websocket
  websocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

  // Set a decorator to change the Server of the handshake
  websocket.set_option(websocket::stream_base::decorator(
      [](websocket::response_type& res)
      { res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-coro"); }));

  // Accept the websocket handshake
  co_await websocket.async_accept();

  for (;;)
  {
    try
    {
      // This buffer will hold the incoming message
      beast::flat_buffer buffer;

      // Read a message
      co_await websocket.async_read(buffer);

      // Echo the message back
      websocket.text(websocket.got_text());
      co_await websocket.async_write(buffer.data());
    }
    catch (boost::system::system_error& se)
    {
      if (se.code() != websocket::error::closed)
      {
        throw;
      }
    }
  }
}

//------------------------------------------------------------------------------

// Accepts incoming connections on endpoint and launches the sessions
net::awaitable<void> listen(tcp::endpoint endpoint)
{
  // Open the acceptor
  auto acceptor = net::use_awaitable_t<>::as_default_on(tcp::acceptor(co_await net::this_coro::executor));
  acceptor.open(endpoint.protocol());

  // Allow address reuse
  acceptor.set_option(net::socket_base::reuse_address(true));

  // Bind to the server address
  acceptor.bind(endpoint);

  // Start listening for connections
  acceptor.listen(net::socket_base::max_listen_connections);

  for (;;)
  {
    net::co_spawn(
        acceptor.get_executor(),
        do_session(stream(co_await acceptor.async_accept())),
        [](const std::exception_ptr& error)
        {
          try
          {
            std::rethrow_exception(error);
          }
          catch (std::exception& err)
          {
            std::cerr << "Error in session: " << err.what() << "\n";
          }
        });
  }
}