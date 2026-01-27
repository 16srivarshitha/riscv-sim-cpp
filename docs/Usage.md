# Usage Guide

Comprehensive guide to using the RISC-V simulator.

## Building

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.10+
- Make (optional, for Makefile build)
- RISC-V GNU Toolchain (for creating test programs)
- Python 3.6+ with Graphviz (for flow visualization)

### Install Dependencies

**Ubuntu/Debian**:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo apt-get install -y gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf
sudo apt-get install -y python3 python3-pip graphviz
pip3 install graphviz
```

**macOS** (with Homebrew):
```bash
brew install cmake
brew install riscv-gnu-toolchain
brew install python graphviz
pip3 install graphviz
```

### Build with CMake (Recommended)
```bash
# Configure and build
cmake -B build -S .
cmake --build build

# The executable will be at: build/riscv-sim
```

### Build with Make
```bash
# Build
make

# Clean
make clean

# The executable will be at: riscv-sim
```

## Running Programs

### Basic Execution
```bash
./build/riscv-sim tests/test_program.bin
```

### Command-Line Options
```
Usage: ./riscv-sim [options] <binary_file>

Options:
  -h, --help        Show help message
  -d, --debug       Enable debug mode (dump registers after execution)
  -s, --step        Step through instructions interactively
  -m, --memory      Dump memory contents after execution
```

### Examples

**Run with debug output**:
```bash
./build/riscv-sim -d tests/test_program.bin
```

**Step through execution**:
```bash
./build/riscv-sim -s tests/test_program.bin
```
Press Enter after each instruction to execute the next one.

**Run with all debugging features**:
```bash
./build/riscv-sim -d -m tests/test_program.bin
```

**Generate flow visualization**:
```bash
./build/riscv-sim tests/test_program.bin | python3 scripts/generate_flow.py
```

## Creating Programs

### Writing Assembly

Create a `.s` file with RISC-V assembly:
```asm
# example.s - Simple program
.text
.globl _start

_start:
    # Your code here
    li x10, 42        # Load immediate
    addi x11, x10, 8  # Add immediate
    
    # Exit
    li x17, 93
    ecall
```

### Assembling and Linking
```bash
# Assemble
riscv64-unknown-elf-as -march=rv32i -mabi=ilp32 -o example.o example.s

# Link
riscv64-unknown-elf-ld -m elf32lriscv -Ttext=0x0 -o example.elf example.o

# Convert to binary
riscv64-unknown-elf-objcopy -O binary example.elf example.bin
```

### One-Line Build Script
```bash
#!/bin/bash
# build_riscv.sh
riscv64-unknown-elf-as -march=rv32i -mabi=ilp32 -o $1.o $1.s && \
riscv64-unknown-elf-ld -m elf32lriscv -Ttext=0x0 -o $1.elf $1.o && \
riscv64-unknown-elf-objcopy -O binary $1.elf $1.bin && \
echo "Built $1.bin successfully"
```

Usage:
```bash
chmod +x build_riscv.sh
./build_riscv.sh example
```

## Example Programs

### 1. Hello World (Minimal)
```asm
.text
.globl _start

_start:
    li x10, 0         # Return value 0
    li x17, 93        # Exit syscall
    ecall
```

### 2. Sum of Numbers
```asm
.text
.globl _start

_start:
    li x10, 0         # sum = 0
    li x11, 1         # i = 1
    li x12, 10        # limit = 10

loop:
    add x10, x10, x11 # sum += i
    addi x11, x11, 1  # i++
    ble x11, x12, loop

    li x17, 93
    ecall
```

### 3. Fibonacci Sequence
```asm
.text
.globl _start

_start:
    li x10, 0         # fib(0)
    li x11, 1         # fib(1)
    li x12, 10        # count

loop:
    add x13, x10, x11 # next = fib(n-1) + fib(n-2)
    mv x10, x11       # shift values
    mv x11, x13
    addi x12, x12, -1
    bnez x12, loop

    li x17, 93
    ecall
```

### 4. Array Processing
```asm
.text
.globl _start

_start:
    # Initialize array in memory
    li x10, 0x1000    # base address
    li x11, 5         # value
    li x12, 10        # count

store_loop:
    sw x11, 0(x10)    # store value
    addi x10, x10, 4  # next address
    addi x11, x11, 1  # increment value
    addi x12, x12, -1
    bnez x12, store_loop

    # Sum array
    li x10, 0x1000    # reset address
    li x11, 0         # sum = 0
    li x12, 10        # count

load_loop:
    lw x13, 0(x10)    # load value
    add x11, x11, x13 # add to sum
    addi x10, x10, 4
    addi x12, x12, -1
    bnez x12, load_loop

    mv x10, x11       # result in x10
    li x17, 93
    ecall
```

### 5. Function Call Example
```asm
.text
.globl _start

_start:
    li x10, 5         # argument
    jal x1, square    # call square function
    
    # x10 now contains result
    li x17, 93
    ecall

square:
    mul x10, x10, x10 # x10 = x10 * x10
    jalr x0, 0(x1)    # return
```

## Understanding Output

### Normal Execution
```
Loading program: tests/test_program.bin
Total instructions executed: 55
Total cycles: 89
```

### Debug Mode (`-d`)
```
Register Dump:
x0  = 0x00000000  x1  = 0x00000000  x2  = 0x00000000  x3  = 0x00000000
x4  = 0x00000000  x5  = 0x00000000  x6  = 0x00000000  x7  = 0x00000000
x8  = 0x00000000  x9  = 0x00000000  x10 = 0x00000037  x11 = 0x0000000b
...
PC  = 0x00000040
Instructions executed: 55

