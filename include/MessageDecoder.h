// File: include/MessageDecoder.h
#pragma once

#include <rapidjson/document.h>
#include <string>

/// Parses a CSV tick line into a RapidJSON document using SchemaLoader::fields().
class MessageDecoder {
public:
    /// Returns true on successful parse, false otherwise.
    static bool decode(const std::string& csvLine, rapidjson::Document& doc);
};
