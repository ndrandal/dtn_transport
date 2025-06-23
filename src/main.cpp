// File: src/main.cpp

#include "SchemaLoader.h"
#include "MessageDecoder.h"
#include "ConnectionManager.h"
#include "WebSocketServer.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <boost/asio.hpp>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// trim helper
static std::string trim(const std::string& s) {
    auto ws = " \t\r\n";
    auto l = s.find_first_not_of(ws);
    auto r = s.find_last_not_of(ws);
    return (l == std::string::npos) ? "" : s.substr(l, r - l + 1);
}

// Returns the config directory alongside the executable
static std::filesystem::path getConfigDir() {
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        throw std::runtime_error("Unable to determine executable path");
    }
    std::filesystem::path exePath(buf);
    auto exeDir  = exePath.parent_path();       // e.g. build/Debug
    auto buildDir = exeDir.parent_path();       // e.g. build/
    return buildDir / "config";
}

int main() {
    try {
        boost::asio::io_context ioc;
        auto configDir = getConfigDir();

        // — Load schemas —
        if (!SchemaLoader::load("L1", (configDir / "L1FeedMessages.csv").string())) {
            std::cerr << "Failed to load L1 schema\n";
            return 1;
        }
        if (!SchemaLoader::load("L2", (configDir / "MarketDepthMessages.csv").string())) {
            std::cerr << "Failed to load L2 schema\n";
            return 1;
        }

        // — Read symbols from CSV —
        std::vector<std::string> symbols;
        {
            std::ifstream in(configDir / "symbols.csv");
            if (!in.is_open()) {
                std::cerr << "Could not open symbols.csv\n";
                return 1;
            }
            std::string line;
            while (std::getline(in, line)) {
                auto s = trim(line);
                if (!s.empty()) symbols.push_back(s);
            }
        }
        if (symbols.empty()) {
            std::cerr << "No symbols found in symbols.csv\n";
            return 1;
        }

        // — Start WebSocket server —
        WebSocketServer ws(ioc, 8080);
        ws.start();
        std::cout << "WebSocketServer listening on port 8080\n";

        // — Admin port connection (9300) —
        ConnectionManager adminConn(ioc, "127.0.0.1", 9300);
        adminConn.setConnectHandler([&]() {
            // negotiate protocol
            adminConn.send("S,SET PROTOCOL,5.2\r\n");
            // optionally register client or name here...
        });
        adminConn.setMessageHandler([&](const std::string& msg) {
            // print stats and other admin messages
            std::cout << "[ADMIN] " << msg << "\n";
        });
        adminConn.start();
        std::cout << "Connecting to Admin port on 127.0.0.1:9300\n";

        // — Level 1 feed connection (5009) —
        ConnectionManager tickConn(ioc, "127.0.0.1", 5009);
        tickConn.setConnectHandler([&]() {
            // protocol handshake for L1
            tickConn.send("S,SET PROTOCOL,5.2\r\n");
            for (auto& sym : symbols) {
                std::cout << "requesting symbol ", sym, " \n";
                // 'w' followed by the uppercase symbol, then CRLF
                tickConn.send("w" + sym + "\r\n");
            }
        });
        tickConn.setMessageHandler([&](const std::string& msg) {
            // echo for debug
            std::cout << "[L1] " << msg << "\n";

            if (msg.rfind("S,KEY,", 0) == 0) {
                // respond to key challenge
                tickConn.send(msg + "\r\n");
                return;
            }
            // skip system messages (starting with S, T, etc.)
            char mt = msg.empty() ? 0 : msg[0];
            if (!((mt >= '0' && mt <= '9') || mt == 'Q')) {
                std::cout << "Skipping non-data L1 msg: " << msg << "\n";
                return;
            }

            rapidjson::Document d;
            if (!MessageDecoder::decode(SchemaLoader::fields("L1"), msg, d)) {
                std::cout << "Couldnt decode feilds\n";
                return;
            }

            auto& a = d.GetAllocator();
            d.AddMember("feed", rapidjson::Value("L1", a), a);
            d.AddMember("messageType",
                        rapidjson::Value(std::string(1,mt).c_str(), a), a);

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> w(sb);
            d.Accept(w);
            std::cout << "maade it here\n";
            ws.broadcast(sb.GetString());
        });
        tickConn.start();
        std::cout << "Connecting to L1 feed on 127.0.0.1:5009\n";

        // — Level 2 feed connection (9200) —
        ConnectionManager depthConn(ioc, "127.0.0.1", 9200);
        depthConn.setConnectHandler([&]() {
            depthConn.send("S,SET PROTOCOL,5.2\r\n");
            for (auto& sym : symbols) {
                depthConn.send("WOR," + sym + "\r\n");
            }
        });
        depthConn.setMessageHandler([&](const std::string& msg) {
            // skip non-data codes
            if (msg.empty()) return;
            char mt = msg[0];
            if (!(mt=='0'||mt=='3'||mt=='4'||mt=='5'||
                  mt=='6'||mt=='7'||mt=='8'||mt=='9')) return;

            rapidjson::Document d;
            if (!MessageDecoder::decode(SchemaLoader::fields("L2"), msg, d))
                return;

            auto& a = d.GetAllocator();
            d.AddMember("feed", rapidjson::Value("L2", a), a);
            d.AddMember("messageType",
                        rapidjson::Value(std::string(1,mt).c_str(), a), a);

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> w(sb);
            d.Accept(w);
            ws.broadcast(sb.GetString());
        });
        depthConn.start();
        std::cout << "Connecting to L2 feed on 127.0.0.1:9200\n";

        // — Run the I/O loop —
        ioc.run();
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
}
