// File: src/SchemaLoader.cpp
#include "SchemaLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::string> SchemaLoader::fieldNames_;

bool SchemaLoader::load(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cerr << "SchemaLoader: cannot open schema file: " << filePath << "\n";
        return false;
    }

    std::string line;
    if (!std::getline(in, line)) {
        std::cerr << "SchemaLoader: empty schema file\n";
        return false;
    }

    std::istringstream ss(line);
    std::string token;
    fieldNames_.clear();
    while (std::getline(ss, token, ',')) {
        fieldNames_.push_back(token);
    }

    if (fieldNames_.empty()) {
        std::cerr << "SchemaLoader: no fields parsed\n";
        return false;
    }
    std::cout << "SchemaLoader: loaded " << fieldNames_.size() << " fields\n";
    return true;
}

const std::vector<std::string>& SchemaLoader::fields() {
    return fieldNames_;
}
