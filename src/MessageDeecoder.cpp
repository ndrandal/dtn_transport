// File: src/MessageDecoder.cpp
#include "MessageDecoder.h"
#include "SchemaLoader.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <iostream>

bool MessageDecoder::decode(const std::string& csvLine, rapidjson::Document& doc) {
    const auto& keys = SchemaLoader::fields();
    if (keys.empty()) {
        std::cerr << "MessageDecoder: no schema loaded\n";
        return false;
    }

    doc.SetObject();
    auto& alloc = doc.GetAllocator();
    std::istringstream ss(csvLine);
    std::string token;
    size_t idx = 0;

    while (std::getline(ss, token, ',')) {
        const char* key = (idx < keys.size()
            ? keys[idx].c_str()
            : ("field" + std::to_string(idx)).c_str());

        doc.AddMember(
            rapidjson::Value(key, alloc),
            rapidjson::Value(token.c_str(), alloc),
            alloc
        );
        ++idx;
    }

    if (idx == 0) {
        std::cerr << "MessageDecoder: empty CSV line\n";
        return false;
    }
    return true;
}
