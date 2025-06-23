#include "ConnectionManager.h"
#include <boost/asio.hpp>
#include <iostream>

ConnectionManager::ConnectionManager(boost::asio::io_context& ioc,
                                     const std::string& host,
                                     unsigned short port)
    : ioc_(ioc), socket_(ioc)
{
    boost::asio::ip::tcp::resolver resolver(ioc_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::async_connect(socket_, endpoints,
        [this](boost::system::error_code ec, auto) {
            if (!ec) doRead();
            else std::cerr << "DTN Connect error: " << ec.message() << std::endl;
        });
}

void ConnectionManager::start(MessageHandler handler) {
    handler_ = std::move(handler);
}

void ConnectionManager::doRead() {
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                if (handler_) handler_(line);
                doRead();
            } else {
                std::cerr << "DTN Read error: " << ec.message() << std::endl;
            }
        });
}
