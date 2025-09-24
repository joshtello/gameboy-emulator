#pragma once
#include <cstdint>
#include "memory.h"

class CPU {
private:
    // 8-bit registers
    uint8_t A;  // Accumulator
    uint8_t B;  // General purpose
    uint8_t C;  // General purpose
    uint8_t D;  // General purpose
    uint8_t E;  // General purpose
    uint8_t F;  // Flags register
    uint8_t H;  // General purpose
    uint8_t L;  // General purpose
    
    // 16-bit registers
    uint16_t PC;  // Program Counter
    uint16_t SP;  // Stack Pointer
    
    // Reference to memory system
    Memory& memory;
    
    // Flag positions in F register
    static constexpr uint8_t FLAG_ZERO      = 0x80;  // Bit 7
    static constexpr uint8_t FLAG_SUBTRACT  = 0x40;  // Bit 6
    static constexpr uint8_t FLAG_HALF_CARRY = 0x20;  // Bit 5
    static constexpr uint8_t FLAG_CARRY      = 0x10;  // Bit 4
    
    // Helper methods for flag operations
    bool getFlag(uint8_t flag) const;
    void setFlag(uint8_t flag, bool value);
    
public:
    // Constructor
    CPU(Memory& mem);
    
    // Register access methods
    uint8_t getA() const { return A; }
    uint8_t getB() const { return B; }
    uint8_t getC() const { return C; }
    uint8_t getD() const { return D; }
    uint8_t getE() const { return E; }
    uint8_t getF() const { return F; }
    uint8_t getH() const { return H; }
    uint8_t getL() const { return L; }
    uint16_t getPC() const { return PC; }
    uint16_t getSP() const { return SP; }
    
    void setA(uint8_t value) { A = value; }
    void setB(uint8_t value) { B = value; }
    void setC(uint8_t value) { C = value; }
    void setD(uint8_t value) { D = value; }
    void setE(uint8_t value) { E = value; }
    void setF(uint8_t value) { F = value; }
    void setH(uint8_t value) { H = value; }
    void setL(uint8_t value) { L = value; }
    void setPC(uint16_t value) { PC = value; }
    void setSP(uint16_t value) { SP = value; }
    
    // 16-bit register pairs (for Game Boy's register pairs)
    uint16_t getAF() const { return (A << 8) | F; }
    uint16_t getBC() const { return (B << 8) | C; }
    uint16_t getDE() const { return (D << 8) | E; }
    uint16_t getHL() const { return (H << 8) | L; }
    
    void setAF(uint16_t value) { A = (value >> 8) & 0xFF; F = value & 0xFF; }
    void setBC(uint16_t value) { B = (value >> 8) & 0xFF; C = value & 0xFF; }
    void setDE(uint16_t value) { D = (value >> 8) & 0xFF; E = value & 0xFF; }
    void setHL(uint16_t value) { H = (value >> 8) & 0xFF; L = value & 0xFF; }
    
    // Flag access methods
    bool getZeroFlag() const { return getFlag(FLAG_ZERO); }
    bool getSubtractFlag() const { return getFlag(FLAG_SUBTRACT); }
    bool getHalfCarryFlag() const { return getFlag(FLAG_HALF_CARRY); }
    bool getCarryFlag() const { return getFlag(FLAG_CARRY); }
    
    void setZeroFlag(bool value) { setFlag(FLAG_ZERO, value); }
    void setSubtractFlag(bool value) { setFlag(FLAG_SUBTRACT, value); }
    void setHalfCarryFlag(bool value) { setFlag(FLAG_HALF_CARRY, value); }
    void setCarryFlag(bool value) { setFlag(FLAG_CARRY, value); }
    
    // CPU operations
    void reset();                    // Reset CPU to initial state
    void step();                     // Execute one instruction
    void push(uint16_t value);       // Push value to stack
    uint16_t pop();                  // Pop value from stack
    
    // Memory access helpers
    uint8_t readByte(uint16_t address) const;
    void writeByte(uint16_t address, uint8_t value);
    uint16_t readWord(uint16_t address) const;
    void writeWord(uint16_t address, uint16_t value);
    
    // Debug methods
    void printRegisters() const;
    void printFlags() const;
};

