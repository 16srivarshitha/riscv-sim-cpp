#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <vector>

class Memory {
public:
    Memory(size_t size = 1024 * 1024); // Default 1MB
    
    uint8_t readByte(uint32_t addr) const;
    uint16_t readHalfWord(uint32_t addr) const;
    uint32_t readWord(uint32_t addr) const;
    
    void writeByte(uint32_t addr, uint8_t value);
    void writeHalfWord(uint32_t addr, uint16_t value);
    void writeWord(uint32_t addr, uint32_t value);
    
    void loadProgram(const std::vector<uint8_t>& program, uint32_t start_addr = 0);
    void dump(uint32_t start, uint32_t length) const;
    
private:
    std::vector<uint8_t> data;
    
    void checkAddress(uint32_t addr, size_t access_size) const;
};

#endif // MEMORY_H