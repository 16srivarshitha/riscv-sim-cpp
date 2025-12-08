# RISCV-SIM-cpp
A C++ implementation of a RISC-V (RV32I) instruction set simulator. This simulator can execute RISC-V machine code and provides debugging capabilities.

## Features

- **RV32I Base Integer Instruction Set** support
- Memory management with configurable size
- Instruction decoder for all RV32I formats (R, I, S, B, U, J)
- Debug mode with register dumps
- Step-through execution mode
- Binary and hex file loading
- Clean, modular architecture

## Supported Instructions

### Arithmetic & Logic
- `ADD`, `SUB`, `AND`, `OR`, `XOR`
- `SLL`, `SRL`, `SRA` (shifts)
- `SLT`, `SLTU` (set less than)
- Immediate variants: `ADDI`, `ANDI`, `ORI`, `XORI`, `SLLI`, `SRLI`, `SRAI`, `SLTI`, `SLTIU`

### Memory Access
- `LB`, `LH`, `LW` (load byte/half/word)
- `LBU`, `LHU` (load unsigned)
- `SB`, `SH`, `SW` (store byte/half/word)

### Control Flow
- `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU` (branches)
- `JAL`, `JALR` (jumps)

### Upper Immediate
- `LUI` (load upper immediate)
- `AUIPC` (add upper immediate to PC)

### System
- `ECALL` (environment call - used to halt simulation)

## Building

### Using Make

```bash
make
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./riscv-sim [options] <binary_file>
```

### Options

- `-h, --help`: Show help message
- `-d, --debug`: Enable debug mode (dump registers after execution)
- `-s, --step`: Step through instructions interactively
- `-m, --memory`: Dump memory contents after execution

### Example

```bash
# Run with debug output
./riscv-sim -d tests/test_program.bin

# Step through execution
./riscv-sim -s tests/test_program.bin

# Run with all debugging features
./riscv-sim -d -m tests/test_program.bin
```

## Creating Test Programs

### Using RISC-V Toolchain

1. Write your assembly code (`.s` file)
2. Assemble and link:

```bash
riscv64-unknown-elf-as -march=rv32i -mabi=ilp32 program.s -o program.o
riscv64-unknown-elf-ld -m elf32lriscv program.o -o program.elf
riscv64-unknown-elf-objcopy -O binary program.elf program.bin
```

3. Run in simulator:

```bash
./riscv-sim program.bin
```

### Assembly Example

```asm
.text
.globl _start

_start:
    # Calculate sum of 1 to 10
    li x10, 0       # sum = 0
    li x11, 1       # i = 1
    li x12, 10      # limit = 10

loop:
    add x10, x10, x11   # sum += i
    addi x11, x11, 1    # i++
    ble x11, x12, loop  # if i <= limit, loop

    # Exit
    ecall
```

## Architecture

### File Structure

```
riscv-simulator/
├── src/
│   ├── main.cpp          # Entry point and CLI
│   ├── cpu.cpp/h         # CPU implementation
│   ├── memory.cpp/h      # Memory management
│   ├── instruction.cpp/h # Instruction representation
│   ├── decoder.cpp/h     # Instruction decoder
│   └── loader.cpp/h      # Binary/hex file loader
├── tests/
│   └── test_program.s    # Example assembly program
├── CMakeLists.txt
├── Makefile
└── README.md
```

### Components

- **CPU**: Manages registers, program counter, and instruction execution
- **Memory**: Provides byte-addressable memory with little-endian encoding
- **Decoder**: Decodes 32-bit instruction words into structured format
- **Instruction**: Represents decoded instructions with all fields
- **Loader**: Loads binary and hex files into memory

## Memory Layout

- Default memory size: 1MB
- Little-endian byte ordering
- Program loaded at address 0x00000000
- PC starts at 0x00000000

## Registers

- 32 general-purpose registers (x0-x31)
- x0 is hardwired to zero
- x10 (a0) typically holds return values
- Standard RISC-V calling convention supported

## Exit and Return Values

Programs should use `ECALL` to halt execution. The simulator reads the return value from register x10 (a0).

## Limitations

- Only RV32I base instruction set (no M, A, F, D extensions)
- No privilege modes (runs in M-mode equivalent)
- No MMU or virtual memory
- No interrupts or exceptions (except halt on ECALL)
- No system calls beyond halt

## Future Enhancements

Potential additions:
- M extension (multiplication/division)
- CSR (Control and Status Register) support
- Compressed instructions (C extension)
- ELF file loading
- GDB remote debugging support
- Performance counters
- Memory-mapped I/O

## License

This project is provided as-is for educational purposes.

## Resources

- [RISC-V Specification](https://riscv.org/technical/specifications/)
- [RISC-V Green Card](https://www.cl.cam.ac.uk/teaching/1617/ECAD+Arch/files/docs/RISCVGreenCardv8-20151013.pdf)
- [RISC-V Assembly Programmer's Manual](https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md)
