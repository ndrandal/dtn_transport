// File: src/MessageDecoder.cpp
#include "MessageDecoder.h"
#include "SchemaLoader.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <iostream>
#include <unordered_set>
#include <iomanip>

bool MessageDecoder::decode(const std::vector<std::string>& keys,
                            const std::string& csvLine,
                            rapidjson::Document& doc) {
    if (keys.empty()) {
        std::cerr << "MessageDecoder: empty schema\n";
        return false;
    }

    // Fields that should be treated as integers or floats:
    static const std::unordered_set<std::string> intFields = {
        "Order Size","Total Volume","Bid Size","Ask Size",
        "Order Priority","Level Size","Order Count"
    };
    static const std::unordered_set<std::string> floatFields = {
        "Price","Most Recent Trade","Open","High","Low","Close"
    };

    std::string dateStr, timeStr;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    std::istringstream ss(csvLine);
    std::string token;
    size_t idx = 0;
    while (std::getline(ss, token, ',')) {
        const std::string& field = (idx < keys.size() ? keys[idx] : "field" + std::to_string(idx));

        // capture raw Date/Time for later
        if (field == "Date") {
            dateStr = token;
        } else if (field == "Time") {
            timeStr = token;
        }

        // numeric conversion
        if (intFields.count(field)) {
            try {
                long long v = std::stoll(token);
                doc.AddMember(rapidjson::Value(field.c_str(), alloc),
                              rapidjson::Value(v),
                              alloc);
            } catch (...) {
                doc.AddMember(rapidjson::Value(field.c_str(), alloc),
                              rapidjson::Value(token.c_str(), alloc),
                              alloc);
            }
        }
        else if (floatFields.count(field)) {
            try {
                double v = std::stod(token);
                doc.AddMember(rapidjson::Value(field.c_str(), alloc),
                              rapidjson::Value(v),
                              alloc);
            } catch (...) {
                doc.AddMember(rapidjson::Value(field.c_str(), alloc),
                              rapidjson::Value(token.c_str(), alloc),
                              alloc);
            }
        }
        // leave raw for everything else
        else {
            doc.AddMember(rapidjson::Value(field.c_str(), alloc),
                          rapidjson::Value(token.c_str(), alloc),
                          alloc);
        }
        ++idx;
    }

    if (idx == 0) {
        std::cerr << "MessageDecoder: no tokens in line\n";
        return false;
    }

    // merge Date+Time into ISO-8601 timestamp
    if (!dateStr.empty() && !timeStr.empty()) {
        // assume date is YYYY-MM-DD and time is HH:mm:SS(.ffffff)
        std::string iso = dateStr + "T" + timeStr + "Z";
        doc.AddMember("timestamp",
                      rapidjson::Value(iso.c_str(), alloc),
                      alloc);
        // optionally remove raw Date/Time:
        doc.RemoveMember("Date");
        doc.RemoveMember("Time");
    }

    return true;
}
