#include "websocket_proxy_cpp/websocket_proxy.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <iostream>

namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>
namespace beast = boost::beast;          // from <boost/beast.hpp>

// this is a stream which makes the executor have awaitable as its default completion handler
using awaitable_stream = websocket::stream<typename beast::tcp_stream::rebind_executor<
    typename net::use_awaitable_t<>::executor_with_default<net::any_io_executor>>::other>;

namespace {
std::string extract_target_from_url(const std::string& url) {
    std::size_t start_pos = url.find("target=");
    if (start_pos == std::string::npos) {
        return "";
    }

    static constexpr size_t inc = 7;
    start_pos += inc; // Length of "target="
    std::size_t end_pos = url.find('&', start_pos);
    if (end_pos == std::string::npos) {
        end_pos = url.length();
    }

    return url.substr(start_pos, end_pos - start_pos);
}
}

net::awaitable<void> start_websocket_session(awaitable_socket socket)
{
  beast::flat_buffer buffer;
  http::request<http::string_body> req;
  for(;;) {
    co_await http::async_read(socket, buffer, req, net::use_awaitable);

    if(websocket::is_upgrade(req)) {
      std::string target = extract_target_from_url(req.target());

      std::cout << "Target: " << target << std::endl;

      break;
    }

    buffer.clear();
    req.clear();
  }
  auto websocket = awaitable_stream(std::move(socket));

  // Set suggested timeout settings for the websocket
  websocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

  // Set a decorator to change the Server of the handshake
  websocket.set_option(websocket::stream_base::decorator(
      [](websocket::response_type& res)
      { res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-coro"); }));
  
  // Accept the websocket handshake
  co_await websocket.async_accept(req);

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
        start_websocket_session(co_await acceptor.async_accept()),
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