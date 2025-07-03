
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

static thread_local rapidjson::Document    _reuseDoc;
static thread_local rapidjson::StringBuffer _reuseSb;
static thread_local rapidjson::Writer<rapidjson::StringBuffer> _reuseWriter{_reuseSb};

static std::string trim(const std::string& s) {
    const auto ws = " \t\r\n";
    auto l = s.find_first_not_of(ws);
    auto r = s.find_last_not_of(ws);
    return (l == std::string::npos) ? std::string{} : s.substr(l, r - l + 1);
}

static std::filesystem::path getConfigDir() {
    wchar_t buf[MAX_PATH];
    const DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) throw std::runtime_error("Unable to determine executable path");
    return std::filesystem::path(buf).parent_path().parent_path() / "config";
}

int main() {
    try {
        boost::asio::io_context ioc;
        auto configDir = getConfigDir();

        if (!SchemaLoader::load("L1", (configDir / "L1FeedMessages.csv").string())) return 1;
        if (!SchemaLoader::load("L2", (configDir / "MarketDepthMessages.csv").string())) return 1;

        std::vector<std::string> symbols;
        std::ifstream in(configDir / "symbols.csv");
        if (!in.is_open()) return 1;
        for (std::string line; std::getline(in, line); ) {
            auto s = trim(line);
            if (!s.empty()) symbols.push_back(s);
        }
        if (symbols.empty()) return 1;

        WebSocketServer ws(ioc, 8080);
        ws.start();
        std::cout << "WebSocketServer listening on port 8080\n";

        ConnectionManager admin(ioc, "127.0.0.1", 9300);
        admin.setConnectHandler([&]() { admin.send("S,SET PROTOCOL,6.2\r\n"); });
        admin.setMessageHandler([&](const std::string& msg) { std::cout << "[ADMIN] " << msg << "\n"; });
        admin.start();

        bool l1sub = false, l2sub = false;
        ConnectionManager l1(ioc, "127.0.0.1", 5009);
        l1.setConnectHandler([&]() { l1.send("S,SET PROTOCOL,6.2\r\n"); });
        l1.setMessageHandler([&](const std::string& raw) {
            auto msg = trim(raw);
            std::cout << "[L1] " << msg << "\n";
            if (!l1sub && msg.rfind("S,SERVER CONNECTED", 0) == 0) {
                for (auto& sym : symbols) l1.send("w" + sym + "\r\n");
                l1sub = true; return;
            }
            if (msg.rfind("S,KEY,", 0) == 0) { l1.send(msg + "\r\n"); return; }
            if (msg.empty() || (!isdigit(msg[0]) && msg[0] != 'Q')) return;
            _reuseDoc.SetObject();
            if (!MessageDecoder::decode(SchemaLoader::fields("L1"), msg, _reuseDoc)) return;
            auto& a = _reuseDoc.GetAllocator();
            _reuseDoc.AddMember("feed", "L1", a);
            _reuseDoc.AddMember("messageType", std::string(1, msg[0]).c_str(), a);
            _reuseSb.Clear(); _reuseWriter.Reset(_reuseSb); _reuseDoc.Accept(_reuseWriter);
            ws.broadcast(_reuseSb.GetString());
        }); l1.start();

        ConnectionManager l2(ioc, "127.0.0.1", 9200);
        l2.setConnectHandler([&]() { l2.send("S,SET PROTOCOL,6.2\r\n"); });
        l2.setMessageHandler([&](const std::string& raw) {
            auto msg = trim(raw);
            std::cout << "[L2] " << msg << "\n";
            if (!l2sub && msg.rfind("S,SERVER CONNECTED", 0) == 0) {
                for (auto& sym : symbols) l2.send("WOR," + sym + "\r\n");
                l2sub = true; return;
            }
            if (msg.empty() || (msg[0] < '0' || msg[0] > '9')) return;
            _reuseDoc.SetObject();
            if (!MessageDecoder::decode(SchemaLoader::fields("L2"), msg, _reuseDoc)) return;
            auto& a2 = _reuseDoc.GetAllocator();
            _reuseDoc.AddMember("feed", "L2", a2);
            _reuseDoc.AddMember("messageType", std::string(1, msg[0]).c_str(), a2);
            _reuseSb.Clear(); _reuseWriter.Reset(_reuseSb); _reuseDoc.Accept(_reuseWriter);
            ws.broadcast(_reuseSb.GetString());
        }); l2.start();

        ioc.run();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
}
