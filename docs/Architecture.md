# Architecture Overview

This document describes the internal architecture of the RISC-V simulator.

## High-Level Design
```
┌────────────────────────────────────────────────────────────
│                      RISC-V Simulator                     │
├───────────────────────────────────────────────────────────┤
│                                                           │
│  ┌──────────┐      ┌──────────┐      ┌──────────┐         │
│  │  Loader  │─────>│  Memory  │<────>│   CPU    │         │
│  │  Module  │      │  (1 MB)  │      │          │         │
│  └──────────┘      └──────────┘      └────┬─────┘         │
│                                           │               │
│                                      ┌─────V─────┐        │
│                                      │  Decoder  │        │
│                                      └───────────┘        │
│                                                           │
└───────────────────────────────────────────────────────────
```

## Component Descriptions

### Memory (`memory.cpp/h`)

**Purpose**: Provides byte-addressable memory with little-endian encoding

**Key Features**:
- Configurable size (default: 1 MB)
- Byte, halfword, and word access methods
- Little-endian byte ordering
- Bounds checking for memory safety

**Interface**:
```cpp
uint8_t readByte(uint32_t addr);
uint16_t readHalfWord(uint32_t addr);
uint32_t readWord(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void writeHalfWord(uint32_t addr, uint16_t value);
void writeWord(uint32_t addr, uint32_t value);
```

### CPU (`cpu.cpp/h`)

**Purpose**: Central processing unit managing registers, PC, and instruction execution

**State**:
- 32 general-purpose registers (x0-x31)
- Program Counter (PC)
- Halt flag
- Performance counters (instructions, cycles)
- Instruction statistics

**Execution Model**:
- Fetch instruction from memory at PC
- Decode instruction
- Execute operation
- Update PC and statistics
- Repeat until halt

**Performance Tracking**:
- Instruction count
- Cycle count (with instruction-specific latencies)
- Instruction type breakdown (ALU, loads, stores, branches, jumps)

### Decoder (`decoder.cpp/h`)

**Purpose**: Decodes 32-bit instruction words into structured format

**Supported Formats**:
- **R-type**: Register-register operations (ADD, SUB, AND, OR, XOR, shifts, comparisons)
- **I-type**: Immediate operations and loads (ADDI, LW, LB, JALR)
- **S-type**: Store operations (SW, SH, SB)
- **B-type**: Branch operations (BEQ, BNE, BLT, BGE, BLTU, BGEU)
- **U-type**: Upper immediate operations (LUI, AUIPC)
- **J-type**: Jump operations (JAL)

**Decoding Process**:
1. Extract opcode (bits 0-6)
2. Extract register fields based on format
3. Extract and sign-extend immediate values
4. Return structured Instruction object

### Instruction (`instruction.cpp/h`)

**Purpose**: Data structure representing a decoded instruction

**Fields**:
```cpp
Opcode opcode;      // Operation type
uint8_t rd;         // Destination register
uint8_t rs1;        // Source register 1
uint8_t rs2;        // Source register 2
uint8_t funct3;     // Function code (3-bit)
uint8_t funct7;     // Function code (7-bit)
uint32_t imm;       // Immediate value
```

### Loader (`loader.cpp/h`)

**Purpose**: Loads binary programs into memory

**Capabilities**:
- Binary file loading (`.bin`)
- Hex file loading (`.hex`)
- Validates file existence and size
- Loads starting at address 0x0

## Execution Flow
```
1. Load Program
   ├─> Loader reads binary file
   └─> Memory populated at address 0x0

2. Initialize CPU
   ├─> Registers cleared (x0-x31 = 0)
   ├─> PC set to 0x0
   └─> Statistics counters reset

3. Fetch-Decode-Execute Loop
   │
   ├─> Fetch: Read 32-bit word from memory[PC]
   │
   ├─> Decode: Parse instruction format and fields
   │
   ├─> Execute: Perform operation based on opcode
   │   ├─> ALU: Arithmetic/logic operations
   │   ├─> Load: Read from memory
   │   ├─> Store: Write to memory
   │   ├─> Branch: Conditional PC update
   │   ├─> Jump: Unconditional PC update
   │   └─> System: Handle ECALL (halt)
   │
   ├─> Update PC (PC+4 or branch target)
   │
   ├─> Update statistics (instructions, cycles, type)
   │
   └─> Repeat until ECALL

4. Cleanup
   └─> Print statistics and register dump (if enabled)
```

