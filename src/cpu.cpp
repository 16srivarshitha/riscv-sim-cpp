#include "cpu.h"
#include "decoder.h"
#include <iostream>
#include <iomanip>

CPU::CPU(Memory& mem) : memory(mem), pc(0), halted(false), inst_count(0), cycle_count(0) {
    registers.fill(0);
}

void CPU::reset() {
    registers.fill(0);
    pc = 0;
    halted = false;
    inst_count = 0;
    cycle_count = 0; // Ensure cycles start at 0
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
    
    // Output flow data for Python visualization
    std::cout << "[FLOW] Cycle: " << (cycle_count + 1) 
              << " | PC: 0x" << std::hex << std::setw(8) << std::setfill('0') << pc 
              << " | 0x" << std::hex << std::setw(8) << std::setfill('0') << inst_word 
              << " | " << getDisassembly(inst) << std::dec << std::endl;
    
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
    stats.alu_ops++;
    cycle_count += 1;
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
    stats.loads++;
    cycle_count += 2; //one for address calculation + one for memory access
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
    stats.stores++;
    cycle_count += 2;
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
    cycle_count += 3; // Penalty for changing control flow
    } else {
        pc += 4;
        cycle_count += 1; // Direct path
    }
    stats.branches++;
}

void CPU::execJAL(const Instruction& inst) {
    setRegister(inst.rd, pc + 4);
    pc += inst.imm;
    stats.jumps++;
    cycle_count += 3;
}

void CPU::execJALR(const Instruction& inst) {
    uint32_t target = (getRegister(inst.rs1) + inst.imm) & ~1;
    setRegister(inst.rd, pc + 4);
    pc = target;
    stats.jumps++;
    cycle_count += 3;
}

void CPU::execLUI(const Instruction& inst) {
    setRegister(inst.rd, inst.imm);
    pc += 4;
    cycle_count += 1;
}

void CPU::execAUIPC(const Instruction& inst) {
    setRegister(inst.rd, pc + inst.imm);
    pc += 4;
    cycle_count += 1;
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

void CPU::printPerformanceStats() const {
    std::cout << "\n========== Performance Summary ==========" << std::endl;
    std::cout << "Total Instructions: " << inst_count << std::endl;
    
    auto printStat = [&](std::string name, uint64_t count) {
        double percentage = (inst_count > 0) ? (static_cast<double>(count) / inst_count) * 100.0 : 0;
        std::cout << std::left << std::setw(15) << name << ": " 
                  << std::setw(10) << count << " (" << std::fixed << std::setprecision(2) 
                  << percentage << "%)" << std::endl;
    };

    printStat("ALU Ops", stats.alu_ops);
    printStat("Loads", stats.loads);
    printStat("Stores", stats.stores);
    printStat("Branches", stats.branches);
    printStat("Jumps", stats.jumps);
    printStat("System", stats.system);
    std::cout << "==========================================" << std::endl;
}

std::string CPU::getDisassembly(const Instruction& inst) const {
    std::string result;
    
    switch (inst.opcode) {
        case Opcode::LUI:
            result = "LUI x" + std::to_string(inst.rd) + ", 0x" + 
                     std::to_string(inst.imm >> 12);
            break;
        case Opcode::AUIPC:
            result = "AUIPC x" + std::to_string(inst.rd) + ", 0x" + 
                     std::to_string(inst.imm >> 12);
            break;
        case Opcode::JAL:
            result = "JAL x" + std::to_string(inst.rd) + ", " + 
                     std::to_string(static_cast<int32_t>(inst.imm));
            break;
        case Opcode::JALR:
            result = "JALR x" + std::to_string(inst.rd) + ", x" + 
                     std::to_string(inst.rs1) + ", " + std::to_string(static_cast<int32_t>(inst.imm));
            break;
        case Opcode::BRANCH:
            switch (inst.funct3) {
                case 0x0: result = "BEQ"; break;
                case 0x1: result = "BNE"; break;
                case 0x4: result = "BLT"; break;
                case 0x5: result = "BGE"; break;
                case 0x6: result = "BLTU"; break;
                case 0x7: result = "BGEU"; break;
            }
            result += " x" + std::to_string(inst.rs1) + ", x" + 
                      std::to_string(inst.rs2) + ", " + std::to_string(static_cast<int32_t>(inst.imm));
            break;
        case Opcode::LOAD:
            switch (inst.funct3) {
                case 0x0: result = "LB"; break;
                case 0x1: result = "LH"; break;
                case 0x2: result = "LW"; break;
                case 0x4: result = "LBU"; break;
                case 0x5: result = "LHU"; break;
            }
            result += " x" + std::to_string(inst.rd) + ", " + 
                      std::to_string(static_cast<int32_t>(inst.imm)) + "(x" + 
                      std::to_string(inst.rs1) + ")";
            break;
        case Opcode::STORE:
            switch (inst.funct3) {
                case 0x0: result = "SB"; break;
                case 0x1: result = "SH"; break;
                case 0x2: result = "SW"; break;
            }
            result += " x" + std::to_string(inst.rs2) + ", " + 
                      std::to_string(static_cast<int32_t>(inst.imm)) + "(x" + 
                      std::to_string(inst.rs1) + ")";
            break;
        case Opcode::OP_IMM:
            switch (inst.funct3) {
                case 0x0: result = "ADDI"; break;
                case 0x1: result = "SLLI"; break;
                case 0x2: result = "SLTI"; break;
                case 0x3: result = "SLTIU"; break;
                case 0x4: result = "XORI"; break;
                case 0x5: result = (inst.funct7 == 0x20) ? "SRAI" : "SRLI"; break;
                case 0x6: result = "ORI"; break;
                case 0x7: result = "ANDI"; break;
            }
            result += " x" + std::to_string(inst.rd) + ", x" + 
                      std::to_string(inst.rs1) + ", " + std::to_string(static_cast<int32_t>(inst.imm));
            break;
        case Opcode::OP:
            switch (inst.funct3) {
                case 0x0: result = (inst.funct7 == 0x20) ? "SUB" : "ADD"; break;
                case 0x1: result = "SLL"; break;
                case 0x2: result = "SLT"; break;
                case 0x3: result = "SLTU"; break;
                case 0x4: result = "XOR"; break;
                case 0x5: result = (inst.funct7 == 0x20) ? "SRA" : "SRL"; break;
                case 0x6: result = "OR"; break;
                case 0x7: result = "AND"; break;
            }
            result += " x" + std::to_string(inst.rd) + ", x" + 
                      std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
            break;
        case Opcode::SYSTEM:
            result = "ECALL";
            break;
        default:
            result = "UNKNOWN";
            break;
    }
    
    return result;
}
