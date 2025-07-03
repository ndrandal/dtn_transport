// File: src/MessageDecoder.cpp
#include "MessageDecoder.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

bool MessageDecoder::decode(const std::vector<std::string>& keys,
                            const std::string& csvLine,
                            rapidjson::Document& doc) {
    if (keys.empty()) {
        std::cerr << "MessageDecoder: empty schema\n";
        return false;
    }

    static const std::unordered_set<std::string> intFields = {
        "Order Size","Total Volume","Bid Size","Ask Size",
        "Order Priority","Level Size","Order Count"
    };
    static const std::unordered_set<std::string> floatFields = {
        "Price","Most Recent Trade","Open","High","Low","Close"
    };

    doc.SetObject();
    auto& alloc = doc.GetAllocator();
    std::string dateStr, timeStr;

    // Manual split preserving empty fields
    std::vector<std::string> tokens;
    size_t start = 0;
    while (true) {
        size_t end = csvLine.find(',', start);
        tokens.emplace_back(csvLine.substr(start, end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }

    for (size_t idx = 0; idx < tokens.size(); ++idx) {
        std::string token = tokens[idx];
        // Trim whitespace and CR/LF
        auto l = token.find_first_not_of(" \t\r\n");
        auto r = token.find_last_not_of(" \t\r\n");
        token = (l == std::string::npos) ? std::string{} : token.substr(l, r - l + 1);

        const std::string& fieldName = (idx < keys.size()) ? keys[idx] : ("field" + std::to_string(idx));

        if (fieldName == "Date") dateStr = token;
        else if (fieldName == "Time") timeStr = token;

        // Numeric and default handling
        if (intFields.count(fieldName)) {
            try {
                long long v = std::stoll(token);
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(v), alloc);
            } catch (...) {
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
            }
        } else if (floatFields.count(fieldName)) {
            try {
                double v = std::stod(token);
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(v), alloc);
            } catch (...) {
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
            }
        } else {
            doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
        }
    }

    if (!dateStr.empty() && !timeStr.empty()) {
        std::string iso = dateStr + "T" + timeStr + "Z";
        doc.AddMember("timestamp", rapidjson::Value(iso.c_str(), alloc), alloc);
        doc.RemoveMember("Date");
        doc.RemoveMember("Time");
    }

    return !tokens.empty();
}