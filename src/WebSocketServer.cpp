// File: src/WebSocketServer.cpp
#include "WebSocketServer.h"
#include <iostream>

using tcp   = boost::asio::ip::tcp;
namespace ws = boost::beast::websocket;

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc,
                                 unsigned short port)
  : ioc_(ioc),
    acceptor_{ioc_, {tcp::v4(), port}}
{}

void WebSocketServer::start() {
    doAccept();
}

void WebSocketServer::broadcast(const std::string& message) {
    std::lock_guard lock(sessionsMutex_);
    std::cout << "Broadcasting message to " << sessions_.size()
        << " session(s)\n";
    for (auto& session : sessions_) {
        session->send(message);
    }
}

void WebSocketServer::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket sock) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(sock), *this);
                {
                    std::lock_guard lock(sessionsMutex_);
                    sessions_.insert(session);
                }
                session->start();
            } else {
                std::cerr << "WebSocketServer: accept error: " << ec.message() << "\n";
            }
            doAccept();
        });
}

// — Session —

WebSocketServer::Session::Session(tcp::socket socket,
                                  WebSocketServer& parent)
  : parent_(parent),
    ws_(std::move(socket))
{}

void WebSocketServer::Session::start() {
    ws_.async_accept(
        [self = shared_from_this()](boost::beast::error_code ec) {
            self->onAccept(ec);
        });
}

void WebSocketServer::Session::onAccept(boost::beast::error_code ec) {
    if (!ec) {
        std::cout << "Session: client connected\n";
    } else {
        std::cerr << "Session: accept handshake error: " << ec.message() << "\n";
    }
}

void WebSocketServer::Session::send(const std::string& msg) {
    std::lock_guard lock(writeMutex_);
    ws_.async_write(boost::asio::buffer(msg),
        [self = shared_from_this()](boost::beast::error_code ec, std::size_t) {
            self->onWrite(ec, 0);
        });
}

void WebSocketServer::Session::onWrite(boost::beast::error_code ec, std::size_t) {
    if (ec) {
        std::cerr << "Session: write error: " << ec.message() << "\n";
    }
}
