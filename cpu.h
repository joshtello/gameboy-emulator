#pragma once
#include <cstdint>
#include "memory.h"
#include "debug.h"

// Forward declarations
class PPU;
class Timer;

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
    static constexpr uint8_t RLA = 0x17;        // Rotate A left through carry
    static constexpr uint8_t RRCA = 0x0F;       // Rotate A right, carry to bit 7
    static constexpr uint8_t JP_NN = 0xC3;      // Jump to address NN
    static constexpr uint8_t HALT = 0x76;       // Halt CPU until interrupt
    static constexpr uint8_t EI = 0xFB;         // Enable interrupts
    static constexpr uint8_t DI = 0xF3;         // Disable interrupts

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
    
    // Cycle counter for preventing infinite loops
    uint32_t cycleCount;
    
    // Interrupt handling
    bool IME = false;               // Interrupt Master Enable
    bool pendingIME = false;        // EI instruction pending (enables after next instruction)
    
    // References to other systems
    Memory& memory;
    PPU* ppu = nullptr;
    Timer* timer = nullptr;
    
    // Cycle table for all Game Boy opcodes
    static constexpr uint8_t CYCLE_TABLE[256] = {
          // 0x00 - 0x0F
     4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
     // 0x10 - 0x1F
      4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
     // 0x20 - 0x2F
      8, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
     // 0x30 - 0x3F
      8, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
     // 0x40 - 0x4F
      4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
     // 0x50 - 0x5F
      4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
     // 0x60 - 0x6F
      4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
     // 0x70 - 0x7F
      4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
     // 0x80 - 0x8F
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     // 0x90 - 0x9F
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     // 0xA0 - 0xAF
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     // 0xB0 - 0xBF
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     // 0xC0 - 0xCF
      8, 12, 12, 12, 12, 16,  8, 16,  8, 16, 16, 16, 12, 16,  8, 16,
     // 0xD0 - 0xDF
      8, 12, 12, 12, 12, 16,  8, 16,  8, 16, 16, 16, 12, 16,  8, 16,
     // 0xE0 - 0xEF
     12, 12,  8, 12,  8, 16,  8, 16, 16,  4, 16,  4,  8,  8,  8,  8,
     // 0xF0 - 0xFF
     12, 12,  8, 12,  8, 16,  8, 16, 12,  4, 16,  4,  8,  8,  8,  8
    };
    
    // Cycle table for CB prefixed opcodes (0xCB)
    static constexpr uint8_t CB_CYCLE_TABLE[256] = {
        // 0x00-0x0F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0x10-0x1F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0x20-0x2F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0x30-0x3F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0x40-0x4F
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        // 0x50-0x5F
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        // 0x60-0x6F
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        // 0x70-0x7F
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        // 0x80-0x8F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0x90-0x9F
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xA0-0xAF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xB0-0xBF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xC0-0xCF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xD0-0xDF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xE0-0xEF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        // 0xF0-0xFF
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8
    };
    
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
    void setF(uint8_t value) { AF.low = value & 0xF0; }
    void setH(uint8_t value) { HL.high = value; }
    void setL(uint8_t value) { HL.low = value; }
    void setPC(uint16_t value) { PC = value; }
    void setSP(uint16_t value) { SP = value; }
    
    // 16-bit register pairs
    uint16_t getAF() const { return AF.pair; }
    uint16_t getBC() const { return BC.pair; }
    uint16_t getDE() const { return DE.pair; }
    uint16_t getHL() const { return HL.pair; }
    
    void setAF(uint16_t value) { AF.pair = value; AF.low &= 0xF0; }
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


    static constexpr uint8_t LD_NN_A = 0xEA;    // Store A to memory address
    static constexpr uint8_t LD_A_NN = 0xFA;    // Load A from memory address

    static constexpr uint8_t JR_R8 = 0x18;    // Jump relative

    static constexpr uint8_t RET = 0xC9;    // Return from Function
    
    static constexpr uint8_t LD_HL_NN = 0x21;    // Load 16-bit immediate into HL

    static constexpr uint8_t SUB_d8 = 0xD6;    // Subtract 8-bit immediate from A

    static constexpr uint8_t ADD_A_d8 = 0xC6;    // Add 8-bit immediate to A

    static constexpr uint8_t INC_A = 0x3C;

    static constexpr uint8_t DEC_A = 0x3D;

    static constexpr uint8_t CP_d8 = 0xFE;

    static constexpr uint8_t LD_HLplus_A = 0x22;  // Store A to (HL) and increment HL

    static constexpr uint8_t LD_DE_NN = 0x11;    // Load 16-bit immediate into DE

    static constexpr uint8_t JR_NZ_R8 = 0x20;    // Jump if not zero

    static constexpr uint8_t LDH_A_a8 = 0xF0;    // Load A from I/O register

    static constexpr uint8_t XOR_A = 0xAF;       // XOR A with A (sets A to 0)

    static constexpr uint8_t LDH_a8_A = 0xE0;    // Store A to I/O register

    static constexpr uint8_t LD_A_HLplus = 0x2A; // Load A from memory and increment HL

    static constexpr uint8_t JR_Z_R8 = 0x28;     // Jump if zero flag is set

    static constexpr uint8_t INC_BC = 0x03;      // Increment BC register

    static constexpr uint8_t ADD_A_A = 0x87;     // Add A to A

    static constexpr uint8_t CB_PREFIX = 0xCB;   // CB prefix for bit operations

    static constexpr uint8_t LD_A_HL = 0x7E;     // Load A from (HL)
    static constexpr uint8_t LD_HL_A = 0x77;     // Store A to (HL)
    static constexpr uint8_t ADD_HL_BC = 0x09;   // Add BC to HL
    static constexpr uint8_t DEC_BC = 0x0B;      // Decrement BC


    static constexpr uint8_t JR_C_R8 = 0x38;     // Jump if carry flag set
    static constexpr uint8_t JR_NC_R8 = 0x30;   // Jump if carry flag not set
    static constexpr uint8_t ADD_HL_DE = 0x19;   // Add DE to HL
    static constexpr uint8_t ADD_HL_HL = 0x29;  // Add HL to HL
    static constexpr uint8_t LD_HL_D8 = 0x36;   // Store immediate to (HL)
    static constexpr uint8_t LD_A_BC = 0x0A;     // Load A from (BC)
    static constexpr uint8_t LD_BC_A = 0x02;     // Store A to (BC)

    static constexpr uint8_t LD_NN_A_ALT = 0xEA;  // Store A to absolute address (alternative)

    // Critical missing instructions for Pokemon Red
    static constexpr uint8_t LD_HLminus_A = 0x32;  // Store A to (HL) and decrement HL
    static constexpr uint8_t LD_A_HLminus = 0x3A;  // Load A from (HL) and decrement HL
    static constexpr uint8_t LD_NN_SP = 0x08;      // Store SP to absolute address
    static constexpr uint8_t JP_C_NN = 0xDA;       // Jump to address if carry
    static constexpr uint8_t JP_NC_NN = 0xD2;      // Jump to address if not carry
    static constexpr uint8_t JP_Z_NN = 0xCA;       // Jump to address if zero
    static constexpr uint8_t JP_NZ_NN = 0xC2;      // Jump to address if not zero
    static constexpr uint8_t RETI = 0xD9;          // Return from interrupt
    static constexpr uint8_t RST_00 = 0xC7;        // Restart to 0x00
    static constexpr uint8_t RST_08 = 0xCF;        // Restart to 0x08
    static constexpr uint8_t RST_10 = 0xD7;        // Restart to 0x10
    static constexpr uint8_t RST_18 = 0xDF;        // Restart to 0x18
    static constexpr uint8_t RST_20 = 0xE7;        // Restart to 0x20
    static constexpr uint8_t RST_28 = 0xEF;        // Restart to 0x28
    static constexpr uint8_t RST_30 = 0xF7;        // Restart to 0x30
    static constexpr uint8_t RST_38 = 0xFF;        // Restart to 0x38
    static constexpr uint8_t INC_DE = 0x13;        // Increment DE
    static constexpr uint8_t INC_HL = 0x23;        // Increment HL
    static constexpr uint8_t INC_HL_mem = 0x34;    // Increment memory at (HL)
    static constexpr uint8_t DEC_DE = 0x1B;        // Decrement DE
    static constexpr uint8_t DEC_HL = 0x2B;        // Decrement HL
    static constexpr uint8_t ADD_HL_SP = 0x39;     // Add SP to HL
    static constexpr uint8_t DAA = 0x27;           // Decimal adjust A
    static constexpr uint8_t CPL = 0x2F;           // Complement A
    static constexpr uint8_t SCF = 0x37;           // Set carry flag
    static constexpr uint8_t CCF = 0x3F;            // Complement carry flag

    // Additional critical memory operations
    static constexpr uint8_t LD_DE_A = 0x12;        // Store A to (DE)
    static constexpr uint8_t LD_A_DE = 0x1A;        // Load A from (DE)

    // CB prefix opcodes (bit operations, rotates, shifts)
    static constexpr uint8_t RLC_A = 0x07;          // Rotate Left Circular A
    static constexpr uint8_t RL_A = 0x17;           // Rotate Left A through carry
    static constexpr uint8_t RRC_A = 0x0F;          // Rotate Right Circular A
    static constexpr uint8_t SLA_A = 0x27;          // Shift Left Arithmetic A
    static constexpr uint8_t SRA_A = 0x2F;         // Shift Right Arithmetic A
    static constexpr uint8_t SRL_A = 0x3F;         // Shift Right Logical A
    static constexpr uint8_t SWAP_A = 0x37;        // Swap nibbles of A
    
    // BIT operations (CB prefix)
    static constexpr uint8_t BIT_0_A = 0x47;        // Test bit 0 of A
    static constexpr uint8_t BIT_1_A = 0x4F;        // Test bit 1 of A
    static constexpr uint8_t BIT_2_A = 0x57;        // Test bit 2 of A
    static constexpr uint8_t BIT_3_A = 0x5F;        // Test bit 3 of A
    static constexpr uint8_t BIT_4_A = 0x67;        // Test bit 4 of A
    static constexpr uint8_t BIT_5_A = 0x6F;        // Test bit 5 of A
    static constexpr uint8_t BIT_6_A = 0x77;        // Test bit 6 of A
    static constexpr uint8_t BIT_7_A = 0x7F;        // Test bit 7 of A
    
    // BIT operations for other registers
    static constexpr uint8_t BIT_0_B = 0x40;        // Test bit 0 of B
    static constexpr uint8_t BIT_0_C = 0x41;        // Test bit 0 of C
    static constexpr uint8_t BIT_0_D = 0x42;        // Test bit 0 of D
    static constexpr uint8_t BIT_0_E = 0x43;        // Test bit 0 of E
    static constexpr uint8_t BIT_0_H = 0x44;        // Test bit 0 of H
    static constexpr uint8_t BIT_0_L = 0x45;        // Test bit 0 of L
    static constexpr uint8_t BIT_0_HL = 0x46;       // Test bit 0 of (HL)
    
    // RES operations (CB prefix)
    static constexpr uint8_t RES_0_A = 0x87;        // Reset bit 0 of A
    static constexpr uint8_t RES_1_A = 0x8F;        // Reset bit 1 of A
    static constexpr uint8_t RES_2_A = 0x97;        // Reset bit 2 of A
    static constexpr uint8_t RES_3_A = 0x9F;        // Reset bit 3 of A
    static constexpr uint8_t RES_4_A = 0xA7;        // Reset bit 4 of A
    static constexpr uint8_t RES_5_A = 0xAF;        // Reset bit 5 of A
    static constexpr uint8_t RES_6_A = 0xB7;        // Reset bit 6 of A
    static constexpr uint8_t RES_7_A = 0xBF;        // Reset bit 7 of A
    
    // SET operations (CB prefix)
    static constexpr uint8_t SET_0_A = 0xC7;        // Set bit 0 of A
    static constexpr uint8_t SET_1_A = 0xCF;        // Set bit 1 of A
    static constexpr uint8_t SET_2_A = 0xD7;        // Set bit 2 of A
    static constexpr uint8_t SET_3_A = 0xDF;        // Set bit 3 of A
    static constexpr uint8_t SET_4_A = 0xE7;        // Set bit 4 of A
    static constexpr uint8_t SET_5_A = 0xEF;        // Set bit 5 of A
    static constexpr uint8_t SET_6_A = 0xF7;        // Set bit 6 of A
    static constexpr uint8_t SET_7_A = 0xFF;        // Set bit 7 of A

    // Arithmetic and logic operations
    static constexpr uint8_t OR_C = 0xB1;            // Bitwise OR between A and C
    static constexpr uint8_t RRA = 0x1F;            // Rotate Right A through carry
    static constexpr uint8_t XOR_C = 0xA9;          // Bitwise XOR between A and C
    static constexpr uint8_t DEC_B = 0x05;          // Decrement register B
    static constexpr uint8_t DEC_D = 0x15;          // Decrement register D
    static constexpr uint8_t INC_H = 0x24;           // Increment register H
    static constexpr uint8_t INC_L = 0x2C;           // Increment register L
    static constexpr uint8_t AND_d8 = 0xE6;         // Bitwise AND between A and immediate
    static constexpr uint8_t OR_A = 0xB7;           // OR A with A (test A for zero)
    static constexpr uint8_t DEC_L = 0x2D;           // Decrement register L
    static constexpr uint8_t DEC_H = 0x25;           // Decrement register H
    static constexpr uint8_t DEC_E = 0x1D;           // Decrement register E
    static constexpr uint8_t ADC_A_d8 = 0xCE;       // Add with carry immediate to A
    static constexpr uint8_t XOR_d8 = 0xEE;         // XOR A with immediate value

    // Memory access operations
    static constexpr uint8_t LD_B_HL = 0x46;        // Load B from (HL)
    static constexpr uint8_t LD_L_HL = 0x6E;        // Load L from (HL)
    static constexpr uint8_t LD_C_HL = 0x4E;        // Load C from (HL)
    static constexpr uint8_t LD_D_HL = 0x56;        // Load D from (HL)
    static constexpr uint8_t LD_HL_D = 0x72;        // Store D to (HL)
    static constexpr uint8_t LD_HL_C = 0x71;        // Store C to (HL)
    static constexpr uint8_t LD_HL_B = 0x70;        // Store B to (HL)
    static constexpr uint8_t DEC_HL_mem = 0x35;     // Decrement memory at (HL)
    static constexpr uint8_t OR_HL = 0xB6;          // OR A with (HL)
    static constexpr uint8_t XOR_HL = 0xAE;         // XOR A with (HL)

    // Conditional return operations
    static constexpr uint8_t RET_NC = 0xD0;         // Return if carry flag not set
    static constexpr uint8_t RET_Z = 0xC8;          // Return if zero flag set

    // Jump operations
    static constexpr uint8_t JP_HL = 0xE9;          // Jump to address in HL register

    // Additional missing opcodes
    static constexpr uint8_t OR_d8 = 0xF6;          // OR A with immediate value
    static constexpr uint8_t INC_B = 0x04;          // Increment register B
    static constexpr uint8_t ADD_A_C = 0x81;       // Add C to A
    static constexpr uint8_t LD_HL_SP_r8 = 0xF8;   // LD HL, SP+r8
    static constexpr uint8_t INC_E = 0x1C;          // Increment register E
    static constexpr uint8_t INC_D = 0x14;          // Increment register D
    static constexpr uint8_t DEC_C = 0x0D;          // Decrement register C
    static constexpr uint8_t CP_E = 0xBB;          // Compare E with A
    static constexpr uint8_t INC_C = 0x0C;          // Increment register C
    static constexpr uint8_t RET_C = 0xD8;          // Return if carry flag is set
    static constexpr uint8_t LD_SP_HL = 0xF9;       // LD SP, HL - Load SP with HL
    static constexpr uint8_t RLCA = 0x07;           // RLCA - Rotate A left through carry
    static constexpr uint8_t STOP = 0x10;
    static constexpr uint8_t LD_HL_E = 0x73;           // LD (HL), E - Load memory at HL with E
    static constexpr uint8_t LD_E_HL = 0x5E;           // LD E, (HL) - Load E with memory at HL
    static constexpr uint8_t LD_H_HL = 0x66;           // LD H, (HL) - Load H with memory at HL
    static constexpr uint8_t INC_SP = 0x33;            // INC SP - Increment stack pointer
    static constexpr uint8_t XOR_L = 0xAD;             // XOR L - XOR L with A
    static constexpr uint8_t OR_B = 0xB0;              // OR B - OR B with A
    static constexpr uint8_t DEC_SP = 0x3B;            // DEC SP - Decrement stack pointer
    static constexpr uint8_t ADD_SP_r8 = 0xE8;         // ADD SP, r8 - Add signed immediate to SP
    static constexpr uint8_t SBC_A_d8 = 0xDE;         // SBC A, d8 - Subtract with carry immediate from A
    static constexpr uint8_t LD_HL_H = 0x74;          // LD (HL), H - Load memory at HL with H
    static constexpr uint8_t LD_HL_L = 0x75;          // LD (HL), L - Load memory at HL with L
    static constexpr uint8_t RET_NZ = 0xC0;           // RET NZ - Return if not zero
    static constexpr uint8_t LD_A_FF00_C = 0xF2;      // LD A, (FF00+C) - Load A from I/O port C
    static constexpr uint8_t LD_FF00_C_A = 0xE2;      // LD (FF00+C), A - Store A to I/O port C
    static constexpr uint8_t OR_D = 0xB2;             // OR D - OR D with A
    static constexpr uint8_t OR_E = 0xB3;             // OR E - OR E with A
    static constexpr uint8_t OR_H = 0xB4;             // OR H - OR H with A
    static constexpr uint8_t OR_L = 0xB5;             // OR L - OR L with A
    static constexpr uint8_t XOR_B = 0xA8;             // XOR B - XOR B with A
    static constexpr uint8_t XOR_D = 0xAA;             // XOR D - XOR D with A
    static constexpr uint8_t XOR_E = 0xAB;             // XOR E - XOR E with A
    static constexpr uint8_t XOR_H = 0xAC;             // XOR H - XOR H with A
    static constexpr uint8_t CP_B = 0xB8;              // CP B - Compare B with A
    static constexpr uint8_t CP_C = 0xB9;              // CP C - Compare C with A
    static constexpr uint8_t CP_D = 0xBA;              // CP D - Compare D with A
    static constexpr uint8_t CP_H = 0xBC;              // CP H - Compare H with A
    static constexpr uint8_t CP_L = 0xBD;              // CP L - Compare L with A
    static constexpr uint8_t CP_HL = 0xBE;             // CP (HL) - Compare memory at HL with A
    static constexpr uint8_t AND_B = 0xA0;             // AND B - AND B with A
    static constexpr uint8_t AND_C = 0xA1;             // AND C - AND C with A
    static constexpr uint8_t AND_D = 0xA2;             // AND D - AND D with A
    static constexpr uint8_t AND_E = 0xA3;             // AND E - AND E with A
    static constexpr uint8_t AND_H = 0xA4;             // AND H - AND H with A
    static constexpr uint8_t AND_L = 0xA5;             // AND L - AND L with A
    static constexpr uint8_t AND_HL = 0xA6;            // AND (HL) - AND memory at HL with A
    static constexpr uint8_t AND_A = 0xA7;             // AND A - AND A with A
    static constexpr uint8_t CP_A = 0xBF;              // CP A - Compare A with A (always zero)
    static constexpr uint8_t ADD_A_B = 0x80;           // ADD A, B - Add B to A
    static constexpr uint8_t ADD_A_D = 0x82;           // ADD A, D - Add D to A
    static constexpr uint8_t ADD_A_E = 0x83;           // ADD A, E - Add E to A
    static constexpr uint8_t ADD_A_H = 0x84;           // ADD A, H - Add H to A
    static constexpr uint8_t ADD_A_L = 0x85;           // ADD A, L - Add L to A
    static constexpr uint8_t ADD_A_HL = 0x86;          // ADD A, (HL) - Add memory at HL to A
    static constexpr uint8_t ADC_A_B = 0x88;           // ADC A, B - Add B to A with carry
    static constexpr uint8_t ADC_A_C = 0x89;           // ADC A, C - Add C to A with carry
    static constexpr uint8_t ADC_A_D = 0x8A;           // ADC A, D - Add D to A with carry
    static constexpr uint8_t ADC_A_E = 0x8B;           // ADC A, E - Add E to A with carry
    static constexpr uint8_t ADC_A_H = 0x8C;           // ADC A, H - Add H to A with carry
    static constexpr uint8_t ADC_A_L = 0x8D;           // ADC A, L - Add L to A with carry
    static constexpr uint8_t ADC_A_HL = 0x8E;          // ADC A, (HL) - Add memory at HL to A with carry
    static constexpr uint8_t ADC_A_A = 0x8F;           // ADC A, A - Add A to A with carry
    static constexpr uint8_t SUB_B = 0x90;             // SUB B - Subtract B from A
    static constexpr uint8_t SUB_C = 0x91;             // SUB C - Subtract C from A
    static constexpr uint8_t SUB_D = 0x92;             // SUB D - Subtract D from A
    static constexpr uint8_t SUB_E = 0x93;             // SUB E - Subtract E from A
    static constexpr uint8_t SUB_H = 0x94;             // SUB H - Subtract H from A
    static constexpr uint8_t SUB_L = 0x95;             // SUB L - Subtract L from A
    static constexpr uint8_t SUB_HL = 0x96;            // SUB (HL) - Subtract memory at HL from A
    static constexpr uint8_t SUB_A = 0x97;             // SUB A - Subtract A from A (always zero)
    static constexpr uint8_t SBC_A_B = 0x98;           // SBC A, B - Subtract B from A with carry
    static constexpr uint8_t SBC_A_C = 0x99;           // SBC A, C - Subtract C from A with carry
    static constexpr uint8_t SBC_A_D = 0x9A;           // SBC A, D - Subtract D from A with carry
    static constexpr uint8_t SBC_A_E = 0x9B;           // SBC A, E - Subtract E from A with carry
    static constexpr uint8_t SBC_A_H = 0x9C;           // SBC A, H - Subtract H from A with carry
    static constexpr uint8_t SBC_A_L = 0x9D;           // SBC A, L - Subtract L from A with carry
    static constexpr uint8_t SBC_A_HL = 0x9E;          // SBC A, (HL) - Subtract memory at HL from A with carry
    static constexpr uint8_t SBC_A_A = 0x9F;           // SBC A, A - Subtract A from A with carry
    static constexpr uint8_t LD_C_d8 = 0x0E;        // LD C, d8 - Load C with immediate
    static constexpr uint8_t LD_DE_d16 = 0x11;     // LD DE, d16 - Load DE with 16-bit immediate
    static constexpr uint8_t JP_a16 = 0xC3;        // JP a16 - Jump to 16-bit address

    // CPU operations
    void reset();                    // Reset CPU to initial state
    int step();                      // Execute one instruction, return cycles consumed
    void push(uint16_t value);       // Push value to stack
    uint16_t pop();                  // Pop value from stack
    
    // System integration
    void setPPU(PPU* ppuRef) { ppu = ppuRef; }           // Set PPU reference
    void setTimer(Timer* timerRef) { timer = timerRef; } // Set Timer reference
    
    // Interrupt handling
    bool checkInterrupts();          // Check and service interrupts
    void serviceInterrupt(uint16_t vector, uint8_t bit); // Service specific interrupt
    
    // Memory access helpers
    uint8_t readByte(uint16_t address) const;
    void writeByte(uint16_t address, uint8_t value);
    uint16_t readWord(uint16_t address) const;
    void writeWord(uint16_t address, uint16_t value);
    
    // Debug methods
    void printRegisters() const;
    void printFlags() const;

    void setInterruptEnable(bool value);
    void updateTimer(int cycles);
    void updatePPUTiming(int cycles);
};