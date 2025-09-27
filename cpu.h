#pragma once
#include <cstdint>
#include "memory.h"

union RegisterPair {
    struct {
        uint8_t low;    // F, C, E, or L
        uint8_t high;   // A, B, D, or H
    };
    uint16_t pair;      // Combined value (AF, BC, DE, or HL)
};

class CPU {
private:
    // Opcodes
    static constexpr uint8_t NOP = 0x00;        // No operation
    static constexpr uint8_t LD_BC_NN = 0x01;   // Load 16-bit immediate into BC
    static constexpr uint8_t LD_B_N = 0x06;     // Load 8-bit immediate into B
    static constexpr uint8_t JP_NN = 0xC3;      // Jump to address NN
    static constexpr uint8_t HALT = 0x76;       // Halt CPU until interrupt

    // Load immediate value into register (LD r,n)
    static constexpr uint8_t LD_C_N = 0x0E;     // Load 8-bit immediate into C
    static constexpr uint8_t LD_D_N = 0x16;     // Load 8-bit immediate into D
    static constexpr uint8_t LD_E_N = 0x1E;     // Load 8-bit immediate into E
    static constexpr uint8_t LD_H_N = 0x26;     // Load 8-bit immediate into H
    static constexpr uint8_t LD_L_N = 0x2E;     // Load 8-bit immediate into L
    static constexpr uint8_t LD_A_N = 0x3E;     // Load 8-bit immediate into A

    // Load register into B (LD B,r)
    static constexpr uint8_t LD_B_B = 0x40;     // Load B into B
    static constexpr uint8_t LD_B_C = 0x41;     // Load C into B
    static constexpr uint8_t LD_B_D = 0x42;     // Load D into B
    static constexpr uint8_t LD_B_E = 0x43;     // Load E into B
    static constexpr uint8_t LD_B_H = 0x44;     // Load H into B
    static constexpr uint8_t LD_B_L = 0x45;     // Load L into B
    static constexpr uint8_t LD_B_A = 0x47;     // Load A into B

    // Load register into C (LD C,r)
    static constexpr uint8_t LD_C_B = 0x48;     // Load B into C
    static constexpr uint8_t LD_C_C = 0x49;     // Load C into C
    static constexpr uint8_t LD_C_D = 0x4A;     // Load D into C
    static constexpr uint8_t LD_C_E = 0x4B;     // Load E into C
    static constexpr uint8_t LD_C_H = 0x4C;     // Load H into C
    static constexpr uint8_t LD_C_L = 0x4D;     // Load L into C
    static constexpr uint8_t LD_C_A = 0x4F;     // Load A into C

    // Load register into D (LD D,r)
    static constexpr uint8_t LD_D_B = 0x50;     // Load B into D
    static constexpr uint8_t LD_D_C = 0x51;     // Load C into D
    static constexpr uint8_t LD_D_D = 0x52;     // Load D into D
    static constexpr uint8_t LD_D_E = 0x53;     // Load E into D
    static constexpr uint8_t LD_D_H = 0x54;     // Load H into D
    static constexpr uint8_t LD_D_L = 0x55;     // Load L into D
    static constexpr uint8_t LD_D_A = 0x57;     // Load A into D

    // Load register into E (LD E,r)
    static constexpr uint8_t LD_E_B = 0x58;     // Load B into E
    static constexpr uint8_t LD_E_C = 0x59;     // Load C into E
    static constexpr uint8_t LD_E_D = 0x5A;     // Load D into E
    static constexpr uint8_t LD_E_E = 0x5B;     // Load E into E
    static constexpr uint8_t LD_E_H = 0x5C;     // Load H into E
    static constexpr uint8_t LD_E_L = 0x5D;     // Load L into E
    static constexpr uint8_t LD_E_A = 0x5F;     // Load A into E

    // Load register into H (LD H,r)
    static constexpr uint8_t LD_H_B = 0x60;     // Load B into H
    static constexpr uint8_t LD_H_C = 0x61;     // Load C into H
    static constexpr uint8_t LD_H_D = 0x62;     // Load D into H
    static constexpr uint8_t LD_H_E = 0x63;     // Load E into H
    static constexpr uint8_t LD_H_H = 0x64;     // Load H into H
    static constexpr uint8_t LD_H_L = 0x65;     // Load L into H
    static constexpr uint8_t LD_H_A = 0x67;     // Load A into H

    // Load register into L (LD L,r)
    static constexpr uint8_t LD_L_B = 0x68;     // Load B into L
    static constexpr uint8_t LD_L_C = 0x69;     // Load C into L
    static constexpr uint8_t LD_L_D = 0x6A;     // Load D into L
    static constexpr uint8_t LD_L_E = 0x6B;     // Load E into L
    static constexpr uint8_t LD_L_H = 0x6C;     // Load H into L
    static constexpr uint8_t LD_L_L = 0x6D;     // Load L into L
    static constexpr uint8_t LD_L_A = 0x6F;     // Load A into L

