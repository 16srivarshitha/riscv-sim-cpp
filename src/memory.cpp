#include "memory.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

Memory::Memory(size_t size) : data(size, 0) {}

void Memory::checkAddress(uint32_t addr, size_t access_size) const {
    if (addr + access_size > data.size()) {
        throw std::runtime_error("Memory access out of bounds");
    }
}

uint8_t Memory::readByte(uint32_t addr) const {
    checkAddress(addr, 1);
    return data[addr];
}

uint16_t Memory::readHalfWord(uint32_t addr) const {
    checkAddress(addr, 2);
    return static_cast<uint16_t>(data[addr]) | 
           (static_cast<uint16_t>(data[addr + 1]) << 8);
}

uint32_t Memory::readWord(uint32_t addr) const {
    checkAddress(addr, 4);
    return static_cast<uint32_t>(data[addr]) |
           (static_cast<uint32_t>(data[addr + 1]) << 8) |
           (static_cast<uint32_t>(data[addr + 2]) << 16) |
           (static_cast<uint32_t>(data[addr + 3]) << 24);
}

void Memory::writeByte(uint32_t addr, uint8_t value) {
    checkAddress(addr, 1);
    data[addr] = value;
}

void Memory::writeHalfWord(uint32_t addr, uint16_t value) {
    checkAddress(addr, 2);
    data[addr] = value & 0xFF;
    data[addr + 1] = (value >> 8) & 0xFF;
}

void Memory::writeWord(uint32_t addr, uint32_t value) {
    checkAddress(addr, 4);
    data[addr] = value & 0xFF;
    data[addr + 1] = (value >> 8) & 0xFF;
    data[addr + 2] = (value >> 16) & 0xFF;
    data[addr + 3] = (value >> 24) & 0xFF;
}

void Memory::loadProgram(const std::vector<uint8_t>& program, uint32_t start_addr) {
    if (start_addr + program.size() > data.size()) {
        throw std::runtime_error("Program too large for memory");
    }
    
    std::copy(program.begin(), program.end(), data.begin() + start_addr);
}

void Memory::dump(uint32_t start, uint32_t length) const {
    std::cout << "Memory Dump (0x" << std::hex << start << " - 0x" 
              << (start + length) << "):" << std::endl;
    
    for (uint32_t i = 0; i < length; i++) {
        if (i % 16 == 0) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') 
                      << (start + i) << ": ";
        }
        
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(data[start + i]) << " ";
        
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;
}