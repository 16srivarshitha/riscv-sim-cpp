#include "instruction.h"
#include <sstream>
#include <iomanip>

std::string Instruction::toString() const {
    std::ostringstream oss;
    
    switch (opcode) {
        case Opcode::LUI:
            oss << "LUI x" << static_cast<int>(rd) << ", 0x" << std::hex << imm;
            break;
        case Opcode::AUIPC:
            oss << "AUIPC x" << static_cast<int>(rd) << ", 0x" << std::hex << imm;
            break;
        case Opcode::JAL:
            oss << "JAL x" << static_cast<int>(rd) << ", " << std::dec << static_cast<int32_t>(imm);
            break;
        case Opcode::JALR:
            oss << "JALR x" << static_cast<int>(rd) << ", x" << static_cast<int>(rs1) 
                << ", " << std::dec << static_cast<int32_t>(imm);
            break;
        case Opcode::OP_IMM:
            oss << "OP_IMM (funct3=" << static_cast<int>(funct3) << ") x" 
                << static_cast<int>(rd) << ", x" << static_cast<int>(rs1) 
                << ", " << std::dec << static_cast<int32_t>(imm);
            break;
        case Opcode::OP:
            oss << "OP (funct3=" << static_cast<int>(funct3) << ", funct7=" 
                << static_cast<int>(funct7) << ") x" << static_cast<int>(rd) 
                << ", x" << static_cast<int>(rs1) << ", x" << static_cast<int>(rs2);
            break;
        default:
            oss << "UNKNOWN";
    }
    
    return oss.str();
}