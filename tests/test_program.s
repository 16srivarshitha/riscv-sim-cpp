# RISC-V Test Program: Fibonacci Sequence
# Computes first 10 Fibonacci numbers and stores them in memory

.text
.globl _start

_start:
    # Initialize Fibonacci variables
    li x10, 0           # fib(0) = 0
    li x11, 1           # fib(1) = 1
    li x12, 10          # counter/limit
    li x13, 0           # current count
    li x14, 0x1000      # memory base address for storing results
    
    # Store first two Fibonacci numbers
    sw x10, 0(x14)      # mem[0x1000] = 0
    sw x11, 4(x14)      # mem[0x1004] = 1
    addi x13, x13, 2    # count = 2
    addi x14, x14, 8    # move to next memory location

fib_loop:
    # Check if we've computed enough numbers
    bge x13, x12, done  # if count >= 10, exit
    
    # Compute next Fibonacci number
    add x15, x10, x11   # fib(n) = fib(n-1) + fib(n-2)
    
    # Store result in memory
    sw x15, 0(x14)      # save to memory
    
    # Shift values for next iteration
    mv x10, x11         # fib(n-2) = fib(n-1)
    mv x11, x15         # fib(n-1) = fib(n)
    
    # Increment counter and memory pointer
    addi x13, x13, 1    # count++
    addi x14, x14, 4    # move memory pointer
    
    # Continue loop
    j fib_loop

done:
    # Load some results back for verification
    li x20, 0x1000      # reset base address
    lw x21, 0(x20)      # load fib(0)
    lw x22, 16(x20)     # load fib(4)
    lw x23, 36(x20)     # load fib(9)
    
    # Perform some ALU operations for diversity
    sll x24, x22, 2     # shift left
    srl x25, x23, 1     # shift right
    and x26, x22, x23   # bitwise AND
    or x27, x22, x23    # bitwise OR
    xor x28, x22, x23   # bitwise XOR
    
    # Exit with result in x23 (fib(9) = 34)
    li x17, 93
    ecall