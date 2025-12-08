#include "cpu.h"
#include "decoder.h"
#include <iostream>
#include <iomanip>

CPU::CPU(Memory& mem) : memory(mem), pc(0), halted(false), inst_count(0) {
    registers.fill(0);
}

void CPU::reset() {
    registers.fill(0);
    pc = 0;
    halted = false;
    inst_count = 0;
}

uint32_t CPU::getRegister(int reg) const {
    if (reg == 0) return 0;
    return registers[reg];
}

void CPU::setRegister(int reg, uint32_t value) {
    if (reg != 0) {
        registers[reg] = value;
    }
}

void CPU::step() {
    if (halted) return;
    
    uint32_t inst_word = memory.readWord(pc);
    Instruction inst = Decoder::decode(inst_word);
    
    executeInstruction(inst);
    inst_count++;
}

void CPU::run() {
    while (!halted) {
        step();
    }
}

void CPU::executeInstruction(const Instruction& inst) {
    switch (inst.opcode) {
        case Opcode::LUI:
            execLUI(inst);
            break;
        case Opcode::AUIPC:
            execAUIPC(inst);
            break;
        case Opcode::JAL:
            execJAL(inst);
            break;
        case Opcode::JALR:
            execJALR(inst);
            break;
        case Opcode::BRANCH:
            execBranch(inst);
            break;
        case Opcode::LOAD:
            execLoad(inst);
            break;
        case Opcode::STORE:
            execStore(inst);
            break;
        case Opcode::OP_IMM:
        case Opcode::OP:
            execALU(inst);
            break;
        case Opcode::SYSTEM:
            execSystem(inst);
            break;
        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << static_cast<int>(inst.opcode) << std::endl;
            halted = true;
            break;
    }
}

void CPU::execALU(const Instruction& inst) {
    uint32_t rs1_val = getRegister(inst.rs1);
    uint32_t rs2_val = (inst.opcode == Opcode::OP) ? getRegister(inst.rs2) : inst.imm;
    uint32_t result = 0;
    
    switch (inst.funct3) {
        case 0x0: // ADD/ADDI or SUB
            if (inst.opcode == Opcode::OP && inst.funct7 == 0x20) {
                result = rs1_val - rs2_val;
            } else {
                result = rs1_val + rs2_val;
            }
            break;
        case 0x1: // SLL/SLLI
            result = rs1_val << (rs2_val & 0x1F);
            break;
        case 0x2: // SLT/SLTI
            result = (static_cast<int32_t>(rs1_val) < static_cast<int32_t>(rs2_val)) ? 1 : 0;
            break;
        case 0x3: // SLTU/SLTIU
            result = (rs1_val < rs2_val) ? 1 : 0;
            break;
        case 0x4: // XOR/XORI
            result = rs1_val ^ rs2_val;
            break;
        case 0x5: // SRL/SRLI or SRA/SRAI
            if (inst.funct7 == 0x20) {
                result = static_cast<uint32_t>(static_cast<int32_t>(rs1_val) >> (rs2_val & 0x1F));
            } else {
                result = rs1_val >> (rs2_val & 0x1F);
            }
            break;
        case 0x6: // OR/ORI
            result = rs1_val | rs2_val;
            break;
        case 0x7: // AND/ANDI
            result = rs1_val & rs2_val;
            break;
    }
    
    setRegister(inst.rd, result);
    pc += 4;
}

void CPU::execLoad(const Instruction& inst) {
    uint32_t addr = getRegister(inst.rs1) + inst.imm;
    uint32_t value = 0;
    
    switch (inst.funct3) {
        case 0x0: // LB
            value = static_cast<int32_t>(static_cast<int8_t>(memory.readByte(addr)));
            break;
        case 0x1: // LH
            value = static_cast<int32_t>(static_cast<int16_t>(memory.readHalfWord(addr)));
            break;
        case 0x2: // LW
            value = memory.readWord(addr);
            break;
        case 0x4: // LBU
            value = memory.readByte(addr);
            break;
        case 0x5: // LHU
            value = memory.readHalfWord(addr);
            break;
    }
    
    setRegister(inst.rd, value);
    pc += 4;
}

void CPU::execStore(const Instruction& inst) {
    uint32_t addr = getRegister(inst.rs1) + inst.imm;
    uint32_t value = getRegister(inst.rs2);
    
    switch (inst.funct3) {
        case 0x0: // SB
            memory.writeByte(addr, value & 0xFF);
            break;
        case 0x1: // SH
            memory.writeHalfWord(addr, value & 0xFFFF);
            break;
        case 0x2: // SW
            memory.writeWord(addr, value);
            break;
    }
    
    pc += 4;
}

void CPU::execBranch(const Instruction& inst) {
    uint32_t rs1_val = getRegister(inst.rs1);
    uint32_t rs2_val = getRegister(inst.rs2);
    bool take_branch = false;
    
    switch (inst.funct3) {
        case 0x0: // BEQ
            take_branch = (rs1_val == rs2_val);
            break;
        case 0x1: // BNE
            take_branch = (rs1_val != rs2_val);
            break;
        case 0x4: // BLT
            take_branch = (static_cast<int32_t>(rs1_val) < static_cast<int32_t>(rs2_val));
            break;
        case 0x5: // BGE
            take_branch = (static_cast<int32_t>(rs1_val) >= static_cast<int32_t>(rs2_val));
            break;
        case 0x6: // BLTU
            take_branch = (rs1_val < rs2_val);
            break;
        case 0x7: // BGEU
            take_branch = (rs1_val >= rs2_val);
            break;
    }
    
    if (take_branch) {
        pc += inst.imm;
    } else {
        pc += 4;
    }
}

void CPU::execJAL(const Instruction& inst) {
    setRegister(inst.rd, pc + 4);
    pc += inst.imm;
}

void CPU::execJALR(const Instruction& inst) {
    uint32_t target = (getRegister(inst.rs1) + inst.imm) & ~1;
    setRegister(inst.rd, pc + 4);
    pc = target;
}

void CPU::execLUI(const Instruction& inst) {
    setRegister(inst.rd, inst.imm);
    pc += 4;
}

void CPU::execAUIPC(const Instruction& inst) {
    setRegister(inst.rd, pc + inst.imm);
    pc += 4;
}

void CPU::execSystem(const Instruction& inst) {
    if (inst.funct3 == 0 && inst.imm == 0) { // ECALL
        halted = true;
    }
    // Handle other system instructions as needed
    pc += 4;
}

void CPU::dumpRegisters() const {
    std::cout << "Register Dump:" << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << std::dec << std::setw(2) << i << " = 0x" 
                  << std::hex << std::setw(8) << std::setfill('0') << registers[i];
        if ((i + 1) % 4 == 0) std::cout << std::endl;
        else std::cout << "  ";
    }
    std::cout << "PC  = 0x" << std::hex << std::setw(8) << std::setfill('0') << pc << std::endl;
    std::cout << std::dec << "Instructions executed: " << inst_count << std::endl;
}