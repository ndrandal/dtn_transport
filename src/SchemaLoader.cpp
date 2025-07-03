// File: src/SchemaLoader.cpp
#include "SchemaLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// Helper: trim whitespace and CR/LF from both ends
static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? std::string() : s.substr(start, end - start + 1);
}

// Static member definition
std::map<std::string, std::vector<std::string>> SchemaLoader::schemas_;

bool SchemaLoader::load(const std::string& id, const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cerr << "SchemaLoader: cannot open schema file '"
                  << filePath << "' for id '" << id << "'\n";
        return false;
    }
    std::string line;
    if (!std::getline(in, line)) {
        std::cerr << "SchemaLoader: empty schema file for id '"
                  << id << "'\n";
        return false;
    }
    // Remove BOM if present
    if (!line.empty() && static_cast<unsigned char>(line[0]) == 0xEF) {
        line.erase(0, 3);
    }

    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> fields;
    while (std::getline(ss, token, ',')) {
        auto t = trim(token);
        fields.push_back(std::move(t));
    }
    if (fields.empty()) {
        std::cerr << "SchemaLoader: no fields parsed for id '"
                  << id << "'\n";
        return false;
    }
    schemas_[id] = std::move(fields);
    std::cout << "SchemaLoader: loaded " << schemas_[id].size()
              << " fields for '" << id << "'\n";
    return true;
}

const std::vector<std::string>& SchemaLoader::fields(const std::string& id) {
    return schemas_.at(id);
}
