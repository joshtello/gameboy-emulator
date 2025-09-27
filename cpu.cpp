#include "cpu.h"
#include <iostream>
#include <iomanip>

// Constructor
CPU::CPU(Memory& mem) : memory(mem) {
    reset();
}

// Reset CPU to initial state
void CPU::reset() {
    AF.pair = 0;
    BC.pair = 0;
    DE.pair = 0;
    HL.pair = 0;
    SP = 0xFFFE;  // Stack starts at top of memory
    PC = 0x100;   // Game Boy starts execution at 0x100
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
    // 1. Fetch opcode from memory at PC
    uint8_t opcode = readByte(PC);
    PC++;  // Increment PC to next instruction
    
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
            
            
        default:
            std::cout << "Unknown opcode: 0x" << std::hex << static_cast<int>(opcode) 
                      << " at PC=0x" << PC-1 << std::endl;
            break;
    }
}