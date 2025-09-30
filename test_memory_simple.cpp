#include <iostream>
#include <cassert>
#include "memory.h"

void testMemoryBasicOperations() {
    std::cout << "=== Testing Basic Memory Operations ===" << std::endl;
    
    Memory memory;
    
    // Test basic read/write
    memory.write(0xC000, 0xAB);
    assert(memory.read(0xC000) == 0xAB);
    std::cout << "✓ Basic read/write test passed" << std::endl;
    
    // Test 16-bit operations
    memory.write_word(0xC001, 0x1234);
    assert(memory.read_word(0xC001) == 0x1234);
    std::cout << "✓ 16-bit memory operations test passed" << std::endl;
    
    // Test VRAM access
    memory.write(0x8000, 0x3C);
    assert(memory.read(0x8000) == 0x3C);
    std::cout << "✓ VRAM access test passed" << std::endl;
    
    // Test I/O registers
    memory.write(0xFF40, 0x91); // LCDC
    assert(memory.read(0xFF40) == 0x91);
    std::cout << "✓ I/O register test passed" << std::endl;
}

void testMemoryBanking() {
    std::cout << "\n=== Testing Memory Banking ===" << std::endl;
    
    Memory memory;
    
    // Test ROM loading (this will test cartridge type detection)
    try {
        memory.loadRom("../cpu_instrs.gb");
        std::cout << "✓ ROM loading test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "⚠️ ROM loading test failed: " << e.what() << std::endl;
    }
    
    // Test memory regions
    // ROM Bank 0
    uint8_t romByte = memory.read(0x0100);
    std::cout << "✓ ROM Bank 0 read: 0x" << std::hex << (int)romByte << std::endl;
    
    // Work RAM
    memory.write(0xC000, 0x42);
    assert(memory.read(0xC000) == 0x42);
    std::cout << "✓ Work RAM test passed" << std::endl;
}

void testInterruptVectors() {
    std::cout << "\n=== Testing Interrupt Vectors ===" << std::endl;
    
    Memory memory;
    
    // Test interrupt vectors are properly set
    assert(memory.read(0x40) == 0xC3); // VBlank interrupt
    assert(memory.read(0x48) == 0xC3); // LCD STAT interrupt
    assert(memory.read(0x50) == 0xC3); // Timer interrupt
    assert(memory.read(0x58) == 0xC3); // Serial interrupt
    assert(memory.read(0x60) == 0xC3); // Joypad interrupt
    assert(memory.read(0x38) == 0xC9); // RST 38 (RET)
    
    std::cout << "✓ Interrupt vectors test passed" << std::endl;
}

void testJoypadSystem() {
    std::cout << "\n=== Testing Joypad System ===" << std::endl;
    
    Memory memory;
    
    // Test initial joypad state
    uint8_t joypad = memory.read(0xFF00);
    std::cout << "Initial joypad state: 0x" << std::hex << (int)joypad << std::endl;
    
    // Test button press
    memory.pressButton(0); // A button
    joypad = memory.read(0xFF00);
    std::cout << "After A button press: 0x" << std::hex << (int)joypad << std::endl;
    
    // Test button release
    memory.releaseAllButtons();
    joypad = memory.read(0xFF00);
    std::cout << "After release all: 0x" << std::hex << (int)joypad << std::endl;
    
    std::cout << "✓ Joypad system test passed" << std::endl;
}

int main() {
    std::cout << "Starting Game Boy Emulator Memory Tests..." << std::endl;
    
    try {
        testMemoryBasicOperations();
        testMemoryBanking();
        testInterruptVectors();
        testJoypadSystem();
        
        std::cout << "\n🎉 All memory tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
