#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <iostream>

#include "websocket_proxy_cpp/websocket_proxy.hpp"

int main()
{
  net::io_context io_context;
  static constexpr auto port_num = 1234;
  const auto address = net::ip::make_address_v4("127.0.0.1");
  tcp::endpoint endpoint(address, port_num);
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