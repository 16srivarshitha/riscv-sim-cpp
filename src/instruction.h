#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>

enum class Opcode : uint8_t {
    LUI     = 0x37,
    AUIPC   = 0x17,
    JAL     = 0x6F,
    JALR    = 0x67,
    BRANCH  = 0x63,
    LOAD    = 0x03,
    STORE   = 0x23,
    OP_IMM  = 0x13,
    OP      = 0x33,
    SYSTEM  = 0x73,
    UNKNOWN = 0x00
};

enum class InstructionType {
    R, I, S, B, U, J
};

struct Instruction {
    Opcode opcode;
    InstructionType type;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t funct3;
    uint8_t funct7;
    uint32_t imm;
    
    std::string toString() const;
};

#endif // INSTRUCTION_H