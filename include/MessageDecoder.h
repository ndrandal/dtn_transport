#pragma once
#include <rapidjson/document.h>
#include <string>

class MessageDecoder {
public:
    // Parses a CSV tick line into a RapidJSON document.
    // Returns true if successful, false otherwise.
    static bool decode(const std::string& csvLine, rapidjson::Document& doc);
};