    // Load register into A (LD A,r)
    static constexpr uint8_t LD_A_B = 0x78;     // Load B into A
    static constexpr uint8_t LD_A_C = 0x79;     // Load C into A
    static constexpr uint8_t LD_A_D = 0x7A;     // Load D into A
    static constexpr uint8_t LD_A_E = 0x7B;     // Load E into A
    static constexpr uint8_t LD_A_H = 0x7C;     // Load H into A
    static constexpr uint8_t LD_A_L = 0x7D;     // Load L into A
    static constexpr uint8_t LD_A_A = 0x7F;     // Load A into A

    RegisterPair AF;
    RegisterPair BC;
    RegisterPair DE;
    RegisterPair HL;
    
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

    bool interruptEnable;
    
public:
    // Constructor
    CPU(Memory& mem);
    
    // Register access methods
    uint8_t getA() const { return AF.high; }
    uint8_t getB() const { return BC.high; }
    uint8_t getC() const { return BC.low; }
    uint8_t getD() const { return DE.high; }
    uint8_t getE() const { return DE.low; }
    uint8_t getF() const { return AF.low; }
    uint8_t getH() const { return HL.high; }
    uint8_t getL() const { return HL.low; }
    
    uint16_t getPC() const { return PC; }
    uint16_t getSP() const { return SP; }
    
    void setA(uint8_t value) { AF.high = value; }
    void setB(uint8_t value) { BC.high = value; }
    void setC(uint8_t value) { BC.low = value; }
    void setD(uint8_t value) { DE.high = value; }
    void setE(uint8_t value) { DE.low = value; }
    void setF(uint8_t value) { AF.low = value; }
    void setH(uint8_t value) { HL.high = value; }
    void setL(uint8_t value) { HL.low = value; }
    void setPC(uint16_t value) { PC = value; }
    void setSP(uint16_t value) { SP = value; }
    
    // 16-bit register pairs
    uint16_t getAF() const { return AF.pair; }
    uint16_t getBC() const { return BC.pair; }
    uint16_t getDE() const { return DE.pair; }
    uint16_t getHL() const { return HL.pair; }
    
    void setAF(uint16_t value) { AF.pair = value; }
    void setBC(uint16_t value) { BC.pair = value; }
    void setDE(uint16_t value) { DE.pair = value; }
    void setHL(uint16_t value) { HL.pair = value; }
    
    // Flag access methods
    bool getZeroFlag() const { return getFlag(FLAG_ZERO); }
    bool getSubtractFlag() const { return getFlag(FLAG_SUBTRACT); }
    bool getHalfCarryFlag() const { return getFlag(FLAG_HALF_CARRY); }
    bool getCarryFlag() const { return getFlag(FLAG_CARRY); }
    
    void setZeroFlag(bool value) { setFlag(FLAG_ZERO, value); }
    void setSubtractFlag(bool value) { setFlag(FLAG_SUBTRACT, value); }
    void setHalfCarryFlag(bool value) { setFlag(FLAG_HALF_CARRY, value); }
    void setCarryFlag(bool value) { setFlag(FLAG_CARRY, value); }
    
    // Stack operation opcodes
    static constexpr uint8_t PUSH_AF = 0xF5;     // Push AF onto stack
    static constexpr uint8_t PUSH_BC = 0xC5;     // Push BC onto stack
    static constexpr uint8_t PUSH_DE = 0xD5;     // Push DE onto stack
    static constexpr uint8_t PUSH_HL = 0xE5;     // Push HL onto stack

    static constexpr uint8_t POP_AF = 0xF1;     // Pop AF from stack
    static constexpr uint8_t POP_BC = 0xC1;     // Pop BC from stack
    static constexpr uint8_t POP_DE = 0xD1;     // Pop DE from stack
    static constexpr uint8_t POP_HL = 0xE1;     // Pop HL from stack

    // Call operation opcodes
    static constexpr uint8_t CALL_NN = 0xCD;    // Call to address NN
    static constexpr uint8_t CALL_NZ = 0xC4;    // Call if Zero flag is 0
    static constexpr uint8_t CALL_Z = 0xCC;     // Call if Zero flag is 1
    static constexpr uint8_t CALL_NC = 0xD4;    // Call if Carry flag is 0
    static constexpr uint8_t CALL_C = 0xDC;     // Call if Carry flag is 1

    
    static constexpr uint8_t LD_SP_NN = 0x31;    // Load 16-bit immediate into SP

    static constexpr uint8_t DI = 0xF3;     // Disable interrupts

    static constexpr uint8_t LD_NN_A = 0xEA;    // Store A to memory address

    static constexpr uint8_t JR_R8 = 0x18;    // Jump relative

    static constexpr uint8_t RET = 0xC9;    // Return from Function
    
    static constexpr uint8_t LD_HL_NN = 0x21;    // Load 16-bit immediate into HL

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

    void setInterruptEnable(bool value);
};