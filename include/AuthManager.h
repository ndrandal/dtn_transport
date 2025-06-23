// File: include/AuthManager.h
#pragma once

#include <boost/asio.hpp>
#include <string>

/// Performs DTN auth via the admin port (e.g. 9300).
class AuthManager {
public:
    /// Reads credentials from a CSV file (user,pass),
    /// connects to the admin port, sends LOGIN,<user>,<pass>,
    /// expects single-line "OK" response.
    /// Returns true on success.
    static bool authenticate(boost::asio::io_context& ioc,
                             const std::string& host,
                             unsigned short port,
                             const std::string& credFile);
};
