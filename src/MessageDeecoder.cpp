#include "MessageDecoder.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>

bool MessageDecoder::decode(const std::string& csvLine, rapidjson::Document& doc) {
    doc.SetObject();
    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    std::istringstream ss(csvLine);
    std::string token;
    int fieldIndex = 0;

    // TODO: Replace with actual field mapping from DTN docs
    while (std::getline(ss, token, ',')) {
        std::string key = "field" + std::to_string(fieldIndex++);
        doc.AddMember(
            rapidjson::Value(key.c_str(), alloc),
            rapidjson::Value(token.c_str(), alloc),
            alloc
        );
    }

    return true;
}
