#include <boost/asio/io_context.hpp>
#include "websocket_proxy_cpp/websocket_proxy.hpp"
#include <boost/asio/co_spawn.hpp>
#include <iostream>

int main() {
    net::io_context io_context;
    static constexpr auto port_num = 1234;
    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port_num);
    net::co_spawn(
        io_context.get_executor(),
        listen(endpoint),
        [](const std::exception_ptr& error)
        {
          try
          {
            std::rethrow_exception(error);
          }
          catch (std::exception& err)
          {
            std::cerr << "Error in listen: " << err.what() << "\n";
          }
        });
    io_context.run();
}