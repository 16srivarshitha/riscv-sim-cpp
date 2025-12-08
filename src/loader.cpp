#include "loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

std::vector<uint8_t> Loader::loadBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return data;
}

std::vector<uint8_t> Loader::loadHex(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::vector<uint8_t> data;
    std::string line;
    
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Parse hex values
        for (size_t i = 0; i < line.length(); i += 2) {
            if (i + 1 < line.length()) {
                std::string byte_str = line.substr(i, 2);
                uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
                data.push_back(byte);
            }
        }
    }
    
    return data;
}