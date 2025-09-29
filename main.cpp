#include <iostream>
#include "memory.h"
#include "cpu.h"

int main(int argc, char* argv[]) {
    // Create memory and CPU only
    Memory memory;
    CPU cpu(memory);

    // Load the Blargg CPU instruction test ROM
    try {
        memory.loadRom("cpu_instrs.gb");  // Load Blargg CPU instruction test
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }

    // Initialize CPU only (no graphics for testing)
    cpu.reset();  // Sets PC to 0x100 (ROM entry point)


    std::cout << "Starting Blargg CPU instruction test..." << std::endl;
    std::cout << "Test output will be displayed below:" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    bool running = true;
    int cycleCount = 0;
    const int MAX_CYCLES = 10000000; // Increased limit for comprehensive testing
    
    std::cout << "Running Blargg test until completion..." << std::endl;
    
    while (running && cycleCount < MAX_CYCLES) {
        // Execute CPU instruction
        cpu.step();
        cycleCount++;
        
        // Check for serial output (Blargg test uses serial port for output)
        // Serial port is at 0xFF01 (SB) and 0xFF02 (SC)
        uint8_t sc = memory.read(0xFF02);
        if (sc & 0x80) { // Transfer enabled
            uint8_t sb = memory.read(0xFF01);
            std::cout << static_cast<char>(sb);
            std::cout.flush();
            
            // Reset transfer flag
            memory.write(0xFF02, sc & 0x7F);
        }
        
        // Check if test has completed by looking for specific patterns
        // Blargg tests typically end with "Passed" or error messages
        static std::string output_buffer;
        if (sc & 0x80) {
            output_buffer += static_cast<char>(memory.read(0xFF01));
            if (output_buffer.find("Passed") != std::string::npos || 
                output_buffer.find("Failed") != std::string::npos ||
                output_buffer.find("Error") != std::string::npos) {
                std::cout << std::endl << "Test completed!" << std::endl;
                running = false;
                break;
            }
        }
        
        // Progress indicator
        if (cycleCount % 100000 == 0) {
            std::cout << "Cycles: " << cycleCount << std::endl;
        }
    }
    
    if (cycleCount >= MAX_CYCLES) {
        std::cout << std::endl << "Test timed out after " << MAX_CYCLES << " cycles" << std::endl;
    }
    
    std::cout << "=====================================" << std::endl;
    std::cout << "CPU test completed. Total cycles: " << cycleCount << std::endl;

    return 0;
}