#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <set>
#include <string>

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket&& socket);
    void start();
    void send(const std::string& message);

private:
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
};

class WebSocketServer {
public:
    WebSocketServer(boost::asio::io_context& ioc, unsigned short port);
    void run();
    void broadcast(const std::string& message);

private:
    void doAccept();

    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::set<std::shared_ptr<WebSocketSession>> sessions_;
};