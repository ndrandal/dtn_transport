#include "WebSocketServer.h"
#include <boost/asio.hpp>
#include <iostream>

WebSocketSession::WebSocketSession(boost::asio::ip::tcp::socket&& socket)
    : ws_(std::move(socket)) {}

void WebSocketSession::start() {
    ws_.async_accept(
        [self = shared_from_this()](boost::system::error_code ec) {
            if (!ec) {
                // Ready to send messages
            }
        }
    );
}

void WebSocketSession::send(const std::string& message) {
    ws_.text(true);
    ws_.async_write(
        boost::asio::buffer(message),
        [self = shared_from_this()](boost::system::error_code, std::size_t) {}
    );
}

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc, unsigned short port)
    : ioc_(ioc)
    , acceptor_(ioc_, {boost::asio::ip::tcp::v4(), port}) {}

void WebSocketServer::run() {
    doAccept();
}

void WebSocketServer::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                auto session = std::make_shared<WebSocketSession>(std::move(socket));
                sessions_.insert(session);
                session->start();
            }
            doAccept();
        }
    );
}

void WebSocketServer::broadcast(const std::string& message) {
    for (auto& session : sessions_) {
        session->send(message);
    }
}
