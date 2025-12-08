#ifndef DECODER_H
#define DECODER_H

#include "instruction.h"

class Decoder {
public:
    static Instruction decode(uint32_t inst_word);
    
private:
    static uint32_t signExtend(uint32_t value, int bits);
    static Instruction decodeR(uint32_t inst_word, Opcode opcode);
    static Instruction decodeI(uint32_t inst_word, Opcode opcode);
    static Instruction decodeS(uint32_t inst_word, Opcode opcode);
    static Instruction decodeB(uint32_t inst_word, Opcode opcode);
    static Instruction decodeU(uint32_t inst_word, Opcode opcode);
    static Instruction decodeJ(uint32_t inst_word, Opcode opcode);
};

#endif // DECODER_H