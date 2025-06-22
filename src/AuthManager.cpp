// File: src/AuthManager.cpp
#include "AuthManager.h"
#include <fstream>
#include <iostream>

bool AuthManager::authenticate(boost::asio::io_context& ioc,
                               const std::string& host,
                               unsigned short port,
                               const std::string& credFile)
{
    // 1) Load credentials
    std::ifstream in(credFile);
    if (!in.is_open()) {
        std::cerr << "AuthManager: cannot open credentials file: "
                  << credFile << "\n";
        return false;
    }
    std::string line;
    std::getline(in, line);
    in.close();

    auto commaPos = line.find(',');
    if (commaPos == std::string::npos) {
        std::cerr << "AuthManager: invalid credentials format\n";
        return false;
    }
    std::string user = line.substr(0, commaPos);
    std::string pass = line.substr(commaPos + 1);

    try {
        // 2) Connect to admin port
        boost::asio::ip::tcp::socket sock(ioc);
        boost::asio::ip::tcp::endpoint ep{
            boost::asio::ip::make_address(host), port};
        sock.connect(ep);

        // 3) Send login request
        std::string req = "LOGIN," + user + "," + pass + "\n";
        boost::asio::write(sock, boost::asio::buffer(req));

        // 4) Wait for response
        boost::asio::streambuf respBuf;
        boost::asio::read_until(sock, respBuf, '\n');
        std::istream is(&respBuf);
        std::string respLine;
        std::getline(is, respLine);

        if (respLine == "OK") {
            std::cout << "AuthManager: authenticated successfully\n";
            return true;
        } else {
            std::cerr << "AuthManager: auth failed: "
                      << respLine << "\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "AuthManager: exception: " << e.what() << "\n";
        return false;
    }
}
