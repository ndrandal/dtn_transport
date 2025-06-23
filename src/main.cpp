#include "SchemaLoader.h"
#include "MessageDecoder.h"
#include "ConnectionManager.h"
#include "WebSocketServer.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

// trim helper
static std::string trim(const std::string& s) {
    auto ws = " \t\r\n";
    auto l = s.find_first_not_of(ws);
    auto r = s.find_last_not_of(ws);
    return (l == std::string::npos) ? "" : s.substr(l, r - l + 1);
}

int main() {
    boost::asio::io_context ioc;

    // — Load schemas —
    if (!SchemaLoader::load("L1", "config/L1FeedMessages.csv")) {
        std::cerr << "Failed to load L1 schema\n";
        return 1;
    }
    if (!SchemaLoader::load("L2", "config/MarketDepthMessages.csv")) {
        std::cerr << "Failed to load L2 schema\n";
        return 1;
    }

    // — Read symbols from CSV (one per line) —
    std::vector<std::string> symbols;
    {
        std::ifstream in("config/symbols.csv");
        if (!in.is_open()) {
            std::cerr << "Could not open config/symbols.csv\n";
            return 1;
        }
        std::string line;
        while (std::getline(in, line)) {
            auto s = trim(line);
            if (!s.empty()) symbols.push_back(s);
        }
    }
    if (symbols.empty()) {
        std::cerr << "No symbols found in config/symbols.csv\n";
        return 1;
    }

    // — Start WebSocket server —
    WebSocketServer ws(ioc, 8080);
    ws.start();
    std::cout << "WebSocketServer listening on port 8080\n";

    // — Connect to Level2 port (9200) —
    ConnectionManager depthConn(ioc, "127.0.0.1", 9200);
    
    // On connect: set protocol & subscribe for each symbol
    depthConn.setConnectHandler([&](){
        depthConn.send("S,SET PROTOCOL,5.2\r\n");
        for (auto& sym : symbols) {
            depthConn.send("WOR," + sym + "\r\n");
        }
    });

    // On each CSV line: filter, decode, tag, broadcast
    depthConn.setMessageHandler([&](const std::string& line){
        if (line.empty()) return;
        char mt = line[0];
        if (!(mt=='0'||mt=='3'||mt=='4'||mt=='5'||
              mt=='6'||mt=='7'||mt=='8'||mt=='9')) return;

        rapidjson::Document d;
        if (!MessageDecoder::decode(SchemaLoader::fields("L2"), line, d))
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
    std::cout << "Connecting to L2 on 127.0.0.1:9200\n";

    ioc.run();
    return 0;
}
