// File: include/SchemaLoader.h
#pragma once

#include <string>
#include <vector>
#include <map>

/// Loads CSV header files into named schemas.
class SchemaLoader {
public:
    /// Load a CSV header (single row) from filePath into schema "id".
    static bool load(const std::string& id, const std::string& filePath);

    /// Retrieve the field list for the named schema.
    static const std::vector<std::string>& fields(const std::string& id);

private:
    /// Storage for all loaded schemas
    static std::map<std::string, std::vector<std::string>> schemas_;
};
