#include "cpu.h"
#include <iostream>
#include <iomanip>

// Constructor
CPU::CPU(Memory& mem) : memory(mem) {
    reset();
}

// Reset CPU to initial state
void CPU::reset() {
    // Set registers to post-BIOS defaults
    AF.pair = 0x01B0;  // A=0x01, F=0xB0 (Z=1, N=0, H=1, C=1)
    BC.pair = 0x0013;  // B=0x00, C=0x13
    DE.pair = 0x00D8;  // D=0x00, E=0xD8
    HL.pair = 0x014D;  // H=0x01, L=0x4D
    SP = 0xFFFE;       // Stack pointer
    PC = 0x0100;       // Program counter (ROM entry point)
    
    // Debug: Print CPU state at reset
    std::cout << "=== CPU RESET STATE ===" << std::endl;
    std::cout << "A=0x" << std::hex << static_cast<int>(getA()) << " (expected: 0x01)" << std::endl;
    std::cout << "F=0x" << std::hex << static_cast<int>(AF.low) << " (expected: 0xB0)" << std::endl;
    std::cout << "BC=0x" << std::hex << getBC() << " (expected: 0x0013)" << std::endl;
    std::cout << "DE=0x" << std::hex << getDE() << " (expected: 0x00D8)" << std::endl;
    std::cout << "HL=0x" << std::hex << getHL() << " (expected: 0x014D)" << std::endl;
    std::cout << "SP=0x" << std::hex << SP << " (expected: 0xFFFE)" << std::endl;
    std::cout << "PC=0x" << std::hex << PC << " (expected: 0x0100)" << std::endl;
    
    // Debug: Print first 32 bytes of ROM from entry point
    std::cout << "\n=== ROM ENTRY POINT (0x0100-0x011F) ===" << std::endl;
    for (int i = 0; i < 32; i++) {
        if (i % 16 == 0) {
            std::cout << "0x" << std::hex << (0x0100 + i) << ": ";
        }
        std::cout << std::hex << static_cast<int>(memory.read(0x0100 + i)) << " ";
        if (i % 16 == 15) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    
    // Print first 64 bytes of memory from 0xFF00
    std::cout << "\n=== MEMORY 0xFF00-0xFF3F ===" << std::endl;
    for (int i = 0; i < 64; i++) {
        if (i % 16 == 0) {
            std::cout << "0x" << std::hex << (0xFF00 + i) << ": ";
        }
        std::cout << std::hex << static_cast<int>(memory.read(0xFF00 + i)) << " ";
        if (i % 16 == 15) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    
    // Check specific registers
    std::cout << "\n=== REGISTER CHECKS ===" << std::endl;
    std::cout << "LCDC (0xFF40) = 0x" << std::hex << static_cast<int>(memory.read(0xFF40)) << " (expected: 0x91)" << std::endl;
    std::cout << "BGP (0xFF47) = 0x" << std::hex << static_cast<int>(memory.read(0xFF47)) << " (expected: 0xFC)" << std::endl;
    
    // Check if values match expected
    bool allMatch = true;
    if (getA() != 0x01) { std::cout << "ERROR: A register mismatch!" << std::endl; allMatch = false; }
    if (AF.low != 0xB0) { std::cout << "ERROR: F register mismatch!" << std::endl; allMatch = false; }
    if (getBC() != 0x0013) { std::cout << "ERROR: BC register mismatch!" << std::endl; allMatch = false; }
    if (getDE() != 0x00D8) { std::cout << "ERROR: DE register mismatch!" << std::endl; allMatch = false; }
    if (getHL() != 0x014D) { std::cout << "ERROR: HL register mismatch!" << std::endl; allMatch = false; }
    if (SP != 0xFFFE) { std::cout << "ERROR: SP register mismatch!" << std::endl; allMatch = false; }
    if (PC != 0x0100) { std::cout << "ERROR: PC register mismatch!" << std::endl; allMatch = false; }
    if (memory.read(0xFF40) != 0x91) { std::cout << "ERROR: LCDC register mismatch!" << std::endl; allMatch = false; }
    if (memory.read(0xFF47) != 0xFC) { std::cout << "ERROR: BGP register mismatch!" << std::endl; allMatch = false; }
    
    if (allMatch) {
        std::cout << "✓ All registers match expected post-BIOS defaults!" << std::endl;
    } else {
        std::cout << "✗ Some registers don't match expected values!" << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

// Memory access helpers
uint8_t CPU::readByte(uint16_t address) const {
    return memory.read(address);
}

void CPU::writeByte(uint16_t address, uint8_t value) {
    memory.write(address, value);
}

uint16_t CPU::readWord(uint16_t address) const {
    return memory.read_word(address);
}

void CPU::writeWord(uint16_t address, uint16_t value) {
    memory.write_word(address, value);
}

// Flag operations
bool CPU::getFlag(uint8_t flag) const {
    return (AF.low & flag) != 0;
}

void CPU::setFlag(uint8_t flag, bool value) {
    if (value) {
        AF.low |= flag;
    } else {
        AF.low &= ~flag;
    }
}

// Stack operations
void CPU::push(uint16_t value) {
    SP -= 2;
    writeWord(SP, value);
}

uint16_t CPU::pop() {
    uint16_t value = readWord(SP);
    SP += 2;
    return value;
}

// Debug methods
void CPU::printRegisters() const {
    std::cout << std::hex << std::uppercase << std::setfill('0')
              << "AF: 0x" << std::setw(4) << getAF()
              << " BC: 0x" << std::setw(4) << getBC()
              << " DE: 0x" << std::setw(4) << getDE()
              << " HL: 0x" << std::setw(4) << getHL()
              << " PC: 0x" << std::setw(4) << PC
              << " SP: 0x" << std::setw(4) << SP << std::endl;
    printFlags();
}

void CPU::printFlags() const {
    std::cout << "Flags: "
              << "Z:" << (getZeroFlag() ? "1" : "0")
              << " N:" << (getSubtractFlag() ? "1" : "0")
              << " H:" << (getHalfCarryFlag() ? "1" : "0")
              << " C:" << (getCarryFlag() ? "1" : "0")
              << std::endl;
}

void CPU::setInterruptEnable(bool value) {
    interruptEnable = value;
}

void CPU::step() {
    // CPU tracer - log first few thousand cycles
    static int cycleCount = 0;
    static constexpr int MAX_TRACE_CYCLES = 5000;
    
    if (cycleCount < MAX_TRACE_CYCLES) {
        std::cout << "Cycle " << cycleCount << ": PC=0x" << std::hex << PC 
                  << " A=0x" << static_cast<int>(getA()) 
                  << " BC=0x" << getBC() 
                  << " DE=0x" << getDE() 
                  << " HL=0x" << getHL() 
                  << " SP=0x" << SP << std::endl;
        cycleCount++;
    }
    
    // 1. Fetch opcode from memory at PC
    uint8_t opcode = readByte(PC);
    PC++;  // Increment PC to next instruction
    
    // Log instruction execution
    if (cycleCount < MAX_TRACE_CYCLES) {
        std::cout << "  Executing: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
    }
    
    // 2. Execute the instruction
    switch (opcode) {
        case NOP:  // 0x00: No operation
            break;
            
        case LD_BC_NN:  // 0x01: Load 16-bit immediate into BC
            {
                uint16_t value = readWord(PC);
                setBC(value);
                PC += 2;  // Increment PC by 2 for 16-bit value
            }
            break;
            
        case LD_B_N:   // 0x06: Load 8-bit immediate into B
            {
                uint8_t value = readByte(PC);
                setB(value);
                PC++;    // Increment PC for 8-bit value
            }
            break;
            
        case JP_NN:    // 0xC3: Jump to address NN
            {
                uint16_t address = readWord(PC);
                PC = address;  // Set PC to jump address
            }
            break;
            
        case HALT:     // 0x76: Halt CPU until interrupt
            // For now, just print a message
            std::cout << "CPU HALT instruction at PC=" << std::hex << PC << std::endl;
            break;
        
        // Load immediate value into register
        case LD_C_N: { uint8_t value = readByte(PC); setC(value); PC++; break; }
        case LD_D_N: { uint8_t value = readByte(PC); setD(value); PC++; break; }
        case LD_E_N: { uint8_t value = readByte(PC); setE(value); PC++; break; }
        case LD_H_N: { uint8_t value = readByte(PC); setH(value); PC++; break; }
        case LD_L_N: { uint8_t value = readByte(PC); setL(value); PC++; break; }
        case LD_A_N: { uint8_t value = readByte(PC); setA(value); PC++; break; }

        // Load register into B
        case LD_B_B: setB(getB()); break;
        case LD_B_C: setB(getC()); break;
        case LD_B_D: setB(getD()); break;
        case LD_B_E: setB(getE()); break;
        case LD_B_H: setB(getH()); break;
        case LD_B_L: setB(getL()); break;
        case LD_B_A: setB(getA()); break;

        // Load register into C
        case LD_C_B: setC(getB()); break;
        case LD_C_C: setC(getC()); break;
        case LD_C_D: setC(getD()); break;
        case LD_C_E: setC(getE()); break;
        case LD_C_H: setC(getH()); break;
        case LD_C_L: setC(getL()); break;
        case LD_C_A: setC(getA()); break;

        // Load register into D
        case LD_D_B: setD(getB()); break;
        case LD_D_C: setD(getC()); break;
        case LD_D_D: setD(getD()); break;
        case LD_D_E: setD(getE()); break;
        case LD_D_H: setD(getH()); break;
        case LD_D_L: setD(getL()); break;
        case LD_D_A: setD(getA()); break;

        // Load register into E
        case LD_E_B: setE(getB()); break;
        case LD_E_C: setE(getC()); break;
        case LD_E_D: setE(getD()); break;
        case LD_E_E: setE(getE()); break;
        case LD_E_H: setE(getH()); break;
        case LD_E_L: setE(getL()); break;
        case LD_E_A: setE(getA()); break;

        // Load register into H
        case LD_H_B: setH(getB()); break;
        case LD_H_C: setH(getC()); break;
        case LD_H_D: setH(getD()); break;
        case LD_H_E: setH(getE()); break;
        case LD_H_H: setH(getH()); break;
        case LD_H_L: setH(getL()); break;
        case LD_H_A: setH(getA()); break;

        // Load register into L
        case LD_L_B: setL(getB()); break;
        case LD_L_C: setL(getC()); break;
        case LD_L_D: setL(getD()); break;
        case LD_L_E: setL(getE()); break;
        case LD_L_H: setL(getH()); break;
        case LD_L_L: setL(getL()); break;
        case LD_L_A: setL(getA()); break;

        // Load register into A
        case LD_A_B: setA(getB()); break;
        case LD_A_C: setA(getC()); break;
        case LD_A_D: setA(getD()); break;
        case LD_A_E: setA(getE()); break;
        case LD_A_H: setA(getH()); break;
        case LD_A_L: setA(getL()); break;
        case LD_A_A: setA(getA()); break;

        //PUSH
        case PUSH_AF: // 0xF5
            std::cout << "PUSH AF executed!" << std::endl;
            push(getAF()); 
            break;
        case PUSH_BC: // 0xC5
            push(getBC()); 
            break;
        case PUSH_DE: // 0xD5
            push(getDE()); 
            break;
        case PUSH_HL: // 0xE5
            push(getHL()); 
            break;

        //POP
        case POP_AF: // 0xF1
            setAF(pop()); 
            break;
        case POP_BC: // 0xC1
            setBC(pop()); 
            break;
        case POP_DE: // 0xD1
            setDE(pop()); 
            break;
        case POP_HL: // 0xE1
            setHL(pop()); 
            break;
        
        //CALL
        case CALL_NN: // 0xCD
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                push(PC); //Save return address
                PC = address; //Jump to function
            }
            break;
        case CALL_NZ: // 0xC4
            {
                    uint16_t address = readWord(PC);
                    PC += 2; //Move past the address
                    if (!getZeroFlag()) {
                    push(PC); //Save return address
                    PC = address; //Jump to function
                } 
            }
            break;
        case CALL_Z: // 0xCC
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                if (getZeroFlag()) {
                    push(PC); //Save return address
                    PC = address; //Jump to function
                }
            }
            break;
        case CALL_NC: // 0xD4
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                if (!getCarryFlag()) {
                    push(PC); //Save return address
                    PC = address; //Jump to function
                }
            }
            break;
        case CALL_C: // 0xDC
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                if (getCarryFlag()) {
                    push(PC); //Save return address
                    PC = address; //Jump to function
                }
            }
            break;
        case LD_SP_NN: // 0x31: Load 16-bit immediate into SP
            {
                uint16_t value = readWord(PC);
                setSP(value);
                PC += 2; //Move past the address
            }
            break;
        case LD_NN_A: // 0xEA
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                writeByte(address, getA()); //store A at address
            }
            break;
        case DI: // 0xF3
            setInterruptEnable(false);
            break;
        case JR_R8: // 0x18
            {
                int8_t offset = readByte(PC);
                PC += 1; //Move past the offset
                PC += offset; //Jump to address
            }
            break;
        case RET: // 0xC9
            PC = pop(); //Return to address
            break;
        case LD_HL_NN: // 0x21
            {
                uint16_t value = readWord(PC);
                setHL(value);
                PC += 2; //Move past the address
            }
            break;
            case SUB_d8: // 0xD6
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();        // Store original A
                uint8_t result = aValue - value;
                setA(result);
                
                // Set flags correctly
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((aValue & 0x0F) < (value & 0x0F)); // Use original A
                setCarryFlag(aValue < value);                       // Use original A
            }
            break;
            case ADD_A_d8: // 0xC6
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t result = aValue + value;

                setA(result);
                setZeroFlag(result == 0);
                setHalfCarryFlag((aValue & 0x0F) + (value & 0x0F) > 0x0F);
                setSubtractFlag(false);
            }
            break;
            case DEC_A: // 0x3D
            {
                uint8_t aValue = getA();
                uint8_t result = aValue - 1;
                setA(result);
                
                // Set flags
                setZeroFlag(result == 0);
                setSubtractFlag(true);   // DEC is subtraction
                setHalfCarryFlag((aValue & 0x0F) < 1);
                // Carry flag is NOT affected by DEC
            }
            break;
            case INC_A: // 0x3C
            {
                uint8_t aValue = getA();
                uint8_t result = aValue + 1;
                setA(result);
                
                // Set flags
                setZeroFlag(result == 0);
                setSubtractFlag(false);  // INC is addition
                setHalfCarryFlag((aValue & 0x0F) + 1 > 0x0F);
                // Carry flag is NOT affected by INC
            }
            break;
            case CP_d8: // 0xFE
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t result = aValue - value;  // Don't store result!
                
                // Set flags based on comparison
                setZeroFlag(result == 0);
                setSubtractFlag(true);   // CP is subtraction
                setHalfCarryFlag((aValue & 0x0F) < (value & 0x0F));
                setCarryFlag(aValue < value);
            }
            break;
            case LD_HLplus_A: // 0x22
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
                setHL(hlValue + 1);
            }
            break;
            case LD_DE_NN: // 0x11
            {
                uint16_t value = readWord(PC);
                setDE(value);
                PC += 2;
            }
            break;
            case JR_NZ_R8: // 0x20
            {
                int8_t offset = readByte(PC);
                PC++;
                if (!getZeroFlag()) {
                    PC += offset;
                }
            }
            break;
            case LDH_A_a8: // 0xF0
            {
                uint8_t ioAddr = readByte(PC);
                PC++;
                // For now, return 0xFF for most I/O registers (no input pressed)
                // This will allow the game to continue past the input loop
                if (ioAddr == 0x00) {  // P1/JOYP register
                    setA(0xFF);  // No buttons pressed
                } else {
                    setA(0x00);  // Default for other I/O registers
                }
            }
            break;
            case XOR_A: // 0xAF
            {
                uint8_t aValue = getA();
                uint8_t result = aValue ^ aValue;  // XOR A with A = 0
                setA(result);
                
                // Set flags
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            case LDH_a8_A: // 0xE0
            {
                uint8_t ioAddr = readByte(PC);
                PC++;
                // For now, just ignore I/O writes (store to nowhere)
                // This prevents crashes when the game tries to configure I/O
            }
            break;
            case LD_A_HLplus: // 0x2A
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
                setHL(hlValue + 1);
            }
            break;
            case JR_Z_R8: // 0x28
            {
                int8_t offset = readByte(PC);
                PC++;
                if (getZeroFlag()) {
                    PC += offset;
                }
            }
            break;
            case INC_BC: // 0x03
            {
                uint16_t bcValue = getBC();
                setBC(bcValue + 1);
                // INC BC doesn't affect flags
            }
            break;
            case ADD_A_A: // 0x87
            {
                uint8_t aValue = getA();
                uint8_t result = aValue + aValue;
                setA(result);
                
                // Set flags
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((aValue & 0x0F) + (aValue & 0x0F) > 0x0F);
                setCarryFlag(result < aValue);
            }
            break;
            case CB_PREFIX: // 0xCB
            {
                // Read the next byte to get the CB instruction
                uint8_t cbOpcode = readByte(PC);
                PC++;
                
                // Handle CB-prefix instructions
                switch (cbOpcode) {
                    // Rotate and shift instructions
                    case 0x07: // RLC A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x80) != 0;
                        setA(((aValue << 1) | (carry ? 1 : 0)) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case 0x17: // RL A
                    {
                        uint8_t aValue = getA();
                        bool oldCarry = getCarryFlag();
                        bool newCarry = (aValue & 0x80) != 0;
                        setA(((aValue << 1) | (oldCarry ? 1 : 0)) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(newCarry);
                    }
                    break;
                    case 0x0F: // RRC A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x01) != 0;
                        setA(((aValue >> 1) | (carry ? 0x80 : 0)) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case 0x1F: // RR A
                    {
                        uint8_t aValue = getA();
                        bool oldCarry = getCarryFlag();
                        bool newCarry = (aValue & 0x01) != 0;
                        setA(((aValue >> 1) | (oldCarry ? 0x80 : 0)) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(newCarry);
                    }
                    break;
                    case 0x27: // SLA A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x80) != 0;
                        setA((aValue << 1) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case 0x2F: // SRA A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x01) != 0;
                        setA(((aValue >> 1) | (aValue & 0x80)) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case 0x3F: // SRL A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x01) != 0;
                        setA((aValue >> 1) & 0xFF);
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case 0x37: // SWAP A
                    {
                        uint8_t aValue = getA();
                        setA(((aValue & 0x0F) << 4) | ((aValue & 0xF0) >> 4));
                        setZeroFlag(getA() == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(false);
                    }
                    break;
                    // Bit operations
                    case 0x47: // BIT 0, A
                        setZeroFlag((getA() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x4F: // BIT 1, A
                        setZeroFlag((getA() & 0x02) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x57: // BIT 2, A
                        setZeroFlag((getA() & 0x04) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x5F: // BIT 3, A
                        setZeroFlag((getA() & 0x08) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x67: // BIT 4, A
                        setZeroFlag((getA() & 0x10) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x6F: // BIT 5, A
                        setZeroFlag((getA() & 0x20) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x77: // BIT 6, A
                        setZeroFlag((getA() & 0x40) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x7F: // BIT 7, A
                        setZeroFlag((getA() & 0x80) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case 0x87: // RES 0, A
                        setA(getA() & 0xFE);
                        break;
                    case 0x8F: // RES 1, A
                        setA(getA() & 0xFD);
                        break;
                    case 0x97: // RES 2, A
                        setA(getA() & 0xFB);
                        break;
                    case 0x9F: // RES 3, A
                        setA(getA() & 0xF7);
                        break;
                    case 0xA7: // RES 4, A
                        setA(getA() & 0xEF);
                        break;
                    case 0xAF: // RES 5, A
                        setA(getA() & 0xDF);
                        break;
                    case 0xB7: // RES 6, A
                        setA(getA() & 0xBF);
                        break;
                    case 0xBF: // RES 7, A
                        setA(getA() & 0x7F);
                        break;
                    case 0xC7: // SET 0, A
                        setA(getA() | 0x01);
                        break;
                    case 0xCF: // SET 1, A
                        setA(getA() | 0x02);
                        break;
                    case 0xD7: // SET 2, A
                        setA(getA() | 0x04);
                        break;
                    case 0xDF: // SET 3, A
                        setA(getA() | 0x08);
                        break;
                    case 0xE7: // SET 4, A
                        setA(getA() | 0x10);
                        break;
                    case 0xEF: // SET 5, A
                        setA(getA() | 0x20);
                        break;
                    case 0xF7: // SET 6, A
                        setA(getA() | 0x40);
                        break;
                    case 0xFF: // SET 7, A
                        setA(getA() | 0x80);
                        break;
                    default:
                        // For other CB instructions, just continue (no-op for now)
                        break;
                }
            }
            break;
            case LD_A_HL: // 0x7E
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
            }
            break;
            case LD_HL_A: // 0x77
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
            }
            break;
            case ADD_HL_BC: // 0x09
            {
                uint32_t hlValue = getHL();
                uint32_t bcValue = getBC();
                uint32_t result = hlValue + bcValue;
                setHL(result & 0xFFFF);
                
                // Set flags
                setSubtractFlag(false);
                setHalfCarryFlag((hlValue & 0xFFF) + (bcValue & 0xFFF) > 0xFFF);
                setCarryFlag(result > 0xFFFF);
                // Zero flag unchanged
            }
            break;
            case DEC_BC: // 0x0B
            {
                uint16_t bcValue = getBC();
                setBC(bcValue - 1);
                // DEC BC doesn't affect flags
            }
            break;
            case LD_A_NN: // 0xFA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                uint8_t value = readByte(address);
                setA(value);
            }
            break;
            case JR_C_R8: // 0x38
            {
                int8_t offset = readByte(PC);
                PC++;
                if (getCarryFlag()) {
                    PC += offset;
                }
            }
            break;
            case JR_NC_R8: // 0x30
            {
                int8_t offset = readByte(PC);
                PC++;
                if (!getCarryFlag()) {
                    PC += offset;
                }
            }
            break;
            case ADD_HL_DE: // 0x19
            {
                uint32_t hlValue = getHL();
                uint32_t deValue = getDE();
                uint32_t result = hlValue + deValue;
                setHL(result & 0xFFFF);
                
                // Set flags
                setSubtractFlag(false);
                setHalfCarryFlag((hlValue & 0xFFF) + (deValue & 0xFFF) > 0xFFF);
                setCarryFlag(result > 0xFFFF);
                // Zero flag unchanged
            }
            break;
            case ADD_HL_HL: // 0x29
            {
                uint32_t hlValue = getHL();
                uint32_t result = hlValue + hlValue;
                setHL(result & 0xFFFF);
                
                // Set flags
                setSubtractFlag(false);
                setHalfCarryFlag((hlValue & 0xFFF) + (hlValue & 0xFFF) > 0xFFF);
                setCarryFlag(result > 0xFFFF);
                // Zero flag unchanged
            }
            break;
            case LD_HL_D8: // 0x36
            {
                uint8_t value = readByte(PC);
                PC++;
                uint16_t hlValue = getHL();
                writeByte(hlValue, value);
            }
            break;
            case LD_A_BC: // 0x0A
            {
                uint16_t bcValue = getBC();
                uint8_t value = readByte(bcValue);
                setA(value);
            }
            break;
            case LD_BC_A: // 0x02
            {
                uint16_t bcValue = getBC();
                writeByte(bcValue, getA());
            }
            break;
            case LD_HLminus_A: // 0x32
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
                setHL(hlValue - 1);
            }
            break;
            case LD_A_HLminus: // 0x3A
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
                setHL(hlValue - 1);
            }
            break;
            case LD_NN_SP: // 0x08
            {
                uint16_t address = readWord(PC);
                PC += 2;
                writeWord(address, SP);
            }
            break;
            case JP_C_NN: // 0xDA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (getCarryFlag()) {
                    PC = address;
                }
            }
            break;
            case JP_NC_NN: // 0xD2
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (!getCarryFlag()) {
                    PC = address;
                }
            }
            break;
            case JP_Z_NN: // 0xCA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (getZeroFlag()) {
                    PC = address;
                }
            }
            break;
            case JP_NZ_NN: // 0xC2
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (!getZeroFlag()) {
                    PC = address;
                }
            }
            break;
            case EI: // 0xFB
            {
                setInterruptEnable(true);
            }
            break;
            case RETI: // 0xD9
            {
                PC = pop();
                setInterruptEnable(true);
            }
            break;
            case RST_00: // 0xC7
            {
                push(PC);
                PC = 0x00;
            }
            break;
            case RST_08: // 0xCF
            {
                push(PC);
                PC = 0x08;
            }
            break;
            case RST_10: // 0xD7
            {
                push(PC);
                PC = 0x10;
            }
            break;
            case RST_18: // 0xDF
            {
                push(PC);
                PC = 0x18;
            }
            break;
            case RST_20: // 0xE7
            {
                push(PC);
                PC = 0x20;
            }
            break;
            case RST_28: // 0xEF
            {
                push(PC);
                PC = 0x28;
            }
            break;
            case RST_30: // 0xF7
            {
                push(PC);
                PC = 0x30;
            }
            break;
            case RST_38: // 0xFF
            {
                push(PC);
                PC = 0x38;
            }
            break;
            case INC_DE: // 0x13
            {
                uint16_t deValue = getDE();
                setDE(deValue + 1);
            }
            break;
            case INC_HL: // 0x23
            {
                uint16_t hlValue = getHL();
                setHL(hlValue + 1);
            }
            break;
            case DEC_DE: // 0x1B
            {
                uint16_t deValue = getDE();
                setDE(deValue - 1);
            }
            break;
            case DEC_HL: // 0x2B
            {
                uint16_t hlValue = getHL();
                setHL(hlValue - 1);
            }
            break;
            case ADD_HL_SP: // 0x39
            {
                uint32_t hlValue = getHL();
                uint32_t spValue = SP;
                uint32_t result = hlValue + spValue;
                setHL(result & 0xFFFF);
                
                // Set flags
                setSubtractFlag(false);
                setHalfCarryFlag((hlValue & 0xFFF) + (spValue & 0xFFF) > 0xFFF);
                setCarryFlag(result > 0xFFFF);
                // Zero flag unchanged
            }
            break;
            case DAA: // 0x27
            {
                // Decimal adjust A - complex BCD arithmetic
                uint8_t aValue = getA();
                uint8_t result = aValue;
                
                if (!getSubtractFlag()) {
                    // Addition
                    if (getHalfCarryFlag() || (aValue & 0x0F) > 9) {
                        result += 0x06;
                    }
                    if (getCarryFlag() || aValue > 0x9F) {
                        result += 0x60;
                        setCarryFlag(true);
                    }
                } else {
                    // Subtraction
                    if (getHalfCarryFlag()) {
                        result -= 0x06;
                    }
                    if (getCarryFlag()) {
                        result -= 0x60;
                    }
                }
                
                setA(result);
                setZeroFlag(result == 0);
                setHalfCarryFlag(false);
            }
            break;
            case CPL: // 0x2F
            {
                setA(~getA());
                setSubtractFlag(true);
                setHalfCarryFlag(true);
            }
            break;
            case SCF: // 0x37
            {
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(true);
            }
            break;
            case CCF: // 0x3F
            {
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(!getCarryFlag());
            }
            break;
            case LD_DE_A: // 0x12
            {
                uint16_t deValue = getDE();
                writeByte(deValue, getA());
            }
            break;
            case LD_A_DE: // 0x1A
            {
                uint16_t deValue = getDE();
                uint8_t value = readByte(deValue);
                setA(value);
            }
            break;
            
        default:
            std::cout << "UNKNOWN OPCODE: 0x" << std::hex << static_cast<int>(opcode) 
                      << " at PC=0x" << PC-1 
                      << " A=0x" << static_cast<int>(getA())
                      << " BC=0x" << getBC()
                      << " DE=0x" << getDE()
                      << " HL=0x" << getHL() << std::endl;
            break;
    }
}