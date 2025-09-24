#include "cpu.h"
#include <iostream>
#include <iomanip>

CPU::CPU(Memory& mem) : memory(mem) {
    reset();
}

bool CPU::getFlag(uint8_t flag) const {
    return (F & flag) != 0;
}

void CPU::setFlag(uint8_t flag, bool value) {
    if (value) {
        F |= flag;
    } else {
        F &= ~flag;
    }
}

void CPU::reset() {
    // Initialize registers to Game Boy boot values
    A = 0x01;
    B = 0x00;
    C = 0x13;
    D = 0x00;
    E = 0xD8;
    F = 0xB0;  // Flags: Z=1, N=0, H=1, C=1
    H = 0x01;
    L = 0x4D;
    PC = 0x0100;  // Game Boy starts execution at 0x0100
    SP = 0xFFFE;  // Stack starts at 0xFFFE (top of high RAM)
}

void CPU::step() {
    // Fetch instruction from memory
    uint8_t opcode = readByte(PC);
    
    // For now, just increment PC (we'll implement actual instructions later)
    PC++;
    
    // Basic instruction handling (we'll expand this later)
    switch (opcode) {
        case 0x00: // NOP - No operation
            // Do nothing
            break;
        case 0x18: // JR r8 - Jump relative
            {
                int8_t offset = readByte(PC);
                PC += offset;
                PC++; // Skip the offset byte
            }
            break;
        default:
            // For now, just increment PC for unknown instructions
            break;
    }
}

void CPU::push(uint16_t value) {
    SP -= 2;  // Stack grows downward
    writeWord(SP, value);
}

uint16_t CPU::pop() {
    uint16_t value = readWord(SP);
    SP += 2;
    return value;
}

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

void CPU::printRegisters() const {
    std::cout << "=== CPU Registers ===" << std::endl;
    std::cout << "A: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)A << std::endl;
    std::cout << "B: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)B << std::endl;
    std::cout << "C: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)C << std::endl;
    std::cout << "D: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)D << std::endl;
    std::cout << "E: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)E << std::endl;
    std::cout << "F: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)F << std::endl;
    std::cout << "H: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)H << std::endl;
    std::cout << "L: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)L << std::endl;
    std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << PC << std::endl;
    std::cout << "SP: 0x" << std::hex << std::setw(4) << std::setfill('0') << SP << std::endl;
    std::cout << "AF: 0x" << std::hex << std::setw(4) << std::setfill('0') << getAF() << std::endl;
    std::cout << "BC: 0x" << std::hex << std::setw(4) << std::setfill('0') << getBC() << std::endl;
    std::cout << "DE: 0x" << std::hex << std::setw(4) << std::setfill('0') << getDE() << std::endl;
    std::cout << "HL: 0x" << std::hex << std::setw(4) << std::setfill('0') << getHL() << std::endl;
    std::cout << std::dec << std::endl;
}

void CPU::printFlags() const {
    std::cout << "=== CPU Flags ===" << std::endl;
    std::cout << "Zero: " << (getZeroFlag() ? "1" : "0") << std::endl;
    std::cout << "Subtract: " << (getSubtractFlag() ? "1" : "0") << std::endl;
    std::cout << "Half Carry: " << (getHalfCarryFlag() ? "1" : "0") << std::endl;
    std::cout << "Carry: " << (getCarryFlag() ? "1" : "0") << std::endl;
    std::cout << std::endl;
}

