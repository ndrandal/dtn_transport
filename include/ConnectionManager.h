#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

class ConnectionManager {
public:
    using MessageHandler = std::function<void(const std::string&)>;

    ConnectionManager(boost::asio::io_context& ioc,
                      const std::string& host,
                      unsigned short port);

    void start(MessageHandler handler);

private:
    void doRead();

    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    MessageHandler handler_;
};