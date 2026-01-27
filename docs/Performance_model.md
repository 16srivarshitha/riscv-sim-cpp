# Performance Modeling

This document explains the cycle-accurate timing model used by the simulator.

## Overview

Unlike a purely functional simulator, this implementation models realistic instruction execution timing. Each instruction type has an associated cycle cost that approximates hardware behavior.

## Cycle Costs

### ALU Operations (1 Cycle)

**Instructions**: ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT, SLTU, LUI, AUIPC  
**Immediate variants**: ADDI, ANDI, ORI, XORI, SLLI, SRLI, SRAI, SLTI, SLTIU

**Rationale**: Simple arithmetic and logic operations complete in a single cycle in most RISC processors. The ALU can compute results within one clock period.

**Examples**:
```asm
ADD  x10, x11, x12    # 1 cycle
ADDI x10, x11, 100    # 1 cycle
SLL  x10, x11, x12    # 1 cycle
```

### Memory Operations (2 Cycles)

**Load Instructions**: LB, LH, LW, LBU, LHU  
**Store Instructions**: SB, SH, SW

**Breakdown**:
- Cycle 1: Address calculation (base + offset)
- Cycle 2: Memory access (read or write)

**Rationale**: Memory operations require two steps: computing the effective address and then accessing memory. Even with a perfect cache, address generation and data transfer take time.

**Examples**:
```asm
LW  x10, 0(x11)      # 2 cycles (compute address, read memory)
SW  x10, 4(x11)      # 2 cycles (compute address, write memory)
LB  x10, 100(x11)    # 2 cycles
```

### Branch Operations (1 or 3 Cycles)

**Instructions**: BEQ, BNE, BLT, BGE, BLTU, BGEU

**Branch Not Taken**: 1 cycle  
**Branch Taken**: 3 cycles

**Rationale**: 
- Not taken: Instruction continues sequentially (PC+4), minimal penalty
- Taken: Pipeline must flush and refetch from target address, causing a 3-cycle penalty

**Examples**:
```asm
BEQ x10, x11, target   # 1 cycle if x10 != x11 (not taken)
                       # 3 cycles if x10 == x11 (taken, pipeline flush)
```

### Jump Operations (3 Cycles)

**Instructions**: JAL, JALR

**Rationale**: Jumps always change control flow, requiring pipeline flush and refetch from the target address. The 3-cycle penalty reflects this disruption.

**Examples**:
```asm
JAL  x1, function      # 3 cycles (always)
JALR x0, 0(x1)         # 3 cycles (return from function)
```

### System Instructions (Variable)

**Instructions**: ECALL, EBREAK

**Cost**: Not counted (simulation halts)

**Rationale**: System calls transition to supervisor mode. Simulation ends on ECALL.

## Calculating Program Performance

### Total Cycles

Total cycles = Σ(instruction_count × cycles_per_instruction)

For a program with:
- 50 ALU operations (1 cycle each)
- 20 loads (2 cycles each)
- 10 stores (2 cycles each)
- 15 branches (10 not taken, 5 taken)
- 5 jumps (3 cycles each)

Total cycles = 50×1 + 20×2 + 10×2 + 10×1 + 5×3 + 5×3  
             = 50 + 40 + 20 + 10 + 15 + 15  
             = **150 cycles**

### Instructions Per Cycle (IPC)

IPC = Total Instructions / Total Cycles

For the example above:
- Total instructions = 100
- Total cycles = 150
- IPC = 100 / 150 = **0.67**

**Interpretation**:
- IPC = 1.0: Perfect efficiency (one instruction per cycle)
- IPC < 1.0: Stalls and penalties reduce throughput
- IPC > 1.0: Not possible in this single-issue model

### Cycles Per Instruction (CPI)

CPI = Total Cycles / Total Instructions = 1 / IPC

For the example: CPI = 150 / 100 = **1.5**

## Performance Analysis Example

### Fibonacci Computation

**Program**:
```asm
    li x10, 0           # 1 cycle
    li x11, 1           # 1 cycle
    li x12, 10          # 1 cycle
loop:
    add x15, x10, x11   # 1 cycle
    mv x10, x11         # 1 cycle (pseudo-op: ADDI)
    mv x11, x15         # 1 cycle
    addi x13, x13, 1    # 1 cycle
    blt x13, x12, loop  # 1 cycle (not taken) or 3 cycles (taken)
```

**Analysis for 10 iterations**:
- Setup: 3 cycles (3 × LI)
- Per iteration:
  - ADD: 1 cycle
  - MV: 1 cycle
  - MV: 1 cycle
  - ADDI: 1 cycle
  - BLT (taken): 3 cycles for iterations 1-9
  - BLT (not taken): 1 cycle for iteration 10
- Total per iteration (first 9): 7 cycles
- Last iteration: 5 cycles (branch not taken)

**Total cycles**: 3 + 9×7 + 5 = **71 cycles**  
**Total instructions**: 3 + 10×5 = **53 instructions**  
**IPC**: 53 / 71 = **0.75**

### Memory-Intensive Program