========== Performance Summary ==========
Total Instructions: 55
ALU Ops        : 33        (60.00%)
Loads          : 11        (20.00%)
Stores         : 5         (9.09%)
Branches       : 4         (7.27%)
Jumps          : 2         (3.64%)
Total cycles: 89
==========================================
```

### Step Mode (`-s`)
```
[Press Enter to execute next instruction]
PC: 0x00000000  Instruction: ADDI x10, x0, 0

[Press Enter to execute next instruction]
PC: 0x00000004  Instruction: ADDI x11, x0, 1

[Press Enter to execute next instruction]
...
```

### Memory Dump (`-m`)
```
Memory Dump (first 256 bytes):
0x00000000: 93 00 00 00 93 05 10 00 13 06 a0 00 33 85 b5 00
0x00000010: 93 05 15 00 e3 d8 c5 fe 93 08 d0 05 73 00 00 00
...
```

## Flow Visualization

### Generating Diagrams
```bash
# Run simulator with flow output piped to Python script
./build/riscv-sim tests/test_program.bin | python3 scripts/generate_flow.py
```

Output:
```
[FLOW] Cycle: 1 | PC: 0x00000000 | 0x00000093 | ADDI x1, x0, 0
Generated docs/frames/frame_1.png
[FLOW] Cycle: 2 | PC: 0x00000004 | 0x00100593 | ADDI x11, x0, 1
Generated docs/frames/frame_2.png
...
```

### Viewing Diagrams
```bash
# Open a specific frame
xdg-open docs/frames/frame_1.png  # Linux
open docs/frames/frame_1.png      # macOS

# View all frames
ls docs/frames/*.png
```

### Creating Animation
```bash
# Install ImageMagick
sudo apt-get install imagemagick  # Linux
brew install imagemagick          # macOS

# Create GIF animation
convert -delay 50 docs/frames/frame_*.png docs/animation.gif
```

## Troubleshooting

### Issue: "Command not found: riscv64-unknown-elf-as"

**Solution**: Install RISC-V toolchain
```bash
sudo apt-get install gcc-riscv64-unknown-elf
```

### Issue: "Cannot open file"

**Solution**: Check file path and ensure binary exists
```bash
ls -l tests/test_program.bin
file tests/test_program.bin
```

### Issue: "Unknown opcode"

**Solution**: Your program may contain unsupported instructions. Check that you're using only RV32I base instructions.

### Issue: "Segmentation fault"

**Solution**: Your program may be accessing invalid memory addresses. Add bounds checking to your assembly code.

### Issue: Flow diagrams not generating

**Solution**: Install Python Graphviz package
```bash
pip3 install graphviz
sudo apt-get install graphviz  # Also need system Graphviz
```

## Performance Tips

### 1. Optimize Memory Access

- Load data once, keep in registers
- Minimize store operations
- Use register-to-register operations when possible

### 2. Reduce Branches

- Unroll loops when iteration count is known
- Use branchless code for simple conditionals
- Keep hot loops small and tight

### 3. Instruction Selection

- Use immediate instructions (ADDI, ANDI) instead of register operations when possible
- Prefer shifts over multiplication when multiplying by powers of 2
- Use LUI+ADDI to load large constants efficiently

### 4. Profile Your Code
```bash
# Run with debug mode to see instruction mix
./build/riscv-sim -d your_program.bin

# Look for:
# - High branch percentage → consider branch reduction
# - High load/store percentage → optimize memory access
# - Low IPC → identify bottlenecks
```

## Advanced Usage

### Custom Memory Size

Edit `src/memory.cpp`:
```cpp
Memory::Memory(size_t size) : mem(size, 0) {
    // Change default size here
}
```

Or modify `src/main.cpp`:
```cpp
Memory memory(2 * 1024 * 1024);  // 2 MB instead of 1 MB
```

### Adding Breakpoints

Modify `src/cpu.cpp` in the `step()` function:
```cpp
void CPU::step() {
    if (halted) return;
    
    // Add breakpoint
    if (pc == 0x00000020) {
        std::cout << "Breakpoint hit at PC: 0x" << std::hex << pc << std::endl;
        dumpRegisters();
        // Can add interactive debugging here
    }
    
    // ... rest of step function
}
```

### Custom Statistics

Add your own counters in `cpu.h`:
```cpp
struct InstStats {
    // ... existing stats ...
    uint64_t custom_counter = 0;
} stats;
```

Update in `cpu.cpp`:
```cpp
void CPU::execALU(const Instruction& inst) {
    // ... existing code ...
    if (inst.funct3 == 0x0) {  // Track ADD/ADDI specifically
        stats.custom_counter++;
    }
}
```

## Best Practices

1. **Always assemble with `-march=rv32i`** to ensure compatibility
2. **Use `-Ttext=0x0`** when linking to start program at address 0
3. **End programs with ECALL** to properly halt simulation
4. **Test incrementally** - start with simple programs and add complexity
5. **Use debug mode** (`-d`) to verify register states
6. **Profile before optimizing** - measure performance first
7. **Keep documentation updated** when modifying the simulator

## Getting Help

- Check error messages carefully - they usually indicate the problem
- Use `-d` flag to see register states and identify issues
- Use `-s` flag to step through problematic code
- Review the RISC-V spec for instruction encoding details
- Check `docs/` directory for additional documentation