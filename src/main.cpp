#include <boost/asio.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "ConnectionManager.h"
#include "MessageDecoder.h"
#include "WebSocketServer.h"

int main() {
    boost::asio::io_context ioc;

    // DTN feed connection (e.g., L1 ticks)
    const std::string dtnHost = "127.0.0.1";
    const unsigned short dtnPort = 5009;  // use appropriate DTN port

    // WebSocket server port for middleware clients
    const unsigned short wsPort = 8080;

    WebSocketServer wsServer(ioc, wsPort);
    wsServer.run();

    ConnectionManager connMgr(ioc, dtnHost, dtnPort);
    connMgr.start([&](const std::string& csvLine) {
        rapidjson::Document doc;
        if (MessageDecoder::decode(csvLine, doc)) {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            doc.Accept(writer);
            wsServer.broadcast(sb.GetString());
        } else {
            // TODO: log decode error
        }
    });

    ioc.run();
    return 0;
}
