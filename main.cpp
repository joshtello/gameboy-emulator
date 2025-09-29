#include <SDL2/SDL.h>
#include <iostream>
#include "memory.h"
#include "cpu.h"
#include "ppu.h"

int main(int argc, char* argv[]) {
    // Create memory, CPU, and PPU
    Memory memory;
    CPU cpu(memory);
    PPU ppu(memory); // Pass memory reference to PPU constructor

    // Load the test ROM
    try {
        memory.loadRom("Pokemon - Red Version (USA, Europe) (SGB Enhanced).gb");  // Load Pokemon Red ROM
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }

    // Initialize CPU and PPU
    cpu.reset();  // Sets PC to 0x100 (ROM entry point)
    ppu.init();  // Initialize graphics


    bool running = true;
    while (running) {
        SDL_Event event;
        
        // Wait for events instead of just polling
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        // TEMPORARY: Skip CPU execution to test PPU with manual VRAM data
        // cpu.step();
        
        // Manually populate VRAM with test data
        static bool vramInitialized = false;
        if (!vramInitialized) {
            // Write some test tile data to VRAM
            for (int i = 0; i < 16; i++) {
                memory.write(0x8000 + i, 0xFF); // Solid white tile
            }
            for (int i = 0; i < 16; i++) {
                memory.write(0x8010 + i, 0x00); // Solid black tile
            }
            
            // Write tile map data
            for (int i = 0; i < 32 * 32; i++) {
                memory.write(0x9800 + i, (i % 2) ? 1 : 0); // Alternating tiles
            }
            
            vramInitialized = true;
            std::cout << "VRAM initialized with test data" << std::endl;
        }
        
        ppu.render();
    }

    return 0;
}