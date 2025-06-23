// File: src/ConnectionManager.cpp
#include "ConnectionManager.h"
#include <iostream>

ConnectionManager::ConnectionManager(boost::asio::io_context& ioc,
                                     const std::string& host,
                                     unsigned short port)
  : ioc_(ioc),
    socket_(ioc_),
    host_(host),
    port_(port)
{}

void ConnectionManager::setMessageHandler(MessageHandler handler) {
    handler_ = std::move(handler);
}

void ConnectionManager::start() {
    doConnect();
}

void ConnectionManager::stop() {
    stopped_ = true;
    boost::system::error_code ec;
    socket_.close(ec);
}

void ConnectionManager::doConnect() {
    auto endpoint = boost::asio::ip::tcp::endpoint{
        boost::asio::ip::make_address(host_), port_};
    socket_.async_connect(endpoint,
        [this](auto ec) { onConnect(ec); });
}

void ConnectionManager::onConnect(const boost::system::error_code& ec) {
    if (stopped_) return;
    if (ec) {
        std::cerr << "ConnectionManager: connect error: " << ec.message() << "\n";
        // retry after delay
        boost::asio::steady_timer t{ioc_, std::chrono::seconds(5)};
        t.async_wait([this](auto){ doConnect(); });
        return;
    }
    std::cout << "ConnectionManager: connected to " << host_ << ":" << port_ << "\n";
    doReadLine();
}

void ConnectionManager::doReadLine() {
    if (stopped_) return;
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this](auto ec, auto bytes_transferred){
            onReadLine(ec, bytes_transferred);
        });
}

void ConnectionManager::onReadLine(const boost::system::error_code& ec,
                                   std::size_t /*bytes_transferred*/)
{
    if (stopped_) return;
    if (ec) {
        std::cerr << "ConnectionManager: read error: " << ec.message() << "\n";
        socket_.close();
        doConnect();
        return;
    }

    std::istream is(&buffer_);
    std::string line;
    std::getline(is, line);
    if (!line.empty() && handler_) {
        handler_(line);
    }

    doReadLine();
}
