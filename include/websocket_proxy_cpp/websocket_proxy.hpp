#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

using stream = websocket::stream<typename beast::tcp_stream::rebind_executor<
    typename net::use_awaitable_t<>::executor_with_default<net::any_io_executor>>::other>;

// Echoes back all received WebSocket messages
net::awaitable<void> do_session(stream websocket);

// Accepts incoming connections on endpoint and launches the sessions
net::awaitable<void> listen(tcp::endpoint endpoint);
