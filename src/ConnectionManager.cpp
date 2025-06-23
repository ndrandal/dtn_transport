// File: src/ConnectionManager.cpp
#include "ConnectionManager.h"
#include <boost/asio/write.hpp>
#include <iostream>

ConnectionManager::ConnectionManager(boost::asio::io_context& ioc,
                                     const std::string& host,
                                     unsigned short port)
  : ioc_(ioc),
    socket_(ioc_),
    host_(host),
    port_(port)
{}

void ConnectionManager::setConnectHandler(ConnectHandler h) {
    onConnect_ = std::move(h);
}

void ConnectionManager::setMessageHandler(MessageHandler h) {
    onMessage_ = std::move(h);
}

void ConnectionManager::start() {
    doConnect();
}

void ConnectionManager::stop() {
    stopped_ = true;
    boost::system::error_code ec;
    socket_.close(ec);
}

void ConnectionManager::send(const std::string& cmd) {
    // post to io_context so it's safe even if called from a handler
    boost::asio::post(ioc_, [this, cmd]() {
        boost::system::error_code ec;
        boost::asio::write(socket_, boost::asio::buffer(cmd), ec);
        if (ec) {
            std::cerr << "Send error: " << ec.message() << "\n";
        }
    });
}

void ConnectionManager::doConnect() {
    auto ep = boost::asio::ip::tcp::endpoint{
        boost::asio::ip::make_address(host_), port_};
    socket_.async_connect(ep,
        [this](auto ec){ onConnect(ec); });
}

void ConnectionManager::onConnect(const boost::system::error_code& ec) {
    if (stopped_) return;
    if (ec) {
        std::cerr << "Connect error: " << ec.message() << "\n";
        // retry after 5s
        boost::asio::steady_timer t{ioc_, std::chrono::seconds(5)};
        t.async_wait([this](auto){ doConnect(); });
        return;
    }
    std::cout << "Connected to " << host_ << ":" << port_ << "\n";
    if (onConnect_) onConnect_();
    doReadLine();
}

void ConnectionManager::doReadLine() {
    if (stopped_) return;
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this](auto ec, auto n){ onReadLine(ec, n); });
}

void ConnectionManager::onReadLine(const boost::system::error_code& ec,
                                   std::size_t /*n*/) {
    if (stopped_) return;
    if (ec) {
        std::cerr << "Read error: " << ec.message() << "\n";
        socket_.close();
        doConnect();
        return;
    }
    std::istream is(&buffer_);
    std::string line;
    std::getline(is, line);
    if (!line.empty() && onMessage_) {
        onMessage_(line);
    }
    doReadLine();
}
