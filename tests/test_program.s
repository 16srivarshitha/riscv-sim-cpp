.text
.globl _start

_start:
    li x10, 0      
    li x11, 1      
    li x12, 10     

loop:
    add x10, x10, x11   
    addi x11, x11, 1   
    ble x11, x12, loop  

    li x17, 93     
    ecall
