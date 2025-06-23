// File: src/main.cpp
#include "SchemaLoader.h"
#include "MessageDecoder.h"
#include "ConnectionManager.h"
#include "WebSocketServer.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>

int main() {
    boost::asio::io_context ioc;

    // 1) Load your DTN CSV schema (header row)
    if (!SchemaLoader::load("config/L1FeedMessages.csv")) {
        std::cerr << "Failed to load schema → exiting\n";
        return 1;
    }

    // 2) Start WebSocket server for middleware clients
    WebSocketServer wsServer(ioc, 8080);
    wsServer.start();
    std::cout << "WebSocketServer: listening on port 8080\n";

    // 3) Connect to DTN and handle each CSV line
    ConnectionManager conn(ioc, "127.0.0.1", 5009);
    conn.setMessageHandler(
        [&wsServer](const std::string& csvLine) {
            rapidjson::Document doc;
            if (!MessageDecoder::decode(csvLine, doc)) {
                // drop bad lines
                return;
            }
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            doc.Accept(writer);
            wsServer.broadcast(sb.GetString());
        });
    conn.start();
    std::cout << "ConnectionManager: connecting to DTN on 127.0.0.1:5009\n";

    // 4) Run the I/O loop
    ioc.run();
    return 0;
}