## Register File

The CPU maintains 32 general-purpose registers following the RISC-V standard:

| Register | ABI Name | Usage | Saved by |
|----------|----------|-------|----------|
| x0 | zero | Hardwired zero | - |
| x1 | ra | Return address | Caller |
| x2 | sp | Stack pointer | Callee |
| x3 | gp | Global pointer | - |
| x4 | tp | Thread pointer | - |
| x5-x7 | t0-t2 | Temporaries | Caller |
| x8 | s0/fp | Saved/Frame pointer | Callee |
| x9 | s1 | Saved register | Callee |
| x10-x11 | a0-a1 | Arguments/Return values | Caller |
| x12-x17 | a2-a7 | Arguments | Caller |
| x18-x27 | s2-s11 | Saved registers | Callee |
| x28-x31 | t3-t6 | Temporaries | Caller |

**Note**: x0 is always hardwired to 0. Writes to x0 are ignored.

## Memory Layout
```
0x00000000  ┌─────────────────────┐
            │   Program Code      │
            │   and Data          │
            │                     │
            ├─────────────────────┤
            │   Available RAM     │
            │                     │
            │                     │
0x000FFFFF  └─────────────────────┘
```

- **Size**: 1 MB (configurable in memory.cpp)
- **Addressing**: Byte-addressable
- **Alignment**: No alignment requirements enforced
- **Endianness**: Little-endian

## Design Decisions

### Why Cycle-Accurate Timing?

Traditional functional simulators execute instructions instantly. This simulator models realistic hardware timing to:
- Analyze performance characteristics
- Compare different code implementations
- Teach microarchitecture concepts
- Identify performance bottlenecks

### Simplified Pipeline

This simulator uses a simplified execution model rather than a full 5-stage pipeline:
- Easier to understand and visualize
- Sufficient for performance analysis
- Avoids complexity of hazard detection and forwarding
- Models aggregate cycle costs rather than per-stage timing

### Instruction-Specific Latencies

Different operations have different cycle costs:
- **ALU (1 cycle)**: Simple operations complete quickly
- **Memory (2 cycles)**: Address calculation + memory access
- **Branches/Jumps (1-3 cycles)**: Models pipeline flush penalty

These values approximate real hardware behavior while remaining simple.

## Extending the Simulator

### Adding New Instructions (M Extension example)

1. **Add opcodes** to `instruction.h`:
```cpp
enum class Opcode {
    // ... existing opcodes ...
    OP_M = 0x33,  // M extension uses same opcode as OP
};
```

2. **Update decoder** in `decoder.cpp`:
```cpp
case 0x33:
    if (funct7 == 0x01) {
        // M extension instructions
        return inst; // MUL, DIV, etc.
    }
    // Regular OP instructions
```

3. **Implement execution** in `cpu.cpp`:
```cpp
void CPU::execMUL(const Instruction& inst) {
    uint32_t rs1_val = getRegister(inst.rs1);
    uint32_t rs2_val = getRegister(inst.rs2);
    uint32_t result = rs1_val * rs2_val;
    setRegister(inst.rd, result);
    pc += 4;
    cycle_count += 3; // Multiplication takes longer
}
```

4. **Update disassembly** for visualization

### Adding CSR Support

1. Add CSR register array to CPU
2. Implement CSRRW, CSRRS, CSRRC instructions
3. Update system call handling
4. Add CSR-specific cycle costs

## Performance Characteristics

Typical instruction mix for common programs:

| Program Type | ALU% | Load% | Store% | Branch% | Jump% |
|--------------|------|-------|--------|---------|-------|
| Arithmetic   | 70   | 10    | 5      | 10      | 5     |
| Array Access | 40   | 30    | 20     | 5       | 5     |
| Loops        | 50   | 15    | 10     | 20      | 5     |
| Function Calls | 45 | 20    | 15     | 10      | 10    |

Expected IPC (Instructions Per Cycle):
- Simple arithmetic: 0.8-0.9
- Memory-intensive: 0.5-0.6
- Branch-heavy: 0.4-0.5