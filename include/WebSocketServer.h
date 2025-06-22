// File: include/WebSocketServer.h
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <set>
#include <mutex>
#include <string>

/// A simple Boost.Beast WebSocket server that broadcasts incoming messages.
class WebSocketServer {
public:
    WebSocketServer(boost::asio::io_context& ioc, unsigned short port);

    /// Begin accepting clients
    void start();

    /// Broadcast a text message to all connected sessions
    void broadcast(const std::string& message);

private:
    struct Session : std::enable_shared_from_this<Session> {
        Session(boost::asio::ip::tcp::socket socket,
                WebSocketServer& parent);

        void start();
        void send(const std::string& msg);

    private:
        void onAccept(boost::beast::error_code ec);
        void onWrite(boost::beast::error_code ec, std::size_t);

        WebSocketServer&                          parent_;
        boost::beast::websocket::stream<
            boost::asio::ip::tcp::socket>        ws_;
        std::mutex                                writeMutex_;
    };

    void doAccept();

    boost::asio::io_context&                   ioc_;
    boost::asio::ip::tcp::acceptor             acceptor_;
    std::set<std::shared_ptr<Session>>         sessions_;
    std::mutex                                 sessionsMutex_;
};
