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
    
    // CPU reset complete - ready for testing
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
    // CPU step - no debug output to focus on serial output
    
    // 1. Fetch opcode from memory at PC
    uint8_t opcode = readByte(PC);
    PC++;  // Increment PC to next instruction
    
    // Execute instruction
    std::cout << "At PC=" << std::hex << PC 
          << " opcode=" << (int)opcode 
          << " next two bytes=" 
          << (int)readByte(PC) << " " << (int)readByte(PC+1) 
          << std::endl;

    
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
            // CPU halted
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
                setA(result & 0xFF);
                
                // Set flags correctly
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((aValue & 0x0F) < (value & 0x0F)); // Use original A
                setCarryFlag(aValue < value);                       // Use original A
                std::cout << "[SUB] A=" << std::hex << (int)aValue
          << " - " << (int)value
          << " = " << (int)(result & 0xFF)
          << " | Z=" << getZeroFlag()
          << " N=" << getSubtractFlag()
          << " H=" << getHalfCarryFlag()
          << " C=" << getCarryFlag()
          << std::endl;
            }
            break;
            case ADD_A_d8: // 0xC6
            {
                uint8_t value = readByte(PC++);
                uint8_t a = getA();
                uint16_t result = a + value;

                setA(result & 0xFF);
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((a & 0xF) + (value & 0xF)) > 0xF);
                setCarryFlag(result > 0xFF);
                // ADC instruction completed

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
            
        case 0xB1: // OR C - Bitwise OR between A and C
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue | cValue;
                
                setA(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0x1F: // RRA - Rotate Right A through carry
            {
                uint8_t aValue = getA();
                bool oldCarry = getCarryFlag();
                bool newCarry = (aValue & 0x01) != 0;
                setA(((aValue >> 1) | (oldCarry ? 0x80 : 0)) & 0xFF);
                setZeroFlag(false); // RRA always clears Z flag
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(newCarry);
            }
            break;
            
        case 0xA9: // XOR C - Bitwise XOR between A and C
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue ^ cValue;
                
                setA(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0x05: // DEC B - Decrement register B
            {
                uint8_t bValue = getB();
                uint8_t result = bValue - 1;
                
                setB(result);
                setZeroFlag(result == 0);
                setSubtractFlag(true); // DEC always sets N flag
                setHalfCarryFlag((bValue & 0x0F) == 0x00); // Set if borrow from bit 4
                // Carry flag is not affected by DEC
            }
            break;
            
        case 0x24: // INC H - Increment register H
            {
                uint8_t hValue = getH();
                uint8_t result = hValue + 1;
                
                setH(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((hValue & 0x0F) == 0x0F); // Set if carry from bit 3 to 4
                // Carry flag is not affected by INC
            }
            break;
            
        case 0x2C: // INC L - Increment register L
            {
                uint8_t lValue = getL();
                uint8_t result = lValue + 1;
                
                setL(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((lValue & 0x0F) == 0x0F); // Set if carry from bit 3 to 4
                // Carry flag is not affected by INC
            }
            break;
            
        case 0xE6: // AND d8 - Bitwise AND between A and immediate value
            {
                uint8_t value = readByte(PC);
                PC++; // Increment PC for 8-bit immediate value
                uint8_t aValue = getA();
                uint8_t result = aValue & value;
                
                setA(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true); // AND always sets H flag
                setCarryFlag(false);
            }
            break;
            
            
        case 0xB7: // OR A, A
            {
                // OR A, A is same as A (no change to A)
                setZeroFlag(getA() == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0x46: // LD B, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setB(memValue);
            }
            break;
            
        case 0x2D: // DEC L
            {
                uint8_t lValue = getL();
                uint8_t result = lValue - 1;
                setL(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((lValue & 0x0F) == 0);
            }
            break;
            
        case 0x25: // DEC H
            {
                uint8_t hValue = getH();
                uint8_t result = hValue - 1;
                setH(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((hValue & 0x0F) == 0);
            }
            break;
            
        case 0x72: // LD (HL), D
            {
                uint16_t hlValue = getHL();
                uint8_t dValue = getD();
                writeByte(hlValue, dValue);
            }
            break;
            
        case 0x71: // LD (HL), C
            {
                uint16_t hlValue = getHL();
                uint8_t cValue = getC();
                writeByte(hlValue, cValue);
            }
            break;
            
        case 0x70: // LD (HL), B
            {
                uint16_t hlValue = getHL();
                uint8_t bValue = getB();
                writeByte(hlValue, bValue);
            }
            break;
            
        case 0xD0: // RET NC
            {
                if (!getCarryFlag()) {
                    // Pop return address from stack
                    uint8_t low = readByte(SP);
                    SP++;
                    uint8_t high = readByte(SP);
                    SP++;
                    PC = (high << 8) | low;
                }
            }
            break;
            
        case 0xC8: // RET Z
            {
                if (getZeroFlag()) {
                    // Pop return address from stack
                    uint8_t low = readByte(SP);
                    SP++;
                    uint8_t high = readByte(SP);
                    SP++;
                    PC = (high << 8) | low;
                }
            }
            break;
            
        case 0xB6: // OR (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t aValue = getA();
                uint8_t result = aValue | memValue;
                setA(result);
                
                // Set flags: Z=1 if result is 0, N=0, H=0, C=0
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0x35: // DEC (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = memValue - 1;
                writeByte(hlValue, result);
                
                // Set flags: Z=1 if result is 0, N=1, H=1 if borrow from bit 4
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((memValue & 0x0F) == 0x00);
            }
            break;
            
        case 0x6E: // LD L, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setL(memValue);
            }
            break;
            
        case 0x1D: // DEC E
            {
                uint8_t eValue = getE();
                uint8_t result = eValue - 1;
                setE(result);
                
                // Set flags: Z=1 if result is 0, N=1, H=1 if borrow from bit 4
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((eValue & 0x0F) == 0x00);
            }
            break;
            
        case 0x4E: // LD C, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setC(memValue);
            }
            break;
            
        case 0x56: // LD D, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setD(memValue);
            }
            break;
            
        case 0xAE: // XOR (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t aValue = getA();
                uint8_t result = aValue ^ memValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0xEE: // XOR d8
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t result = aValue ^ value;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
            }
            break;
            
        case 0xCE: // ADC A, d8 - Add with carry immediate
            {
                uint8_t value = readByte(PC);
                PC++; // Increment PC for 8-bit value
                uint8_t aValue = getA();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + value + carry;
                
                setA(result & 0xFF);
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue & 0x0F) + (value & 0x0F) + carry) > 0x0F);
                setCarryFlag(result > 0xFF);
            }
            break;
            
        case 0xE9: // JP (HL) - Jump to address in HL register
            {
                uint16_t hlValue = getHL();
                PC = hlValue;
            }
            break;
            
        default:{
            std::cerr << "Unimplemented opcode: 0x" << std::hex 
            << (int)opcode << " at PC=0x" << (PC-1) << std::endl;
            std::exit(1); 
            }
            break;
    }
}