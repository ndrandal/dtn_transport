// File: src/SchemaLoader.cpp
#include "SchemaLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Definition of the static member
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
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> fields;
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
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
