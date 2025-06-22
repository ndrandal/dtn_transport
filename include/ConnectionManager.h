// File: include/ConnectionManager.h
#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <string>

/// Connects to DTN on localhost, reads CSV lines, calls your handler.
class ConnectionManager {
public:
    using MessageHandler = std::function<void(const std::string&)>;

    ConnectionManager(boost::asio::io_context& ioc,
                      const std::string& host,
                      unsigned short port);

    /// Set the callback for each complete CSV line received
    void setMessageHandler(MessageHandler handler);

    /// Start the async connect + read loop
    void start();

    /// Stop the connection and any pending operations
    void stop();

private:
    void doConnect();
    void onConnect(const boost::system::error_code& ec);
    void doReadLine();
    void onReadLine(const boost::system::error_code& ec, std::size_t bytes_transferred);

    boost::asio::io_context&       ioc_;
    boost::asio::ip::tcp::socket   socket_;
    std::string                    host_;
    unsigned short                 port_;
    boost::asio::streambuf         buffer_;
    MessageHandler                 handler_;
    bool                           stopped_{false};
};
