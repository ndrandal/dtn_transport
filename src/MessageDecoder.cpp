// File: src/MessageDecoder.cpp
#include "MessageDecoder.h"
#include <rapidjson/document.h>        // rapidjson::Document / Value
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <algorithm>                   // std::replace, std::transform
#include <cctype>                      // std::tolower

// ---------------------------------------------------------------------------
// helper: trim ASCII whitespace from both ends (in‑place)
static inline void trim(std::string& s)
{
    const char* ws = " \t\r\n";
    const auto first = s.find_first_not_of(ws);
    if (first == std::string::npos) { s.clear(); return; }  // all whitespace
    const auto last  = s.find_last_not_of(ws);
    s.erase(last + 1);
    s.erase(0, first);
}

bool MessageDecoder::decode(const std::vector<std::string>& keys,
                            const std::string& csvLine,
                            rapidjson::Document& doc)
{
    if (keys.empty()) {
        std::cerr << "MessageDecoder: empty schema\n";
        return false;
    }

    // ---- canonicaliser: "Order Size" → "order-size"
    auto canonical = [](std::string s) {
        std::replace(s.begin(), s.end(), ' ', '-');
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    };

    // ---- numeric field hints (lower‑case, dashes only!)
    static const std::unordered_set<std::string> intFields = {
        "most-recent-trade-size", "total-volume",
        "bid-size", "ask-size", "order-size", "level-size"
    };
    static const std::unordered_set<std::string> floatFields = {
        "most-recent-trade", "bid", "ask", "open",
        "high", "low", "close", "price"
    };

    // ----------------------------------------------------------------------
    doc.SetObject();
    auto& alloc = doc.GetAllocator();
    std::string dateStr, timeStr;

    // --- CSV split (preserve empty fields) ---------------------------------
    std::vector<std::string> tokens;
    size_t start = 0;
    while (true) {
        size_t end = csvLine.find(',', start);
        tokens.emplace_back(csvLine.substr(
            start,
            end == std::string::npos ? end : end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }

    // --- message‑type specific padding (order‑remove = '5')
    if (!tokens.empty() && tokens[0] == "5") {
        const std::vector<size_t> omit = {3,5,6,7,8,9,10};
        for (size_t i = 0; i < omit.size(); ++i) {
            size_t pos = omit[i];
            if (pos <= tokens.size())
                tokens.insert(tokens.begin() + pos, std::string());
        }
    }

    // --- main field loop ---------------------------------------------------
    for (size_t idx = 0; idx < keys.size(); ++idx) {
        std::string token = (idx < tokens.size() ? tokens[idx] : "");
        trim(token);

        const std::string& fieldName = keys[idx];
        const std::string  canon     = canonical(fieldName);

        if (fieldName == "Date") dateStr = token;
        if (fieldName == "Time") timeStr = token;

        rapidjson::Value key(fieldName.c_str(), alloc);  // JSON key

        // empty → JSON null
        if (token.empty()) {
            doc.AddMember(key, rapidjson::Value(rapidjson::kNullType), alloc);
            continue;
        }

        try {
            if (intFields.count(canon)) {                // integer
                long long v = std::stoll(token);
                doc.AddMember(key, rapidjson::Value(v), alloc);
            } else if (floatFields.count(canon)) {       // floating‑point
                double v = std::stod(token);
                doc.AddMember(key, rapidjson::Value(v), alloc);
            } else {                                     // plain string
                doc.AddMember(key,
                              rapidjson::Value(token.c_str(), alloc),
                              alloc);
            }
        }
        catch (...) {                                    // fallback to string
            doc.AddMember(key,
                          rapidjson::Value(token.c_str(), alloc),
                          alloc);
        }
    }

    // --- merge Date + Time into ISO‑8601 timestamp -------------------------
    if (!dateStr.empty() && !timeStr.empty()) {
        std::string iso = dateStr + "T" + timeStr + "Z";
        doc.AddMember(rapidjson::Value("timestamp", alloc),
                      rapidjson::Value(iso.c_str(), alloc),
                      alloc);
        doc.RemoveMember("Date");
        doc.RemoveMember("Time");
    }

    return true;   // decode succeeded
}
