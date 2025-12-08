#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
#include <cstdint>

class Loader {
public:
    static std::vector<uint8_t> loadBinary(const std::string& filename);
    static std::vector<uint8_t> loadHex(const std::string& filename);
};

#endif // LOADER_H