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

    // Define numeric field names for type conversion
    static const std::unordered_set<std::string> intFields = {
        "Order Size", "Total Volume", "Bid Size", "Ask Size",
        "Order Priority", "Level Size", "Order Count"
    };
    static const std::unordered_set<std::string> floatFields = {
        "Price", "Most Recent Trade", "Open", "High", "Low", "Close"
    };

    doc.SetObject();
    auto& alloc = doc.GetAllocator();
    std::string dateStr, timeStr;

    // Split the CSV line by commas, preserving empty fields
    std::vector<std::string> tokens;
    size_t start = 0;
    while (true) {
        size_t end = csvLine.find(',', start);
        tokens.emplace_back(csvLine.substr(start, end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }

    // If a message type is present, adjust for message-specific format differences
    if (!tokens.empty()) {
        std::string msgTypeToken = tokens[0];
        if (msgTypeToken == "5") {
            // Message type '5' (order remove) omits several fields in the L2 schema.
            // Insert empty placeholders for omitted fields to align tokens with schema keys.
            std::vector<size_t> omitPositions = {3, 5, 6, 7, 8, 9, 10};
            // Insert from lowest to highest index (each insertion increases subsequent indices)
            for (size_t posIndex = 0; posIndex < omitPositions.size(); ++posIndex) {
                size_t pos = omitPositions[posIndex];
                if (pos <= tokens.size()) {
                    tokens.insert(tokens.begin() + pos, std::string());  // empty placeholder
                }
            }
        }
        // (If other message types needed special handling, it can be added here similarly)
    }

    // Populate JSON fields for all keys, using tokens or null if token is missing
    for (size_t idx = 0; idx < keys.size(); ++idx) {
        // Get token if available, otherwise use empty string for missing field
        std::string token = (idx < tokens.size() ? tokens[idx] : "");
        // Trim whitespace and CR/LF from token
        auto l = token.find_first_not_of(" \t\r\n");
        auto r = token.find_last_not_of(" \t\r\n");
        token = (l == std::string::npos) ? std::string() : token.substr(l, r - l + 1);

        const std::string& fieldName = keys[idx];
        if (fieldName == "Date") {
            dateStr = token;
        } else if (fieldName == "Time") {
            timeStr = token;
        }

        if (token.empty()) {
            // If field is empty/missing, add as JSON null
            doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(rapidjson::kNullType), alloc);
        } else if (intFields.count(fieldName)) {
            // Parse integer fields
            try {
                long long v = std::stoll(token);
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(v), alloc);
            } catch (...) {
                // If parsing fails, keep as string
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
            }
        } else if (floatFields.count(fieldName)) {
            // Parse floating-point fields
            try {
                double v = std::stod(token);
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(v), alloc);
            } catch (...) {
                doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
            }
        } else {
            // Default: treat as string
            doc.AddMember(rapidjson::Value(fieldName.c_str(), alloc), rapidjson::Value(token.c_str(), alloc), alloc);
        }
    }

    // Combine Date and Time fields into a single ISO timestamp, if both are present
    if (!dateStr.empty() && !timeStr.empty()) {
        std::string isoTimestamp = dateStr + "T" + timeStr + "Z";
        doc.AddMember(rapidjson::Value("timestamp", alloc), rapidjson::Value(isoTimestamp.c_str(), alloc), alloc);
        doc.RemoveMember("Date");
        doc.RemoveMember("Time");
    }

    return !tokens.empty();
}
