// File: include/ConnectionManager.h
#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <string>

/// Connects to DTN localhost, reads CSV lines, lets you send commands,
/// and notifies you on connect & per‐line.
class ConnectionManager {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ConnectHandler = std::function<void()>;

    ConnectionManager(boost::asio::io_context& ioc,
                      const std::string& host,
                      unsigned short port);

    /// Called once, after TCP connect succeeds.
    void setConnectHandler(ConnectHandler h);

    /// Called for every full CSV line read.
    void setMessageHandler(MessageHandler h);

    /// Start connect/read loop.
    void start();

    /// Stop and close socket.
    void stop();

    /// Send a command string (e.g. "WOR,MSFT\r\n") to the feed.
    void send(const std::string& cmd);

private:
    void doConnect();
    void onConnect(const boost::system::error_code& ec);
    void doReadLine();
    void onReadLine(const boost::system::error_code& ec,
                    std::size_t bytes_transferred);

    boost::asio::io_context&     ioc_;
    boost::asio::ip::tcp::socket socket_;
    std::string                  host_;
    unsigned short               port_;
    boost::asio::streambuf       buffer_;
    ConnectHandler               onConnect_;
    MessageHandler               onMessage_;
    bool                         stopped_{false};
};
