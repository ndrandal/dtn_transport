// File: include/SchemaLoader.h
#pragma once

#include <string>
#include <vector>

/// Loads a single-line CSV header file of field names.
class SchemaLoader {
public:
    /// Reads a CSV file where the first row is your field names.
    static bool load(const std::string& filePath);

    /// After load(), holds the ordered list of field names.
    static const std::vector<std::string>& fields();

private:
    static std::vector<std::string> fieldNames_;
};
