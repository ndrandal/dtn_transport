// File: src/MessageDecoder.cpp

#include "MessageDecoder.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string_view>
#include <unordered_set>
#include <iostream>

bool MessageDecoder::decode(const std::vector<std::string>& keys,
                            const std::string& csvLine,
                            rapidjson::Document& doc) {
    if (keys.empty()) {
        std::cerr << "MessageDecoder: empty schema\n";
        return false;
    }

    // Fields that should be treated as integers or floats
    static const std::unordered_set<std::string_view> intFields = {
        "Order Size","Total Volume","Bid Size","Ask Size",
        "Order Priority","Level Size","Order Count"
    };
    static const std::unordered_set<std::string_view> floatFields = {
        "Price","Most Recent Trade","Open","High","Low","Close"
    };

    // Prepare the JSON document
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    // Capture raw date/time for later merging
    std::string dateStr, timeStr;

    // Manual CSV parse without istringstream
    std::string_view csv(csvLine);
    size_t start = 0;
    size_t idx = 0;
    while (start <= csv.size()) {
        size_t end = csv.find(',', start);
        if (end == std::string_view::npos)
            end = csv.size();

        std::string_view token = csv.substr(start, end - start);
        const std::string& fieldName = (idx < keys.size()
            ? keys[idx]
            : "field" + std::to_string(idx));
        std::string_view fieldKey(fieldName);

        // Save raw Date/Time fields
        if (fieldName == "Date") {
            dateStr.assign(token);
        } else if (fieldName == "Time") {
            timeStr.assign(token);
        }

        // Convert to integer if in intFields
        if (intFields.count(fieldKey)) {
            try {
                long long v = std::stoll(std::string(token));
                doc.AddMember(
                    rapidjson::Value(fieldKey.data(), alloc),
                    rapidjson::Value(v),
                    alloc
                );
            } catch (...) {
                // Fallback to string on parse error
                doc.AddMember(
                    rapidjson::Value(fieldKey.data(), alloc),
                    rapidjson::Value(token.data(), alloc),
                    alloc
                );
            }
        }
        // Convert to double if in floatFields
        else if (floatFields.count(fieldKey)) {
            try {
                double v = std::stod(std::string(token));
                doc.AddMember(
                    rapidjson::Value(fieldKey.data(), alloc),
                    rapidjson::Value(v),
                    alloc
                );
            } catch (...) {
                doc.AddMember(
                    rapidjson::Value(fieldKey.data(), alloc),
                    rapidjson::Value(token.data(), alloc),
                    alloc
                );
            }
        }
        // Otherwise leave as string
        else {
            doc.AddMember(
                rapidjson::Value(fieldKey.data(), alloc),
                rapidjson::Value(token.data(), alloc),
                alloc
            );
        }

        ++idx;
        start = end + 1;
    }

    if (idx == 0) {
        std::cerr << "MessageDecoder: no tokens in line\n";
        return false;
    }

    // Merge Date+Time into an ISO-8601 timestamp field
    if (!dateStr.empty() && !timeStr.empty()) {
        std::string iso = dateStr + "T" + timeStr + "Z";
        doc.AddMember(
            "timestamp",
            rapidjson::Value(iso.c_str(), alloc),
            alloc
        );
        // Optionally remove the raw Date/Time members
        doc.RemoveMember("Date");
        doc.RemoveMember("Time");
    }

    return true;
}
