#include "decoder.h"

uint32_t Decoder::signExtend(uint32_t value, int bits) {
    if (value & (1 << (bits - 1))) {
        return value | (~0u << bits);
    }
    return value;
}

Instruction Decoder::decode(uint32_t inst_word) {
    uint8_t opcode_raw = inst_word & 0x7F;
    Opcode opcode = static_cast<Opcode>(opcode_raw);
    
    switch (opcode) {
        case Opcode::LUI:
        case Opcode::AUIPC:
            return decodeU(inst_word, opcode);
        case Opcode::JAL:
            return decodeJ(inst_word, opcode);
        case Opcode::JALR:
        case Opcode::LOAD:
        case Opcode::OP_IMM:
        case Opcode::SYSTEM:
            return decodeI(inst_word, opcode);
        case Opcode::BRANCH:
            return decodeB(inst_word, opcode);
        case Opcode::STORE:
            return decodeS(inst_word, opcode);
        case Opcode::OP:
            return decodeR(inst_word, opcode);
        default:
            Instruction inst = {};
            inst.opcode = Opcode::UNKNOWN;
            return inst;
    }
}

Instruction Decoder::decodeR(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::R;
    inst.rd = (inst_word >> 7) & 0x1F;
    inst.funct3 = (inst_word >> 12) & 0x07;
    inst.rs1 = (inst_word >> 15) & 0x1F;
    inst.rs2 = (inst_word >> 20) & 0x1F;
    inst.funct7 = (inst_word >> 25) & 0x7F;
    inst.imm = 0;
    return inst;
}

Instruction Decoder::decodeI(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::I;
    inst.rd = (inst_word >> 7) & 0x1F;
    inst.funct3 = (inst_word >> 12) & 0x07;
    inst.rs1 = (inst_word >> 15) & 0x1F;
    inst.rs2 = 0;
    inst.funct7 = (inst_word >> 25) & 0x7F;
    
    uint32_t imm = (inst_word >> 20) & 0xFFF;
    inst.imm = signExtend(imm, 12);
    return inst;
}

Instruction Decoder::decodeS(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::S;
    inst.rd = 0;
    inst.funct3 = (inst_word >> 12) & 0x07;
    inst.rs1 = (inst_word >> 15) & 0x1F;
    inst.rs2 = (inst_word >> 20) & 0x1F;
    inst.funct7 = 0;
    
    uint32_t imm = ((inst_word >> 7) & 0x1F) | 
                   (((inst_word >> 25) & 0x7F) << 5);
    inst.imm = signExtend(imm, 12);
    return inst;
}

Instruction Decoder::decodeB(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::B;
    inst.rd = 0;
    inst.funct3 = (inst_word >> 12) & 0x07;
    inst.rs1 = (inst_word >> 15) & 0x1F;
    inst.rs2 = (inst_word >> 20) & 0x1F;
    inst.funct7 = 0;
    
    uint32_t imm = (((inst_word >> 8) & 0x0F) << 1) |
                   (((inst_word >> 25) & 0x3F) << 5) |
                   (((inst_word >> 7) & 0x01) << 11) |
                   (((inst_word >> 31) & 0x01) << 12);
    inst.imm = signExtend(imm, 13);
    return inst;
}

Instruction Decoder::decodeU(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::U;
    inst.rd = (inst_word >> 7) & 0x1F;
    inst.funct3 = 0;
    inst.rs1 = 0;
    inst.rs2 = 0;
    inst.funct7 = 0;
    inst.imm = inst_word & 0xFFFFF000;
    return inst;
}

Instruction Decoder::decodeJ(uint32_t inst_word, Opcode opcode) {
    Instruction inst;
    inst.opcode = opcode;
    inst.type = InstructionType::J;
    inst.rd = (inst_word >> 7) & 0x1F;
    inst.funct3 = 0;
    inst.rs1 = 0;
    inst.rs2 = 0;
    inst.funct7 = 0;
    
    uint32_t imm = (((inst_word >> 21) & 0x3FF) << 1) |
                   (((inst_word >> 20) & 0x01) << 11) |
                   (((inst_word >> 12) & 0xFF) << 12) |
                   (((inst_word >> 31) & 0x01) << 20);
    inst.imm = signExtend(imm, 21);
    return inst;
}