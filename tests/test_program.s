# Simple RISC-V test program
# Calculates the sum of numbers from 1 to 10

.text
.globl _start

_start:
    # Initialize registers
    li x10, 0       # x10 (a0) will hold the sum
    li x11, 1       # x11 (a1) is the counter
    li x12, 10      # x12 (a2) is the limit

loop:
    add x10, x10, x11   # sum += counter
    addi x11, x11, 1    # counter++
    ble x11, x12, loop  # if counter <= limit, continue loop

    # Exit (ECALL)
    li x17, 93      # exit syscall number
    ecall

# To assemble this:
# riscv64-unknown-elf-as -march=rv32i -mabi=ilp32 test_program.s -o test_program.o
# riscv64-unknown-elf-ld -m elf32lriscv test_program.o -o test_program.elf
# riscv64-unknown-elf-objcopy -O binary test_program.elf test_program.bin