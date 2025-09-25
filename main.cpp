#include <SDL3/SDL.h>
#include <iostream>
#include "memory.h"
#include "cpu.h"

int main(int argc, char* argv[]) {
    // Create memory and CPU
    Memory memory;
    CPU cpu(memory);

    // Load the test ROM
    try {
        memory.loadRom("cpu_instrs.gb");  // Make sure this file is in your project directory
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }

    // Initialize CPU
    cpu.reset();  // Sets PC to 0x100 (ROM entry point)

    // Run CPU steps and print output
    for (int i = 0; i < 1000; i++) {  // Run 1000 instructions for now
        cpu.step();
        
        // Print register state every 100 instructions
        if (i % 100 == 0) {
            cpu.printRegisters();
            std::cout << "----------------" << std::endl;
        }
    }

    return 0;
}