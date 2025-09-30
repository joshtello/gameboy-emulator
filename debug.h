#pragma once
#include "debug_config.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdint>

class DebugLogger {
private:
    static int instructionCount;
    static int debugCount;
    
public:
    // Initialize debug system
    static void init() {
        instructionCount = 0;
        debugCount = 0;
    }
    
    // Check if we should log based on current mode and PC
    static bool shouldLog(uint16_t pc) {
        if (!DEBUG_ENABLED) return false;
        
        switch (DEBUG_MODE) {
            case 0: return false;  // No logging
            case 1: return true;   // Log every instruction
            case 2: return (pc >= DEBUG_PC_START && pc <= DEBUG_PC_END);  // Range logging
            case 3: return (++debugCount % DEBUG_INTERVAL == 0);  // Interval logging
            default: return false;
        }
    }
    
    // Log CPU instruction execution
    static void logInstruction(uint16_t pc, uint8_t opcode, const std::string& description = "") {
        if (!shouldLog(pc)) return;
        
        instructionCount++;
        std::cout << "[" << std::setw(6) << instructionCount << "] ";
        std::cout << "PC=0x" << std::hex << std::setw(4) << std::setfill('0') << pc;
        std::cout << " opcode=0x" << std::setw(2) << std::setfill('0') << (int)opcode;
        if (!description.empty()) {
            std::cout << " (" << description << ")";
        }
        std::cout << std::endl;
    }
    
    // Log memory access
    static void logMemoryAccess(uint16_t address, uint8_t value, bool isWrite) {
        if (!DEBUG_ENABLED || !DEBUG_MEMORY_ACCESS) return;
        
        std::cout << "MEM: " << (isWrite ? "WRITE" : "READ") 
                  << " 0x" << std::hex << std::setw(4) << std::setfill('0') << address
                  << " = 0x" << std::setw(2) << std::setfill('0') << (int)value << std::endl;
    }
    
    // Log register changes
    static void logRegisterChange(const std::string& reg, uint8_t oldVal, uint8_t newVal) {
        if (!DEBUG_ENABLED || !DEBUG_REGISTERS) return;
        
        std::cout << "REG: " << reg << " 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)oldVal
                  << " -> 0x" << std::setw(2) << std::setfill('0') << (int)newVal << std::endl;
    }
    
    // Log flag changes
    static void logFlags(uint8_t oldFlags, uint8_t newFlags) {
        if (!DEBUG_ENABLED || !DEBUG_FLAGS) return;
        
        if (oldFlags != newFlags) {
            std::cout << "FLAGS: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)oldFlags
                      << " -> 0x" << std::setw(2) << std::setfill('0') << (int)newFlags << std::endl;
        }
    }
    
    // Log CPU state
    static void logCPUState(uint16_t pc, uint8_t a, uint8_t f, uint8_t b, uint8_t c, 
                           uint8_t d, uint8_t e, uint8_t h, uint8_t l, uint16_t sp) {
        if (!DEBUG_ENABLED) return;
        
        std::cout << "CPU: PC=0x" << std::hex << std::setw(4) << std::setfill('0') << pc
                  << " AF=0x" << std::setw(4) << std::setfill('0') << ((a << 8) | f)
                  << " BC=0x" << std::setw(4) << std::setfill('0') << ((b << 8) | c)
                  << " DE=0x" << std::setw(4) << std::setfill('0') << ((d << 8) | e)
                  << " HL=0x" << std::setw(4) << std::setfill('0') << ((h << 8) | l)
                  << " SP=0x" << std::setw(4) << std::setfill('0') << sp << std::endl;
    }
    
    // Get current instruction count
    static int getInstructionCount() { return instructionCount; }
    
    // Reset counters
    static void reset() {
        instructionCount = 0;
        debugCount = 0;
    }
};

// Static member definitions - moved to debug.cpp to avoid multiple definition errors
