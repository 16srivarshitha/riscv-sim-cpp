#include <iostream>
#include <iomanip>
#include "cpu.h"
#include "memory.h"
#include "loader.h"

void printUsage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] <binary_file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help        Show this help message" << std::endl;
    std::cout << "  -d, --debug       Enable debug mode (dump registers after execution)" << std::endl;
    std::cout << "  -s, --step        Step through instructions" << std::endl;
    std::cout << "  -m, --memory      Dump memory after execution" << std::endl;
}

int main(int argc, char* argv[]) {
    bool debug = false;
    bool step_mode = false;
    bool dump_memory = false;
    std::string filename;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-d" || arg == "--debug") {
            debug = true;
        } else if (arg == "-s" || arg == "--step") {
            step_mode = true;
        } else if (arg == "-m" || arg == "--memory") {
            dump_memory = true;
        } else {
            filename = arg;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        // Load program
        std::cout << "Loading program: " << filename << std::endl;
        std::vector<uint8_t> program = Loader::loadBinary(filename);
        std::cout << "Program size: " << program.size() << " bytes" << std::endl;
        
        // Initialize memory and CPU
        Memory memory(1024 * 1024); // 1MB
        memory.loadProgram(program, 0);
        
        CPU cpu(memory);
        cpu.reset();
        
        std::cout << "Starting execution..." << std::endl;
        
        // Execute
        if (step_mode) {
            std::cout << "Step mode enabled. Press Enter to execute each instruction." << std::endl;
            while (!cpu.isHalted()) {
                std::cout << "PC: 0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << cpu.getPC() << " - Press Enter...";
                std::cin.get();
                cpu.step();
                if (debug) {
                    cpu.dumpRegisters();
                }
            }
        } else {
            cpu.run();
        }
        
        std::cout << "\nExecution completed." << std::endl;
        std::cout << "Total instructions executed: " << std::dec 
                  << cpu.getInstructionCount() << std::endl;
        
        std::cout << "Total cycles: " << cpu.getCycleCount() << std::endl;          
        cpu.printPerformanceStats();
        
        // Dump registers if debug mode
        if (debug) {
            std::cout << "\n";
            cpu.dumpRegisters();
        }
        
        // Dump memory if requested
        if (dump_memory) {
            std::cout << "\n";
            memory.dump(0, 256);
        }
        
        // Print return value (from x10/a0)
        std::cout << "\nReturn value (a0): " << cpu.getRegister(10) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}