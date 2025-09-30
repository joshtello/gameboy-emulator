#include <iostream>
#include "memory.h"
#include "cpu.h"
#include "debug.h"

int main(int argc, char* argv[]) {
    std::cout << "Simple Game Boy Emulator - Pokemon Test" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Create memory and CPU
    Memory memory;
    CPU cpu(memory);
    
    // Load Pokemon ROM
    try {
        memory.loadRom("pokemon_blue.gb");
        std::cout << "Pokemon Blue ROM loaded successfully!" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }
    
    // Initialize CPU
    cpu.reset();
    std::cout << "CPU initialized" << std::endl;
    
    // Run for a limited number of instructions to see what happens
    int instructionCount = 0;
    const int MAX_INSTRUCTIONS = 1000;
    
    std::cout << "Starting Pokemon Blue..." << std::endl;
    std::cout << "Running first " << MAX_INSTRUCTIONS << " instructions..." << std::endl;
    
    while (instructionCount < MAX_INSTRUCTIONS) {
        try {
            int cycles = cpu.step();
            instructionCount++;
            
            // Show progress every 100 instructions
            if (instructionCount % 100 == 0) {
                std::cout << "Instruction " << instructionCount << " - PC: 0x" << std::hex << cpu.getPC() << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "CPU Error at instruction " << instructionCount << ": " << e.what() << std::endl;
            break;
        }
    }
    
    std::cout << "Completed " << instructionCount << " instructions" << std::endl;
    std::cout << "Final CPU state:" << std::endl;
    cpu.printRegisters();
    
    return 0;
}
