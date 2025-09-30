#include "cpu.h"
#include "ppu.h"
#include <iostream>
#include <iomanip>

// Constructor
CPU::CPU(Memory& mem) : memory(mem) {
    // Don't call reset() here - let it be called explicitly after BIOS initialization
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
    cycleCount = 0;     // Reset cycle counter
    
    // CRITICAL: Disable interrupts until game enables them
    IME = false;        // Interrupt Master Enable OFF
    pendingIME = false; // No pending EI instruction
    
    // Reset debug system
    DebugLogger::reset();
    
    std::cout << "CPU: Reset complete - IME=" << (IME ? "ON" : "OFF") << std::endl;
    
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
    // Ensure bits 0-3 of F register are always 0
    AF.low &= 0xF0;
}

// Stack operations
void CPU::push(uint16_t value) {
    SP--;
    writeByte(SP, value & 0xFF);        // Low byte first
    SP--;
    writeByte(SP, (value >> 8) & 0xFF); // High byte second
}

uint16_t CPU::pop() {
    uint8_t low = readByte(SP++);
    uint8_t high = readByte(SP++);
    return (high << 8) | low;
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


void CPU::updateTimer(int cycles) {
    // Update DIV register (increments every 256 cycles)
    static int divCounter = 0;
    divCounter += cycles;
    if (divCounter >= 256) {
        divCounter -= 256;
        uint8_t div = memory.read(0xFF04);
        memory.write(0xFF04, div + 1);
    }
    
    // Update TIMA register based on TAC
    uint8_t tac = memory.read(0xFF07);
    static int timerDebugCount = 0;
    if (timerDebugCount < 10) {
        std::cout << "TIMER: TAC=0x" << std::hex << (int)tac << " enabled=" << ((tac & 0x04) ? "YES" : "NO") << std::endl;
        timerDebugCount++;
    }
    if (tac & 0x04) {  // Timer enabled
        static int timaCounter = 0;
        int frequency = 0;
        
        switch (tac & 0x03) {
            case 0: frequency = 1024; break;  // 4096 Hz (1024 cycles)
            case 1: frequency = 16; break;    // 262144 Hz (16 cycles)  
            case 2: frequency = 64; break;    // 65536 Hz (64 cycles)
            case 3: frequency = 256; break;   // 16384 Hz (256 cycles)
        }
        
        timaCounter += cycles;
        if (timaCounter >= frequency) {
            timaCounter -= frequency;
            uint8_t tima = memory.read(0xFF05);
            if (tima == 0xFF) {
                // Timer overflow - reset to TMA and trigger interrupt
                uint8_t tma = memory.read(0xFF06);
                memory.write(0xFF05, tma);
                
                // Set timer interrupt flag (bit 2 of IF register at 0xFF0F)
                uint8_t if_reg = memory.read(0xFF0F);
                memory.write(0xFF0F, if_reg | 0x04);
                std::cout << "TIMER INTERRUPT: TIMA overflow, IF=0x" << std::hex << (int)(if_reg | 0x04) << std::endl;
            } else {
                memory.write(0xFF05, tima + 1);
            }
        }
    }
}

    int CPU::step() {
        // 1. Fetch opcode from memory at PC
        uint8_t opcode = readByte(PC);
        PC++;  // Always increment PC after fetching opcode
        uint16_t currentPC = PC;  // Store current PC
        
        // Global debug print for every instruction (ENABLED for Pokemon debugging)
        static int instructionCount = 0;
        if (instructionCount < 50) {
            std::cout << "STEP: PC=0x" << std::hex << currentPC 
                      << " opcode=0x" << (int)opcode << std::endl;
            instructionCount++;
        }
        
        // Debug: Print opcodes around PC=0x150-0x160
        if (currentPC >= 0x150 && currentPC <= 0x160) {
            std::cout << "PC=0x" << std::hex << currentPC << " opcode=0x" << (int)opcode;
            if (opcode == 0x76) std::cout << " (HALT)";
            if (opcode == 0xFB) std::cout << " (EI)";
            if (opcode == 0xF3) std::cout << " (DI)";
            std::cout << std::endl;
        }
        
        // Debug: Print opcodes at PC=0x4000-0x4010
        if (currentPC >= 0x4000 && currentPC < 0x4010) {
            std::cout << "PC=0x" << std::hex << currentPC << " opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x20C-0x210 to find the stuck instruction
        if (currentPC >= 0x20C && currentPC <= 0x210) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x151 to see Pokemon Red stuck instruction
        if (currentPC == 0x151) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << " A=" << (int)getA() << " - About to enter switch statement" << std::endl;
        }
        
        // Debug: Print opcode at PC=0x18-0x20 to see what happens after RST_18
        if (currentPC >= 0x18 && currentPC <= 0x20) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x170-0x190 to see what's happening
        if (currentPC >= 0x170 && currentPC <= 0x190) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Check if CPU is stuck at same PC for too long
        static uint16_t lastPC = 0;
        static int pcCount = 0;
        if (currentPC == lastPC) {
            pcCount++;
            if (pcCount > 10) {  // If stuck at same PC for more than 10 instructions
                std::cout << "*** CPU STUCK at PC=" << std::hex << currentPC 
                          << " for " << pcCount << " instructions! ***" << std::endl;
                std::cout << "Next instruction: 0x" << std::hex << (int)opcode << std::endl;
                pcCount = 0;  // Reset counter to avoid spam
            }
        } else {
            lastPC = currentPC;
            pcCount = 0;
        }
        
        // Debug: Print opcode at PC=0x215 to see instruction timing ROM stuck instruction
        if (currentPC == 0x215) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x200 to see next stuck instruction
        if (currentPC == 0x200) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x203 to see next stuck instruction
        if (currentPC == 0x203) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x205 to see next stuck instruction
        if (currentPC == 0x205) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x86a (Pac-Man stuck instruction)
        if (currentPC == 0x86a) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x86c (next stuck instruction)
        if (currentPC == 0x86c) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x86e (next stuck instruction)
        if (currentPC == 0x86e) {
            std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
        }
        
        // Debug: Print opcode at PC=0x3bb (stuck in loop after VRAM write)
        if (currentPC == 0x3bb) {
            static int count_3bb = 0;
            if (count_3bb++ < 5) {
                std::cout << "At PC=0x" << std::hex << currentPC << ", opcode=0x" << (int)opcode << std::endl;
            }
        }
        
        // Log instruction execution if debug is enabled
        DebugLogger::logInstruction(currentPC, opcode);
        
        // Get cycle count from cycle table
        int cycles = CYCLE_TABLE[opcode];

    
    // 2. Execute the instruction
    // Debug: Log opcode at PC=0x151
    if (currentPC == 0x151) {
        std::cout << "SWITCH: Processing opcode=0x" << std::hex << (int)opcode << " at PC=0x" << currentPC << std::endl;
    }
    
    // Debug: Log when we enter switch statement for 0xFE
    if (opcode == 0xFE) {
        std::cout << "ENTERING SWITCH with opcode=0x" << std::hex << (int)opcode << std::endl;
    }
    
    switch (opcode) {
        case NOP:  // 0x00: No operation
            cycles = 4; // NOP takes 4 cycles
            break;
            
        case LD_BC_NN:  // 0x01: Load 16-bit immediate into BC
            {
                uint16_t value = readWord(PC);
                setBC(value);
                PC += 2;  // Increment PC by 2 for 16-bit value
                cycles = 12; // LD r16,nn takes 12 cycles
            }
            break;
            
        case LD_B_N:   // 0x06: Load 8-bit immediate into B
            {
                uint8_t value = readByte(PC);
                setB(value);
                // PC already incremented after opcode fetch, so increment again for immediate value
                PC++;
                cycles = 8; // LD r,n takes 8 cycles
            }
            break;
            
        case JP_NN:    // 0xC3: Jump to address NN
            {
                uint16_t address = readWord(PC);
                PC += 2;
                PC = address;  // Set PC to jump address
                cycles = 16; // JP takes 16 cycles
            }
            break;
            
        case HALT:     // 0x76: Halt CPU until interrupt
            {
                uint8_t IE = memory.read(0xFFFF);
                uint8_t IF = memory.read(0xFF0F);
                
                std::cout << "HALT at PC=0x" << std::hex << PC 
                          << " IME=" << (IME ? "1" : "0")
                          << " IE=0x" << (int)IE 
                          << " IF=0x" << (int)IF << std::endl;
                
                // HALT bug: If IE=0 and IF=0, HALT behaves like NOP
                if (!IME && (IE & IF) == 0) {
                    PC++;  // Advance PC like NOP
                    cycles = 4;
                    std::cout << "HALT bug: advancing PC like NOP" << std::endl;
                } else {
                    // Normal HALT behavior - don't advance PC
                    cycles = 4;
                    std::cout << "HALT: CPU halted, waiting for interrupt" << std::endl;
                }
            }
            break;
            
        case EI:       // 0xFB: Enable interrupts
            pendingIME = true;  // Enable interrupts after next instruction
            cycles = 4; // EI takes 4 cycles
            std::cout << "EI: pendingIME set, will enable after next instruction" << std::endl;
            break;
            
        case DI:       // 0xF3: Disable interrupts
            IME = false;
            pendingIME = false;
            cycles = 4; // DI takes 4 cycles
            break;
        
        // Load immediate value into register
        case LD_C_N: { uint8_t value = readByte(PC); setC(value); PC++; cycles = 8; break; }
        case LD_D_N: { uint8_t value = readByte(PC); setD(value); PC++; cycles = 8; break; }
        case LD_E_N: { uint8_t value = readByte(PC); setE(value); PC++; cycles = 8; break; }
        case LD_H_N: { uint8_t value = readByte(PC); setH(value); PC++; cycles = 8; break; }
        case LD_L_N: { uint8_t value = readByte(PC); setL(value); PC++; cycles = 8; break; }
        case LD_A_N: { uint8_t value = readByte(PC); setA(value); PC++; cycles = 8; break; }

        // Load register into B
        case LD_B_B: setB(getB()); cycles = 4; break;
        case LD_B_C: setB(getC()); cycles = 4; break;
        case LD_B_D: setB(getD()); cycles = 4; break;
        case LD_B_E: setB(getE()); cycles = 4; break;
        case LD_B_H: setB(getH()); cycles = 4; break;
        case LD_B_L: setB(getL()); cycles = 4; break;
        case LD_B_A: setB(getA()); cycles = 4; break;

        // Load register into C
        case LD_C_B: setC(getB()); cycles = 4; break;
        case LD_C_C: setC(getC()); cycles = 4; break;
        case LD_C_D: setC(getD()); cycles = 4; break;
        case LD_C_E: setC(getE()); cycles = 4; break;
        case LD_C_H: setC(getH()); cycles = 4; break;
        case LD_C_L: setC(getL()); cycles = 4; break;
        case LD_C_A: setC(getA()); cycles = 4; break;

        // Load register into D
        case LD_D_B: setD(getB()); cycles = 4; break;
        case LD_D_C: setD(getC()); cycles = 4; break;
        case LD_D_D: setD(getD()); cycles = 4; break;
        case LD_D_E: setD(getE()); cycles = 4; break;
        case LD_D_H: setD(getH()); cycles = 4; break;
        case LD_D_L: setD(getL()); cycles = 4; break;
        case LD_D_A: setD(getA()); cycles = 4; break;

        // Load register into E
        case LD_E_B: setE(getB()); cycles = 4; break;
        case LD_E_C: setE(getC()); cycles = 4; break;
        case LD_E_D: setE(getD()); cycles = 4; break;
        case LD_E_E: setE(getE()); cycles = 4; break;
        case LD_E_H: setE(getH()); cycles = 4; break;
        case LD_E_L: setE(getL()); cycles = 4; break;
        case LD_E_A: setE(getA()); cycles = 4; break;

        // Load register into H
        case LD_H_B: setH(getB()); cycles = 4; break;
        case LD_H_C: setH(getC()); cycles = 4; break;
        case LD_H_D: setH(getD()); cycles = 4; break;
        case LD_H_E: setH(getE()); cycles = 4; break;
        case LD_H_H: setH(getH()); cycles = 4; break;
        case LD_H_L: setH(getL()); cycles = 4; break;
        case LD_H_A: setH(getA()); cycles = 4; break;

        // Load register into L
        case LD_L_B: setL(getB()); cycles = 4; break;
        case LD_L_C: setL(getC()); cycles = 4; break;
        case LD_L_D: setL(getD()); cycles = 4; break;
        case LD_L_E: setL(getE()); cycles = 4; break;
        case LD_L_H: setL(getH()); cycles = 4; break;
        case LD_L_L: setL(getL()); cycles = 4; break;
        case LD_L_A: setL(getA()); cycles = 4; break;

        // Load register into A
        case LD_A_B: setA(getB()); cycles = 4; break;
        case LD_A_C: setA(getC()); cycles = 4; break;
        case LD_A_D: setA(getD()); cycles = 4; break;
        case LD_A_E: setA(getE()); cycles = 4; break;
        case LD_A_H: setA(getH()); cycles = 4; break;
        case LD_A_L: setA(getL()); cycles = 4; break;
        case LD_A_A: setA(getA()); cycles = 4; break;

        //PUSH
        case PUSH_AF: push(getAF()); cycles = 16; break;
        case PUSH_BC: push(getBC()); cycles = 16; break;
        case PUSH_DE: push(getDE()); cycles = 16; break;
        case PUSH_HL: push(getHL()); cycles = 16; break;

        //POP
        case POP_AF:  setAF(pop());  cycles = 12; break;
        case POP_BC:  setBC(pop());  cycles = 12; break;
        case POP_DE:  setDE(pop());  cycles = 12; break;
        case POP_HL:  setHL(pop());  cycles = 12; break;
        
        //CALL
        case CALL_NN: // 0xCD
        {
            uint16_t address = readWord(PC);
            PC += 2;               // Move past operand
            push(PC);              // Push return address
            PC = address;          // Jump
            cycles = 24;
        }
        break;

        case CALL_NZ: // 0xC4
        {
            uint16_t address = readWord(PC);
            PC += 2;
            if (!getZeroFlag()) {
                push(PC);
                PC = address;
                cycles = 24;
            } else {
                cycles = 12;
            }
        }
        break;

        case CALL_Z: // 0xCC
        {
            uint16_t address = readWord(PC);
            PC += 2;
            if (getZeroFlag()) {
                push(PC);
                PC = address;
                cycles = 24;
            } else {
                cycles = 12;
            }
        }
        break;

        case CALL_NC: // 0xD4
        {
            uint16_t address = readWord(PC);
            PC += 2;
            if (!getCarryFlag()) {
                push(PC);
                PC = address;
                cycles = 24;
            } else {
                cycles = 12;
            }
        }
        break;

        case CALL_C: // 0xDC
        {
            uint16_t address = readWord(PC);
            PC += 2;
            if (getCarryFlag()) {
                push(PC);
                PC = address;
                cycles = 24;
            } else {
                cycles = 12;
            }
        }
        break;
        case LD_SP_NN: // 0x31: Load 16-bit immediate into SP
            {
                uint16_t value = readWord(PC);
                setSP(value);
                PC += 2; //Move past the address
                cycles = 12; // LD r16,nn takes 12 cycles
            }
            break;
        case LD_NN_A: // 0xEA
            {
                uint16_t address = readWord(PC);
                PC += 2; //Move past the address
                writeByte(address, getA()); //store A at address
                cycles = 16; // LD (a16),A takes 16 cycles
            }
            break;
        case JR_R8: // 0x18
            {
                int8_t offset = static_cast<int8_t>(readByte(PC++));
                PC += offset; //Jump to address
                cycles = 12; // JR always taken
            }
            break;
        case RET: // 0xC9
            {
                PC = pop();
                cycles = 16;
            }
            break;
        case LD_HL_NN: // 0x21
            {
                uint16_t value = readWord(PC);
                setHL(value);
                PC += 2; //Move past the address
                cycles = 12; // LD r16,nn takes 12 cycles
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
                setHalfCarryFlag(((aValue ^ value ^ result) & 0x10) != 0);
                setCarryFlag(aValue < value);
                cycles = 8;            // SUB d8 takes 8 cycles
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
                setHalfCarryFlag(((a ^ value ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 8; // ADD A,n takes 8 cycles
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
                setHalfCarryFlag((aValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                // Carry flag is NOT affected by DEC
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
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
                setHalfCarryFlag((aValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is NOT affected by INC
                cycles = 4; // INC r takes 4 cycles
            }
            break;
            case CP_d8: // 0xFE
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t result = aValue - value;  // Don't store result!
                
                std::cout << "CP d8 (0xFE): A=" << std::hex << (int)aValue << " compare_with=" << (int)value << " result=" << (int)result << " Z=" << (result == 0 ? "1" : "0") << std::endl;
                
                // Set flags based on comparison
                setZeroFlag(result == 0);
                setSubtractFlag(true);   // CP is subtraction
                setHalfCarryFlag((aValue & 0x0F) < (value & 0x0F));
                setCarryFlag(aValue < value);
                cycles = 8; // CP n takes 8 cycles
            }
            break;
            case LD_HLplus_A: // 0x22
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
                setHL(hlValue + 1);
                cycles = 8; // LD HL+,A takes 8 cycles
            }
            break;
            case LD_DE_NN: // 0x11
            {
                uint16_t value = readWord(PC);
                setDE(value);
                PC += 2;
                cycles = 12; // LD r16,nn takes 12 cycles
            }
            break;
            case JR_NZ_R8: // 0x20
            {
                int8_t offset = static_cast<int8_t>(readByte(PC++));
                if (!getZeroFlag()) {
                    PC += offset;
                    cycles = 12; // JR taken
                } else {
                    cycles = 8; // JR not taken
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
                cycles = 12; // LDH A,(a8) takes 12 cycles
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
                cycles = 4; // XOR A takes 4 cycles
                std::cout << "XOR A executed, PC now=" << std::hex << PC << std::endl;
            }
            break;
            case LDH_a8_A: // 0xE0
            {
                uint8_t ioAddr = readByte(PC);
                PC++;
                // For now, just ignore I/O writes (store to nowhere)
                // This prevents crashes when the game tries to configure I/O
                cycles = 12; // LDH (a8),A takes 12 cycles
            }
            break;
            case LD_A_HLplus: // 0x2A
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
                setHL(hlValue + 1);
                PC++; // Advance program counter
                cycles = 8; // LD A,HL+ takes 8 cycles
            }
            break;
            case JR_Z_R8: // 0x28
            {
                int8_t offset = static_cast<int8_t>(readByte(PC++));
                if (getZeroFlag()) {
                    PC += offset;
                    cycles = 12; // JR taken
                } else {
                    cycles = 8; // JR not taken
                }
            }
            break;
            case INC_BC: // 0x03
            {
                uint16_t bcValue = getBC();
                setBC(bcValue + 1);
                // INC BC doesn't affect flags
                cycles = 8; // INC r16 takes 8 cycles
            }
            break;
            case ADD_A_A: // 0x87
            {
                uint8_t aValue = getA();
                uint16_t result = aValue + aValue;
                setA(result & 0xFF);
                
                // Set flags
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ aValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
            }
            break;
            case CB_PREFIX: // 0xCB
            {
                // Read the next byte to get the CB instruction
                uint8_t cbOpcode = readByte(PC);
                PC++;
                
                // Update cycle count for CB opcode from CB cycle table
                cycles = CB_CYCLE_TABLE[cbOpcode];
                
                // Handle CB-prefix instructions
                switch (cbOpcode) {
                    // Rotate and shift instructions
                    case RLC_A: // RLC A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x80) != 0;
                        setA(((aValue << 1) | (carry ? 1 : 0)) & 0xFF);
                        setZeroFlag(false); // RLCA always clears Z
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case RL_A: // RL A
                    {
                        uint8_t aValue = getA();
                        bool oldCarry = getCarryFlag();
                        bool newCarry = (aValue & 0x80) != 0;
                        setA(((aValue << 1) | (oldCarry ? 1 : 0)) & 0xFF);
                        setZeroFlag(false); // RL A always clears Z
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(newCarry);
                    }
                    break;
                    case RRC_A: // RRC A
                    {
                        uint8_t aValue = getA();
                        bool carry = (aValue & 0x01) != 0;
                        setA(((aValue >> 1) | (carry ? 0x80 : 0)) & 0xFF);
                        setZeroFlag(false); // RRC A always clears Z
                        setSubtractFlag(false);
                        setHalfCarryFlag(false);
                        setCarryFlag(carry);
                    }
                    break;
                    case SLA_A: // SLA A
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
                    case SRA_A: // SRA A
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
                    case SRL_A: // SRL A
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
                    case SWAP_A: // SWAP A
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
                    case BIT_0_A: // BIT 0, A
                        setZeroFlag((getA() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_1_A: // BIT 1, A
                        setZeroFlag((getA() & 0x02) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_2_A: // BIT 2, A
                        setZeroFlag((getA() & 0x04) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_3_A: // BIT 3, A
                        setZeroFlag((getA() & 0x08) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_4_A: // BIT 4, A
                        setZeroFlag((getA() & 0x10) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_5_A: // BIT 5, A
                        setZeroFlag((getA() & 0x20) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_6_A: // BIT 6, A
                        setZeroFlag((getA() & 0x40) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_7_A: // BIT 7, A
                        setZeroFlag((getA() & 0x80) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                        
                    // BIT operations for other registers
                    case BIT_0_B: // BIT 0, B
                        setZeroFlag((getB() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_C: // BIT 0, C
                        setZeroFlag((getC() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_D: // BIT 0, D
                        setZeroFlag((getD() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_E: // BIT 0, E
                        setZeroFlag((getE() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_H: // BIT 0, H
                        setZeroFlag((getH() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_L: // BIT 0, L
                        setZeroFlag((getL() & 0x01) == 0);
                        setSubtractFlag(false);
                        setHalfCarryFlag(true);
                        break;
                    case BIT_0_HL: // BIT 0, (HL)
                        {
                            uint16_t hlValue = getHL();
                            uint8_t memValue = readByte(hlValue);
                            setZeroFlag((memValue & 0x01) == 0);
                            setSubtractFlag(false);
                            setHalfCarryFlag(true);
                        }
                        break;
                    case RES_0_A: // RES 0, A
                        setA(getA() & 0xFE);
                        break;
                    case RES_1_A: // RES 1, A
                        setA(getA() & 0xFD);
                        break;
                    case RES_2_A: // RES 2, A
                        setA(getA() & 0xFB);
                        break;
                    case RES_3_A: // RES 3, A
                        setA(getA() & 0xF7);
                        break;
                    case RES_4_A: // RES 4, A
                        setA(getA() & 0xEF);
                        break;
                    case RES_5_A: // RES 5, A
                        setA(getA() & 0xDF);
                        break;
                    case RES_6_A: // RES 6, A
                        setA(getA() & 0xBF);
                        break;
                    case RES_7_A: // RES 7, A
                        setA(getA() & 0x7F);
                        break;
                    case SET_0_A: // SET 0, A
                        setA(getA() | 0x01);
                        break;
                    case SET_1_A: // SET 1, A
                        setA(getA() | 0x02);
                        break;
                    case SET_2_A: // SET 2, A
                        setA(getA() | 0x04);
                        break;
                    case SET_3_A: // SET 3, A
                        setA(getA() | 0x08);
                        break;
                    case SET_4_A: // SET 4, A
                        setA(getA() | 0x10);
                        break;
                    case SET_5_A: // SET 5, A
                        setA(getA() | 0x20);
                        break;
                    case SET_6_A: // SET 6, A
                        setA(getA() | 0x40);
                        break;
                    case SET_7_A: // SET 7, A
                        setA(getA() | 0x80);
                        break;
                    default:
                        // For other CB instructions, just continue (no-op for now)
                        break;
                }
                if (cbOpcode == BIT_0_HL) cycles = 16;
                else cycles = 8;
            }
            break;
            case LD_A_HL: // 0x7E
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
                cycles = 8; // LD A,(HL) takes 8 cycles
            }
            break;
            case LD_HL_A: // 0x77
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
                cycles = 8; // LD (HL),A takes 8 cycles
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
                cycles = 8; // ADD HL,rr takes 8 cycles
            }
            break;
            case DEC_BC: // 0x0B
            {
                uint16_t bcValue = getBC();
                setBC(bcValue - 1);
                // DEC BC doesn't affect flags
                PC++; // Advance program counter
                cycles = 8; // DEC r16 takes 8 cycles
            }
            break;
            case LD_A_NN: // 0xFA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                uint8_t value = readByte(address);
                setA(value);
                cycles = 16; // LD A,(a16) takes 16 cycles
            }
            break;
            case JR_C_R8: // 0x38
            {
                int8_t offset = static_cast<int8_t>(readByte(PC++));
                if (getCarryFlag()) {
                    PC += offset;
                    cycles = 12; // JR taken
                } else {
                    cycles = 8; // JR not taken
                }
            }
            break;
            case JR_NC_R8: // 0x30
            {
                int8_t offset = static_cast<int8_t>(readByte(PC++));
                if (!getCarryFlag()) {
                    PC += offset;
                    cycles = 12; // JR taken
                } else {
                    cycles = 8; // JR not taken
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
                cycles = 8; // ADD HL,rr takes 8 cycles
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
                cycles = 8; // ADD HL,rr takes 8 cycles
            }
            break;
            case LD_HL_D8: // 0x36
            {
                uint8_t value = readByte(PC);
                PC++;
                uint16_t hlValue = getHL();
                writeByte(hlValue, value);
                cycles = 12; // LD (HL),n takes 12 cycles
            }
            break;
            case LD_A_BC: // 0x0A
            {
                uint16_t bcValue = getBC();
                uint8_t value = readByte(bcValue);
                setA(value);
                cycles = 8; // LD A,(BC) takes 8 cycles
            }
            break;
            case LD_BC_A: // 0x02
            {
                uint16_t bcValue = getBC();
                writeByte(bcValue, getA());
                cycles = 8; // LD (BC),A takes 8 cycles
            }
            break;
            case LD_HLminus_A: // 0x32
            {
                uint16_t hlValue = getHL();
                writeByte(hlValue, getA());
                setHL(hlValue - 1);
                cycles = 8; // LD HL-,A takes 8 cycles
            }
            break;
            case LD_A_HLminus: // 0x3A
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setA(value);
                setHL(hlValue - 1);
                cycles = 8; // LD A,HL- takes 8 cycles
            }
            break;
            case LD_NN_SP: // 0x08
            {
                uint16_t address = readWord(PC);
                PC += 2;
                writeWord(address, SP);
                cycles = 20; // LD (a16),SP takes 20 cycles
            }
            break;
            case JP_C_NN: // 0xDA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (getCarryFlag()) {
                    PC = address;
                    cycles = 16; // JP taken
                } else {
                    cycles = 12; // JP not taken
                }
            }
            break;
            case JP_NC_NN: // 0xD2
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (!getCarryFlag()) {
                    PC = address;
                    cycles = 16; // JP taken
                } else {
                    cycles = 12; // JP not taken
                }
            }
            break;
            case JP_Z_NN: // 0xCA
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (getZeroFlag()) {
                    PC = address;
                    cycles = 16; // JP taken
                } else {
                    cycles = 12; // JP not taken
                }
            }
            break;
            case JP_NZ_NN: // 0xC2
            {
                uint16_t address = readWord(PC);
                PC += 2;
                if (!getZeroFlag()) {
                    PC = address;
                    cycles = 16; // JP taken
                } else {
                    cycles = 12; // JP not taken
                }
            }
            break;
            case RETI: // 0xD9
            {
                PC = pop();
                IME = true;        // Enable interrupts immediately
                pendingIME = false;
                cycles = 16;
            }
            break;
            case RST_00: push(PC); PC = 0x00; cycles = 16; break;
            case RST_08: push(PC); PC = 0x08; cycles = 16; break;
            case RST_10: push(PC); PC = 0x10; cycles = 16; break;
            case RST_18: 
                std::cout << "RST_18 executed, pushing PC=" << std::hex << PC << " jumping to 0x18" << std::endl;
                push(PC); PC = 0x18; cycles = 16; 
                break;
            case RST_20: push(PC); PC = 0x20; cycles = 16; break;
            case RST_28: push(PC); PC = 0x28; cycles = 16; break;
            case RST_30: push(PC); PC = 0x30; cycles = 16; break;
            case RST_38: 
                std::cout << "RST_38 executed, pushing PC=" << std::hex << PC << " jumping to 0x38" << std::endl;
                std::cout << "Memory[0x38] = 0x" << std::hex << (int)memory.read(0x38) << std::endl;
                push(PC); PC = 0x38; cycles = 16; 
                break;
            case INC_DE: // 0x13
            {
                uint16_t deValue = getDE();
                setDE(deValue + 1);
                cycles = 8; // INC r16 takes 8 cycles
            }
            break;
            case INC_HL: // 0x23
            {
                uint16_t hlValue = getHL();
                setHL(hlValue + 1);
                cycles = 8; // INC r16 takes 8 cycles
            }
            break;
            case DEC_DE: // 0x1B
            {
                uint16_t deValue = getDE();
                setDE(deValue - 1);
                cycles = 8; // DEC r16 takes 8 cycles
            }
            break;
            case DEC_HL: // 0x2B
            {
                uint16_t hlValue = getHL();
                setHL(hlValue - 1);
                cycles = 8; // DEC r16 takes 8 cycles
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
                cycles = 8; // ADD HL,rr takes 8 cycles
            }
            break;
            case DAA: // 0x27
            {
                uint8_t a = getA();
                uint8_t correction = 0;
                bool c = getCarryFlag();

                if (!getSubtractFlag()) { // After addition
                    if (getHalfCarryFlag() || (a & 0x0F) > 9) {
                        correction |= 0x06;
                    }
                    if (c || a > 0x99) {
                        correction |= 0x60;
                        c = true;
                    }
                    a += correction;
                } else { // After subtraction
                    if (getHalfCarryFlag()) correction |= 0x06;
                    if (c) correction |= 0x60;
                    a -= correction;
                }

                setA(a);

                // Flags
                setZeroFlag(a == 0);
                setHalfCarryFlag(false);
                setCarryFlag(c);
                // N flag unchanged
                cycles = 4; // DAA takes 4 cycles
            }
            break;
            case CPL: // 0x2F
            {
                setA(~getA());
                setSubtractFlag(true);
                setHalfCarryFlag(true);
                PC++; // Advance program counter
                cycles = 4; // CPL takes 4 cycles
            }
            break;
            case SCF: // 0x37
            {
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(true);
                cycles = 4; // SCF takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            case CCF: // 0x3F
            {
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(!getCarryFlag());
                cycles = 4; // CCF takes 4 cycles
            }
            break;
            case LD_DE_A: // 0x12
            {
                uint16_t deValue = getDE();
                writeByte(deValue, getA());
                cycles = 8; // LD (DE),A takes 8 cycles
            }
            break;
            case LD_A_DE: // 0x1A
            {
                uint16_t deValue = getDE();
                uint8_t value = readByte(deValue);
                setA(value);
                cycles = 8; // LD A,(DE) takes 8 cycles
            }
            break;
            
        case OR_C: // OR C - Bitwise OR between A and C
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue | cValue;
                
                setA(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case RRA: // RRA - Rotate Right A through carry
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
            
            
        case RL_A: // RL A - Rotate Left A through carry
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
            
        case RRC_A: // RRC A - Rotate Right Circular A
            {
                uint8_t aValue = getA();
                bool carry = (aValue & 0x01) != 0;
                setA(((aValue >> 1) | (carry ? 0x80 : 0)) & 0xFF);
                setZeroFlag(getA() == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(carry);
                cycles = 4; // RRC A takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case XOR_C: // XOR C - Bitwise XOR between A and C
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue ^ cValue;
                
                setA(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // XOR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_B: // DEC B - Decrement register B
            {
                uint8_t bValue = getB();
                uint8_t result = bValue - 1;
                
                setB(result);
                setZeroFlag(result == 0);
                setSubtractFlag(true); // DEC always sets N flag
                setHalfCarryFlag((bValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                // Carry flag is not affected by DEC
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_D: // DEC D - Decrement register D
            {
                uint8_t dValue = getD();
                uint8_t result = dValue - 1;
                
                setD(result);
                setZeroFlag(result == 0);
                setSubtractFlag(true); // DEC always sets N flag
                setHalfCarryFlag((dValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                // Carry flag is not affected by DEC
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case INC_H: // INC H - Increment register H
            {
                uint8_t hValue = getH();
                uint8_t result = hValue + 1;
                
                setH(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((hValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case INC_L: // INC L - Increment register L
            {
                uint8_t lValue = getL();
                uint8_t result = lValue + 1;
                
                setL(result);
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((lValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case AND_d8: // AND d8 - Bitwise AND between A and immediate value
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
                cycles = 8; // AND d8 takes 8 cycles
            }
            break;
            
            
        case OR_A: // OR A, A
            {
                // OR A, A is same as A (no change to A)
                setZeroFlag(getA() == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case LD_B_HL: // LD B, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setB(memValue);
                cycles = 8; // LD B,(HL) takes 8 cycles
            }
            break;
            
        case DEC_L: // DEC L
            {
                uint8_t lValue = getL();
                uint8_t result = lValue - 1;
                setL(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((lValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_H: // DEC H
            {
                uint8_t hValue = getH();
                uint8_t result = hValue - 1;
                setH(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((hValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case LD_HL_D: // LD (HL), D
            {
                uint16_t hlValue = getHL();
                uint8_t dValue = getD();
                writeByte(hlValue, dValue);
                cycles = 8; // LD (HL),r takes 8 cycles
            }
            break;
            
        case LD_HL_C: // LD (HL), C
            {
                uint16_t hlValue = getHL();
                uint8_t cValue = getC();
                writeByte(hlValue, cValue);
                cycles = 8; // LD (HL),r takes 8 cycles
            }
            break;
            
        case LD_HL_B: // LD (HL), B
            {
                uint16_t hlValue = getHL();
                uint8_t bValue = getB();
                writeByte(hlValue, bValue);
                cycles = 8; // LD (HL),r takes 8 cycles
            }
            break;
            
        case RET_NC:
            {
                if (!getCarryFlag()) {
                    PC = pop();
                    cycles = 20;
                } else {
                    cycles = 8;  // Just fall through, no PC++
                }
            }
            break;
            
        case RET_Z:
            {
                if (getZeroFlag()) {
                    PC = pop();
                    cycles = 20;
                } else {
                    cycles = 8;
                }
            }
            break;
            
        case OR_HL: // OR (HL)
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
                cycles = 8; // OR (HL) takes 8 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_HL_mem: // DEC (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = memValue - 1;
                writeByte(hlValue, result);
                
                // Set flags: Z=1 if result is 0, N=1, H=1 if borrow from bit 4
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((memValue & 0x0F) == 0x00);
                cycles = 12; // DEC (HL) takes 12 cycles
            }
            break;
            
        case LD_L_HL: // LD L, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setL(memValue);
                cycles = 8; // LD r,(HL) takes 8 cycles
            }
            break;
            
        case DEC_E: // DEC E
            {
                uint8_t eValue = getE();
                uint8_t result = eValue - 1;
                setE(result);
                
                // Set flags: Z=1 if result is 0, N=1, H=1 if borrow from bit 4
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((eValue & 0x0F) == 0x00);
                cycles = 4; // DEC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case LD_C_HL: // LD C, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setC(memValue);
                cycles = 8; // LD C,(HL) takes 8 cycles
            }
            break;
            
        case LD_D_HL: // LD D, (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                setD(memValue);
                cycles = 8; // LD D,(HL) takes 8 cycles
            }
            break;
            
        case XOR_HL: // XOR (HL)
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
                cycles = 8; // XOR (HL) takes 8 cycles
                PC++; // Advance program counter
            }
            break;
            
        case XOR_d8: // XOR d8
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
                cycles = 8; // XOR d8 takes 8 cycles
            }
            break;
            
        case ADC_A_d8: // ADC A, d8 - Add with carry immediate
            {
                uint8_t value = readByte(PC);
                PC++; // Increment PC for 8-bit value
                uint8_t aValue = getA();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + value + carry;
                
                setA(result & 0xFF);
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ value ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 8; // ADC A,r takes 8 cycles
            }
            break;
            
        case JP_HL: // JP (HL) - Jump to address in HL register
            {
                uint16_t hlValue = getHL();
                PC = hlValue;
                cycles = 4; // JP (HL) takes 4 cycles
            }
            break;
            
        case OR_d8: // OR d8 - OR A with immediate value
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t result = aValue | value;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 8; // OR d8 takes 8 cycles
            }
            break;
            
        case INC_B: // INC B - Increment register B
            {
                uint8_t bValue = getB();
                uint8_t result = bValue + 1;
                setB(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((bValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case INC_HL_mem: // INC (HL) - Increment memory at (HL)
            {
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = memValue + 1;
                writeByte(hlValue, result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((memValue & 0x0F) == 0x0F);
                // Carry flag is not affected by INC
                cycles = 12; // INC (HL) takes 12 cycles
            }
            break;
            
        case SUB_C: // SUB C - Subtract C from A
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue - cValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ cValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < cValue);
                cycles = 4; // SUB r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADD_A_C: // ADD A, C - Add C to A
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint16_t result = aValue + cValue;
                setA(result & 0xFF);
                
                setZeroFlag((result & 0xFF) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ cValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
            }
            break;
            
        case LD_HL_SP_r8: // LD HL, SP+r8 - Load HL with SP + signed immediate
            {
                int8_t offset = static_cast<int8_t>(readByte(PC));
                PC++;
                uint16_t spValue = SP;
                uint16_t result = spValue + offset;
                setHL(result);
                
                // Set flags based on the addition
                setZeroFlag(false);
                setSubtractFlag(false);
                setHalfCarryFlag(((spValue & 0x0F) + (offset & 0x0F)) > 0x0F);
                setCarryFlag(((spValue & 0xFF) + (offset & 0xFF)) > 0xFF);
                cycles = 12; // LD HL,SP+r8 takes 12 cycles
            }
            break;
            
        case INC_E: // INC E - Increment register E
            {
                uint8_t eValue = getE();
                uint8_t result = eValue + 1;
                setE(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((eValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case INC_D: // INC D - Increment register D
            {
                uint8_t dValue = getD();
                uint8_t result = dValue + 1;
                setD(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((dValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_C: // DEC C - Decrement register C
            {
                uint8_t cValue = getC();
                uint8_t result = cValue - 1;
                setC(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag((cValue & 0x0F) == 0x00); // Half-carry if low nibble was 0x0
                // Carry flag is not affected by DEC
                cycles = 4; // DEC r takes 4 cycles
            }
            break;
            
        case CP_E: // CP E - Compare E with A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t result = aValue - eValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ eValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < eValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case INC_C: // INC C - Increment register C
            {
                uint8_t cValue = getC();
                uint8_t result = cValue + 1;
                setC(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag((cValue & 0x0F) == 0x0F); // Half-carry if low nibble was 0xF
                // Carry flag is not affected by INC
                cycles = 4; // INC r takes 4 cycles
            }
            break;
            
        case RET_C:
            {
                if (getCarryFlag()) {
                    PC = pop();
                    cycles = 20;
                } else {
                    cycles = 8;
                }
            }
            break;
            
        case LD_SP_HL: // LD SP, HL - Load SP with HL
            {
                SP = getHL();
                cycles = 8; // LD SP,HL takes 8 cycles
            }
            break;
            
        case RLCA: // RLCA - Rotate A left through carry
            {
                uint8_t aValue = getA();
                bool carry = (aValue & 0x80) != 0;
                uint8_t result = (aValue << 1) | (carry ? 1 : 0);
                setA(result);
                
                setZeroFlag(false);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(carry);
                cycles = 4; // RLCA takes 4 cycles
            }
            break;
            
            
        case STOP: // STOP - Stop CPU
            {
                // STOP instruction - CPU stops until interrupt
                // For now, we'll just continue execution
                // In a real emulator, this would halt the CPU until an interrupt occurs
                cycles = 4; // STOP takes 4 cycles
                PC++; // Advance PC past STOP instruction
            }
            break;
            
        case LD_HL_E: // LD (HL), E - Load memory at HL with E
            {
                uint16_t hlValue = getHL();
                uint8_t eValue = getE();
                writeByte(hlValue, eValue);
                cycles = 8; // LD (HL),E takes 8 cycles
            }
            break;
            
        case LD_E_HL: // LD E, (HL) - Load E with memory at HL
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setE(value);
                cycles = 8; // LD E,(HL) takes 8 cycles
            }
            break;
            
        case LD_H_HL: // LD H, (HL) - Load H with memory at HL
            {
                uint16_t hlValue = getHL();
                uint8_t value = readByte(hlValue);
                setH(value);
                cycles = 8; // LD H,(HL) takes 8 cycles
            }
            break;
            
        case INC_SP: // INC SP - Increment stack pointer
            {
                SP++;
                cycles = 8; // INC SP takes 8 cycles
            }
            break;
            
        case XOR_L: // XOR L - XOR L with A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t result = aValue ^ lValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                PC++; // Advance program counter
            }
            break;
            
        case OR_B: // OR B - OR B with A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t result = aValue | bValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case DEC_SP: // DEC SP - Decrement stack pointer
            {
                SP--;
                cycles = 8; // DEC SP takes 8 cycles
            }
            break;
            
        case ADD_SP_r8: // ADD SP, r8 - Add signed immediate to SP
            {
                int8_t offset = static_cast<int8_t>(readByte(PC));
                PC++;
                uint16_t spValue = SP;
                uint16_t result = spValue + offset;
                SP = result;
                
                // Set flags based on the addition
                setZeroFlag(false);
                setSubtractFlag(false);
                setHalfCarryFlag(((spValue & 0x0F) + (offset & 0x0F)) > 0x0F);
                setCarryFlag(((spValue & 0xFF) + (offset & 0xFF)) > 0xFF);
                cycles = 16; // ADD SP,r8 takes 16 cycles
            }
            break;
            
        case SBC_A_d8: // SBC A, d8 - Subtract with carry immediate from A
            {
                uint8_t value = readByte(PC);
                PC++;
                uint8_t aValue = getA();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = value + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (value + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 8; // SBC A,r takes 8 cycles
            }
            break;
            
        case LD_HL_H: // LD (HL), H - Load memory at HL with H
            {
                uint16_t hlValue = getHL();
                uint8_t hValue = getH();
                writeByte(hlValue, hValue);
                cycles = 8; // LD (HL),H takes 8 cycles
            }
            break;
            
        case LD_HL_L: // LD (HL), L - Load memory at HL with L
            {
                uint16_t hlValue = getHL();
                uint8_t lValue = getL();
                writeByte(hlValue, lValue);
                cycles = 8; // LD (HL),L takes 8 cycles
            }
            break;
            
        case RET_NZ:
            {
                if (!getZeroFlag()) {
                    PC = pop();
                    cycles = 20;
                } else {
                    cycles = 8;
                }
            }
            break;
            
        case LD_A_FF00_C: // LD A, (FF00+C) - Load A from I/O port C
            {
                uint8_t cValue = getC();
                uint16_t address = 0xFF00 + cValue;
                uint8_t value = readByte(address);
                setA(value);
                cycles = 8; // LD A,(FF00+C) takes 8 cycles
            }
            break;
            
        case LD_FF00_C_A: // LD (FF00+C), A - Store A to I/O port C
            {
                uint8_t cValue = getC();
                uint16_t address = 0xFF00 + cValue;
                uint8_t aValue = getA();
                writeByte(address, aValue);
                cycles = 8; // LD (FF00+C),A takes 8 cycles
            }
            break;
            
        case OR_D: // OR D - OR D with A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t result = aValue | dValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case OR_E: // OR E - OR E with A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t result = aValue | eValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case OR_H: // OR H - OR H with A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t result = aValue | hValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
            }
            break;
            
        case OR_L: // OR L - OR L with A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t result = aValue | lValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // OR r takes 4 cycles
            }
            break;
            
        case XOR_B: // XOR B - XOR B with A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t result = aValue ^ bValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // XOR r takes 4 cycles
            }
            break;
            
        case XOR_D: // XOR D - XOR D with A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t result = aValue ^ dValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // XOR r takes 4 cycles
            }
            break;
            
        case XOR_E: // XOR E - XOR E with A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t result = aValue ^ eValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // XOR r takes 4 cycles
            }
            break;
            
        case XOR_H: // XOR H - XOR H with A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t result = aValue ^ hValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // XOR r takes 4 cycles
            }
            break;
            
        case CP_B: // CP B - Compare B with A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t result = aValue - bValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ bValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < bValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case CP_C: // CP C - Compare C with A
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue - cValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ cValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < cValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case CP_D: // CP D - Compare D with A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t result = aValue - dValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ dValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < dValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case CP_H: // CP H - Compare H with A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t result = aValue - hValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ hValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < hValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case CP_L: // CP L - Compare L with A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t result = aValue - lValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ lValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < lValue);
                cycles = 4; // CP r takes 4 cycles
            }
            break;
            
        case CP_HL: // CP (HL) - Compare memory at HL with A
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = aValue - memValue;
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ memValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < memValue);
                cycles = 8; // CP (HL) takes 8 cycles
            }
            break;
            
        case AND_B: // AND B - AND B with A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t result = aValue & bValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_C: // AND C - AND C with A
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t result = aValue & cValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_D: // AND D - AND D with A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t result = aValue & dValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_E: // AND E - AND E with A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t result = aValue & eValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_H: // AND H - AND H with A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t result = aValue & hValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_L: // AND L - AND L with A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t result = aValue & lValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case AND_HL: // AND (HL) - AND memory at HL with A
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = aValue & memValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 8; // AND (HL) takes 8 cycles
            }
            break;
            
        case AND_A: // AND A - AND A with A
            {
                uint8_t aValue = getA();
                uint8_t result = aValue & aValue; // This is just A, but we need to set flags
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(true);
                setCarryFlag(false);
                cycles = 4; // AND r takes 4 cycles
            }
            break;
            
        case CP_A: // CP A - Compare A with A (always zero)
            {
                setZeroFlag(true);
                setSubtractFlag(true);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // CP A takes 4 cycles
            }
            break;
            
        case ADD_A_B: // ADD A, B - Add B to A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint16_t result = aValue + bValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ bValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADD_A_D: // ADD A, D - Add D to A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint16_t result = aValue + dValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ dValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
            }
            break;
            
        case ADD_A_E: // ADD A, E - Add E to A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint16_t result = aValue + eValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ eValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
            }
            break;
            
        case ADD_A_H: // ADD A, H - Add H to A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint16_t result = aValue + hValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ hValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
            }
            break;
            
        case ADD_A_L: // ADD A, L - Add L to A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint16_t result = aValue + lValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ lValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADD A,r takes 4 cycles
            }
            break;
            
        case ADD_A_HL: // ADD A, (HL) - Add memory at HL to A
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint16_t result = aValue + memValue;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ memValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 8; // ADD A,(HL) takes 8 cycles
            }
            break;
            
        case ADC_A_B: // ADC A, B - Add B to A with carry
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + bValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ bValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_C: // ADC A, C - Add C to A with carry
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + cValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ cValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_D: // ADC A, D - Add D to A with carry
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + dValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ dValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_E: // ADC A, E - Add E to A with carry
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + eValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ eValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_H: // ADC A, H - Add H to A with carry
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + hValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ hValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_L: // ADC A, L - Add L to A with carry
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + lValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ lValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_HL: // ADC A, (HL) - Add memory at HL to A with carry
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + memValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ memValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 8; // ADC A,(HL) takes 8 cycles
                PC++; // Advance program counter
            }
            break;
            
        case ADC_A_A: // ADC A, A - Add A to A with carry
            {
                uint8_t aValue = getA();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t result = aValue + aValue + carry;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(false);
                setHalfCarryFlag(((aValue ^ aValue ^ (result & 0xFF)) & 0x10) != 0);
                setCarryFlag(result > 0xFF);
                cycles = 4; // ADC A,r takes 4 cycles
                PC++; // Advance program counter
            }
            break;
            
        case SUB_B: // SUB B - Subtract B from A
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t result = aValue - bValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ bValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < bValue);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SUB_D: // SUB D - Subtract D from A
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t result = aValue - dValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ dValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < dValue);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SUB_E: // SUB E - Subtract E from A
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t result = aValue - eValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ eValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < eValue);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SUB_H: // SUB H - Subtract H from A
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t result = aValue - hValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ hValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < hValue);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SUB_L: // SUB L - Subtract L from A
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t result = aValue - lValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ lValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < lValue);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SUB_HL: // SUB (HL) - Subtract memory at HL from A
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t result = aValue - memValue;
                setA(result);
                
                setZeroFlag(result == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ memValue ^ result) & 0x10) != 0);
                setCarryFlag(aValue < memValue);
                cycles = 8; // SUB (HL) takes 8 cycles
            }
            break;
            
        case SUB_A: // SUB A - Subtract A from A (always zero)
            {
                setA(0);
                setZeroFlag(true);
                setSubtractFlag(true);
                setHalfCarryFlag(false);
                setCarryFlag(false);
                cycles = 4; // SUB r takes 4 cycles
            }
            break;
            
        case SBC_A_B: // SBC A, B - Subtract B from A with carry
            {
                uint8_t aValue = getA();
                uint8_t bValue = getB();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = bValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (bValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_C: // SBC A, C - Subtract C from A with carry
            {
                uint8_t aValue = getA();
                uint8_t cValue = getC();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = cValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (cValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_D: // SBC A, D - Subtract D from A with carry
            {
                uint8_t aValue = getA();
                uint8_t dValue = getD();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = dValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (dValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_E: // SBC A, E - Subtract E from A with carry
            {
                uint8_t aValue = getA();
                uint8_t eValue = getE();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = eValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (eValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_H: // SBC A, H - Subtract H from A with carry
            {
                uint8_t aValue = getA();
                uint8_t hValue = getH();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = hValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (hValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_L: // SBC A, L - Subtract L from A with carry
            {
                uint8_t aValue = getA();
                uint8_t lValue = getL();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = lValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (lValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
        case SBC_A_HL: // SBC A, (HL) - Subtract memory at HL from A with carry
            {
                uint8_t aValue = getA();
                uint16_t hlValue = getHL();
                uint8_t memValue = readByte(hlValue);
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = memValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (memValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 8; // SBC A,(HL) takes 8 cycles
            }
            break;
            
        case SBC_A_A: // SBC A, A - Subtract A from A with carry
            {
                uint8_t aValue = getA();
                uint8_t carry = getCarryFlag() ? 1 : 0;
                uint16_t fullSub = aValue + carry;
                uint16_t result = aValue - fullSub;
                setA(static_cast<uint8_t>(result));
                
                setZeroFlag(static_cast<uint8_t>(result) == 0);
                setSubtractFlag(true);
                setHalfCarryFlag(((aValue ^ (aValue + carry) ^ result) & 0x10) != 0);
                setCarryFlag(aValue < fullSub);
                cycles = 4; // SBC A,r takes 4 cycles
            }
            break;
            
            
            
            
            
        case 0xFC: // This should be a different instruction
            {
                std::cout << "Unimplemented opcode: 0x" << std::hex << (int)opcode << " at PC=0x" << PC << std::endl;
                cycles = 4;
            }
            break;
            
        default:{
            std::cout << "Unimplemented opcode: 0x" << std::hex << (int)opcode
                      << " at PC=0x" << PC << std::endl; 
            cycles = 4; // Default cycle count for unimplemented instructions
            }
            break;
    }
    
    // Increment cycle counter with instruction-specific cycles
    cycleCount += cycles;
    
    // Update timer registers
    updateTimer(cycles);
    
    // Update PPU if available
    if (ppu) {
        ppu->step(cycles);
    }
    
    // Update Timer if available (Timer class not implemented yet)
    // if (timer) {
    //     timer->step(cycles);
    // }
    
    // Check for interrupts at instruction boundary
    if (checkInterrupts()) {
        // Interrupt was serviced, add interrupt overhead cycles
        cycles += 20;
    }
    
    // Update PPU timing (LY register and scanline timing)
    updatePPUTiming(cycles);
    
    return cycles;
}

// Update PPU timing (LY register and scanline timing)
void CPU::updatePPUTiming(int cycles) {
    // Each scanline takes 456 cycles (114 cycles per mode)
    // LY increments every 456 cycles
    static int scanlineCycles = 0;
    scanlineCycles += cycles;
    
    if (scanlineCycles >= 456) {
        // Move to next scanline
        scanlineCycles -= 456;
        
        uint8_t currentLY = memory.read(0xFF44);
        uint8_t newLY = currentLY + 1;
        
        // LY wraps at 153 (0-153, so 154 total lines)
        if (newLY > 153) {
            newLY = 0;
        }
        
        memory.write(0xFF44, newLY);
        
        // Update STAT register mode based on LY
        uint8_t stat = memory.read(0xFF41);
        stat &= 0xFC;  // Clear mode bits (0-1)
        
        if (newLY < 144) {
            // Visible scanlines: Mode 2 (OAM search), Mode 3 (Pixel transfer), Mode 0 (HBlank)
            // For simplicity, we'll use Mode 0 (HBlank) for visible scanlines
            stat |= 0x00;  // Mode 0
        } else if (newLY == 144) {
            // VBlank starts
            stat |= 0x01;  // Mode 1 (VBlank)
            
            // Trigger VBlank interrupt
            uint8_t if_reg = memory.read(0xFF0F);
            memory.write(0xFF0F, if_reg | 0x01);  // Set VBlank interrupt flag
        } else {
            // VBlank continues
            stat |= 0x01;  // Mode 1 (VBlank)
        }
        
        memory.write(0xFF41, stat);
    }
}

// Check and service interrupts
bool CPU::checkInterrupts() {
    // Handle EI timing - enable interrupts after next instruction
    if (pendingIME) {
        IME = true;
        pendingIME = false;
    }
    
    // Handle interrupts at instruction boundary
    if (IME && (memory.read(0xFFFF) & memory.read(0xFF0F))) {
        uint8_t fired = memory.read(0xFFFF) & memory.read(0xFF0F);
        std::cout << "INTERRUPT FIRED: IME=" << IME << " IE=" << std::hex << (int)memory.read(0xFFFF) << " IF=" << std::hex << (int)memory.read(0xFF0F) << " fired=" << std::hex << (int)fired << std::endl;
        if (fired & 0x01) serviceInterrupt(0x40, 0); // VBlank
        else if (fired & 0x02) serviceInterrupt(0x48, 1); // STAT
        else if (fired & 0x04) serviceInterrupt(0x50, 2); // Timer
        else if (fired & 0x08) serviceInterrupt(0x58, 3); // Serial
        else if (fired & 0x10) serviceInterrupt(0x60, 4); // Joypad
        
        return true;
    }
    
    return false;
}

// Service a specific interrupt
void CPU::serviceInterrupt(uint16_t vector, uint8_t bit) {
    // Clear the IF bit
    uint8_t IF = memory.read(0xFF0F);
    IF &= ~(1 << bit);
    memory.write(0xFF0F, IF);
    
    // Push current PC
    push(PC);
    
    // Jump to vector
    PC = vector;
    
    // Set IME=false
    IME = false;
}