**Program** (sum array of 10 elements):
```asm
    li x10, 0          # sum = 0
    li x11, 0x1000     # array address
    li x12, 10         # count
loop:
    lw x13, 0(x11)     # load element (2 cycles)
    add x10, x10, x13  # add to sum (1 cycle)
    addi x11, x11, 4   # increment pointer (1 cycle)
    addi x12, x12, -1  # decrement counter (1 cycle)
    bnez x12, loop     # check condition (1/3 cycles)
```

**Analysis**:
- Setup: 3 cycles
- Per iteration (first 9): 2 + 1 + 1 + 1 + 3 = 8 cycles
- Last iteration: 2 + 1 + 1 + 1 + 1 = 6 cycles
- Total: 3 + 9×8 + 6 = **81 cycles**
- Instructions: 3 + 10×5 = **53 instructions**
- IPC: 53 / 81 = **0.65**

**Observation**: Memory-intensive code has lower IPC due to 2-cycle loads.

## Optimization Strategies

### 1. Reduce Memory Accesses

**Before** (IPC ≈ 0.60):
```asm
loop:
    lw x10, 0(x11)     # 2 cycles
    lw x12, 4(x11)     # 2 cycles
    add x13, x10, x12  # 1 cycle
    sw x13, 8(x11)     # 2 cycles
```

**After** (IPC ≈ 0.75):
```asm
    lw x10, 0(x11)     # 2 cycles (load once)
    lw x12, 4(x11)     # 2 cycles (load once)
loop:
    add x13, x10, x12  # 1 cycle
    mv x10, x12        # 1 cycle (keep in registers)
    lw x12, 4(x11)     # 2 cycles
```

### 2. Loop Unrolling

**Before**:
```asm
loop:
    lw x10, 0(x11)     # 2 cycles
    add x12, x12, x10  # 1 cycle
    addi x11, x11, 4   # 1 cycle
    bne x11, x13, loop # 1/3 cycles
# 10 iterations = 10×(2+1+1+3) = 70 cycles
```

**After** (2x unroll):
```asm
loop:
    lw x10, 0(x11)     # 2 cycles
    add x12, x12, x10  # 1 cycle
    lw x10, 4(x11)     # 2 cycles
    add x12, x12, x10  # 1 cycle
    addi x11, x11, 8   # 1 cycle
    bne x11, x13, loop # 1/3 cycles
# 5 iterations = 5×(2+1+2+1+1+3) = 50 cycles
```

**Improvement**: 70 cycles → 50 cycles (29% faster)

### 3. Minimize Branches

**Before**:
```asm
    beq x10, x0, skip  # 1/3 cycles
    add x11, x11, x12  # 1 cycle
skip:
```

**After** (branchless):
```asm
    sltu x13, x0, x10  # 1 cycle (x13 = x10 != 0)
    and x14, x11, x13  # 1 cycle
    add x11, x14, x12  # 1 cycle
```

**Trade-off**: More instructions but no branch penalties

## Performance Counters

The simulator tracks the following metrics:

| Metric | Description |
|--------|-------------|
| `inst_count` | Total instructions executed |
| `cycle_count` | Total cycles elapsed |
| `alu_ops` | ALU/immediate operations |
| `loads` | Load instructions |
| `stores` | Store instructions |
| `branches` | Branch instructions (taken + not taken) |
| `jumps` | JAL/JALR instructions |

**Output Example**:
```
========== Performance Summary ==========
Total Instructions: 86
ALU Ops        : 45        (52.33%)
Loads          : 12        (13.95%)
Stores         : 10        (11.63%)
Branches       : 15        (17.44%)
Jumps          : 4         (4.65%)
Total cycles: 142
IPC: 0.61
==========================================
```

## Comparison with Real Hardware

### This Simulator vs Real RISC-V Core

| Feature | Simulator | Real Hardware (in-order) |
|---------|-----------|-------------------------|
| Pipeline stages | Simplified (F-D-E) | 5-stage (F-D-E-M-W) |
| Branch prediction | None (always penalty) | 2-bit predictor (90%+ accuracy) |
| Data forwarding | N/A | Full forwarding paths |
| Structural hazards | None | Possible (resource conflicts) |
| Cache effects | None (perfect memory) | L1/L2 cache misses |
| IPC (typical) | 0.5-0.8 | 0.7-1.0 |

**Key Differences**:
- Real hardware has branch predictors that reduce penalties
- Real hardware has forwarding to eliminate some hazards
- Real hardware suffers from cache misses (10-100+ cycles)
- This simulator is pessimistic for branches, optimistic for memory

## Limitations

1. **No pipeline hazards**: Real hardware may stall due to data/structural hazards
2. **Perfect branch prediction**: Assumes all branches mispredict (worst case)
3. **Perfect memory**: No cache misses or DRAM latency
4. **No instruction cache**: All fetches assumed to hit
5. **Single-issue**: Cannot execute multiple instructions per cycle

Despite these simplifications, the model provides useful insights into:
- Relative performance of different code sequences
- Impact of memory access patterns
- Cost of control flow changes
- Instruction mix characteristics

## Future Enhancements

Potential improvements to the performance model:
- Branch prediction simulation (2-bit saturating counters)
- Cache hierarchy simulation (L1/L2 hit/miss modeling)
- Multi-cycle multiplication/division (M extension)
- Pipeline visualization showing stalls
- Power consumption estimation