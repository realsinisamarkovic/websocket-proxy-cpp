#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net = boost::asio;             // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

using awaitable_socket = net::use_awaitable_t<>::as_default_on_t<tcp::socket>;


// Accepts incoming connections on endpoint and launches the sessions
net::awaitable<void> listen(tcp::endpoint endpoint);

// Echoes back all received WebSocket messages
net::awaitable<void> start_websocket_session(awaitable_socket socket);
