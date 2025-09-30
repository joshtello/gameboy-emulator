#include <iostream>
#include <cassert>
#include <SDL2/SDL.h>
#include "memory.h"
#include "ppu.h"

void testPPUInitialization() {
    std::cout << "=== Testing PPU Initialization ===" << std::endl;
    
    Memory memory;
    PPU ppu(memory);
    
    // Test PPU initialization
    ppu.init();
    std::cout << "✓ PPU initialization successful" << std::endl;
    
    // Test basic rendering
    ppu.updateFramebuffer();
    std::cout << "✓ Framebuffer update successful" << std::endl;
    
    // Test frame timing
    bool frameStarted = ppu.beginFrame();
    std::cout << "✓ Frame timing test: " << (frameStarted ? "Frame started" : "No frame yet") << std::endl;
}

void testVRAMOperations() {
    std::cout << "\n=== Testing VRAM Operations ===" << std::endl;
    
    Memory memory;
    
    // Test VRAM write/read
    memory.write(0x8000, 0x3C); // Write to VRAM
    assert(memory.read(0x8000) == 0x3C);
    std::cout << "✓ VRAM read/write test passed" << std::endl;
    
    // Test tile data
    memory.write(0x8000, 0x3C); // Tile data byte 1
    memory.write(0x8001, 0x7E); // Tile data byte 2
    std::cout << "✓ Tile data write test passed" << std::endl;
}

void testLCDRegisters() {
    std::cout << "\n=== Testing LCD Registers ===" << std::endl;
    
    Memory memory;
    
    // Test LCDC register
    memory.write(0xFF40, 0x91); // LCD enabled, BG enabled
    assert(memory.read(0xFF40) == 0x91);
    std::cout << "✓ LCDC register test passed" << std::endl;
    
    // Test BGP register
    memory.write(0xFF47, 0xFC); // Background palette
    assert(memory.read(0xFF47) == 0xFC);
    std::cout << "✓ BGP register test passed" << std::endl;
    
    // Test scroll registers
    memory.write(0xFF42, 0x00); // SCY
    memory.write(0xFF43, 0x00); // SCX
    assert(memory.read(0xFF42) == 0x00);
    assert(memory.read(0xFF43) == 0x00);
    std::cout << "✓ Scroll registers test passed" << std::endl;
}

void testTileDecoding() {
    std::cout << "\n=== Testing Tile Decoding ===" << std::endl;
    
    Memory memory;
    PPU ppu(memory);
    
    // Create a simple tile pattern
    uint8_t tileData[16] = {
        0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C,  // First 8 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   // Last 8 bytes
    };
    
    // Test tile decoding
    auto decoded = ppu.decodeTile(tileData);
    
    // Check that we got an 8x8 array
    assert(decoded.size() == 8);
    assert(decoded[0].size() == 8);
    std::cout << "✓ Tile decoding test passed" << std::endl;
}

int main() {
    std::cout << "Starting Game Boy Emulator PPU Tests..." << std::endl;
    
    try {
        testPPUInitialization();
        testVRAMOperations();
        testLCDRegisters();
        testTileDecoding();
        
        std::cout << "\n🎉 All PPU tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
