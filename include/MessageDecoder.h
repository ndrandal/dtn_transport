// File: include/MessageDecoder.h
#pragma once

#include <rapidjson/document.h>
#include <string>
#include <vector>

/// Parses CSV lines into properly-typed JSON.
/// Converts known numeric fields, merges Date+Time into ISO timestamp.
class MessageDecoder {
public:
    /// Decode a CSV line (using keys) into a JSON doc.
    /// Returns false on parse error.
    static bool decode(const std::vector<std::string>& keys,
                       const std::string& csvLine,
                       rapidjson::Document& doc);
};
