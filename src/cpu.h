#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include "memory.h"
#include "instruction.h"

class CPU {
public:
    CPU(Memory& mem);
    
    void reset();
    void step();
    void run();
    
    // Register access
    uint32_t getRegister(int reg) const;
    void setRegister(int reg, uint32_t value);
    uint32_t getPC() const { return pc; }
    void setPC(uint32_t value) { pc = value; }
    
    // Status
    bool isHalted() const { return halted; }
    void halt() { halted = true; }
    
    // Debugging
    void dumpRegisters() const;
    uint64_t getInstructionCount() const { return inst_count; }
    void printPerformanceStats() const;
    //cycle count
    uint64_t getCycleCount() const { return cycle_count; }

    
    

private:
    struct InstStats {
    uint64_t alu_ops = 0;
    uint64_t loads = 0;
    uint64_t stores = 0;
    uint64_t branches = 0;
    uint64_t jumps = 0;
    uint64_t system = 0;
    } stats;
    std::array<uint32_t, 32> registers;
    Memory& memory;
    uint32_t pc;
    bool halted;
    uint64_t inst_count;
    uint64_t cycle_count = 0;
    
    void executeInstruction(const Instruction& inst);
    
    // Instruction execution helpers
    void execALU(const Instruction& inst);
    void execLoad(const Instruction& inst);
    void execStore(const Instruction& inst);
    void execBranch(const Instruction& inst);
    void execJAL(const Instruction& inst);
    void execJALR(const Instruction& inst);
    void execLUI(const Instruction& inst);
    void execAUIPC(const Instruction& inst);
    void execSystem(const Instruction& inst);
};

#endif // CPU_H