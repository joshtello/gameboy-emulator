#include <iostream>
#include <iomanip>  // Added for std::setfill and std::setw
#include <chrono>
#include <SDL2/SDL.h>
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "debug.h"

int SDL_main(int argc, char* argv[]) {
    std::cout << "========== MAIN() FUNCTION STARTED ==========" << std::endl;
    std::cout << "If you don't see this message, main() is not being called!" << std::endl;
    std::cout.flush();  // Force output immediately
    
    // SDL2 is initialized by SDL2main
    
    // Create memory first
    std::cout << "MAIN: About to create Memory object" << std::endl;
    Memory memory;
    std::cout << "MAIN: Memory object created" << std::endl;

    // Load Pokemon Blue ROM
    try {
        memory.loadRom("pokemon_blue.gb");
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }
    
    // Initialize BIOS state (post-BIOS register and memory defaults)
    std::cout << "MAIN: About to call initializeBIOS()" << std::endl;
    memory.initializeBIOS();
    std::cout << "initializeBIOS: Writing RET at 0x38" << std::endl;
    memory.write(0x38, 0xC9);
    std::cout << "Memory[0x38] after BIOS init = " 
          << std::hex << (int)memory.read(0x38) << std::endl;

          std::cout << "Memory[0x38] before CPU start = " 
          << std::hex << (int)memory.read(0x38) << std::endl;

    // Create CPU and PPU AFTER BIOS initialization
    std::cout << "MAIN: About to create CPU" << std::endl;
    CPU cpu(memory);
    std::cout << "MAIN: CPU created, PC=" << std::hex << cpu.getPC() << std::endl;
    PPU ppu(memory);
    
    // Connect CPU to PPU for automatic updates
    cpu.setPPU(&ppu);
    std::cout << "MAIN: PPU reference set in CPU" << std::endl;

    // Explicitly reset CPU after BIOS initialization
    std::cout << "MAIN: About to call cpu.reset()" << std::endl;
    cpu.reset();
    std::cout << "MAIN: CPU reset completed, PC=" << std::hex << cpu.getPC() << std::endl;
    std::cout << "MAIN: CPU reset, PC should be 0x100" << std::endl;
    std::cout << "MAIN: ROM[0x100] = 0x" << std::hex << (int)memory.read(0x100) << std::endl;
    std::cout << "MAIN: ROM[0x101] = 0x" << std::hex << (int)memory.read(0x101) << std::endl;
    std::cout << "MAIN: ROM[0x102] = 0x" << std::hex << (int)memory.read(0x102) << std::endl;
    ppu.init();   // Initialize SDL2 and create window
    
    // Debug: Show ROM execution from 0x100
    std::cout << "ROM execution from 0x100:" << std::endl;
    for (int i = 0x100; i < 0x170; i++) {
        std::cout << std::hex << i << ": " << (int)memory.read(i) << " ";
        if ((i - 0x100 + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Initialize debug system
    DebugLogger::init();
    
    // Button release timer
    auto lastButtonPress = std::chrono::steady_clock::now();

    std::cout << "Starting Pokemon Red..." << std::endl;
    std::cout << "Press ESC to quit" << std::endl;
    std::cout << "Note: VRAM starts empty - graphics will appear when CPU loads them" << std::endl;
    std::cout << std::endl;
    std::cout << "=== GAME CONTROLS ===" << std::endl;
    std::cout << "A Button: ENTER or SPACE" << std::endl;
    std::cout << "B Button: BACKSPACE" << std::endl;
    std::cout << "Start: S" << std::endl;
    std::cout << "Select: A" << std::endl;
    std::cout << "D-Pad: Arrow Keys" << std::endl;
    std::cout << std::endl;
    std::cout << "=== TO GET TO INTRO ===" << std::endl;
    std::cout << "1. Press ENTER/SPACE to start the game" << std::endl;
    std::cout << "2. Navigate menus with arrow keys" << std::endl;
    std::cout << "3. Press S (Start) to begin new game" << std::endl;
    std::cout << "4. Watch the Pokemon Red intro sequence!" << std::endl;
    std::cout << std::endl;
    
    // VRAM will be populated by the game's initialization code
    
    // Game will initialize graphics and VRAM through normal CPU execution
    
    
    
    bool running = true;
    SDL_Event event;
    
    // Main emulation loop
    while (running) {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                // Game Boy button mapping - properly implement joypad
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    // A button - set bit 0 to 0 (pressed)
                    memory.pressButton(0);  // A button
                    lastButtonPress = std::chrono::steady_clock::now();
                    std::cout << "A button pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    // B button (0xFF00 bit 1)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xFD) | 0x00;  // Press B button
                    memory.write(0xFF00, joypad);
                    std::cout << "B button pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_UP) {
                    // Up button (0xFF00 bit 2)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xFB) | 0x00;  // Press Up
                    memory.write(0xFF00, joypad);
                    std::cout << "Up pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    // Down button (0xFF00 bit 3)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xF7) | 0x00;  // Press Down
                    memory.write(0xFF00, joypad);
                    std::cout << "Down pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    // Left button (0xFF00 bit 4)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xEF) | 0x00;  // Press Left
                    memory.write(0xFF00, joypad);
                    std::cout << "Left pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    // Right button (0xFF00 bit 5)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xDF) | 0x00;  // Press Right
                    memory.write(0xFF00, joypad);
                    std::cout << "Right pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_s) {
                    // Start button (0xFF00 bit 6)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0xBF) | 0x00;  // Press Start
                    memory.write(0xFF00, joypad);
                    std::cout << "Start pressed!" << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_a) {
                    // Select button (0xFF00 bit 7)
                    uint8_t joypad = memory.read(0xFF00);
                    joypad = (joypad & 0x7F) | 0x00;  // Press Select
                    memory.write(0xFF00, joypad);
                    std::cout << "Select pressed!" << std::endl;
                }
            }
        }
        
               try {
                   // Auto-release buttons after 100ms
                   auto now = std::chrono::steady_clock::now();
                   if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastButtonPress).count() > 100) {
                       memory.releaseAllButtons();
                   }
                   
                   // Execute CPU instructions continuously (CPU now calls ppu.step() internally)
                   int cycles = cpu.step();
                   
                   // Check if we should render a frame (at VBlank)
                   if (ppu.beginFrame()) {
                       static int frame_count = 0;
                       frame_count++;
                       if (frame_count <= 5) {
                           std::cout << "MAIN LOOP: Rendering frame #" << frame_count << std::endl;
                       }
                       ppu.render();
                       
                       // Debug output once per frame - KEY REGISTERS (DISABLED for speed)
                       // std::cout << "PC=" << std::hex << cpu.getPC()
                       //           << " DIV=" << (int)memory.read(0xFF04)
                       //           << " LCDC=" << (int)memory.read(0xFF40)
                       //           << " STAT=" << (int)memory.read(0xFF41)
                       //           << " LY="   << (int)memory.read(0xFF44)
                       //           << " JOYPAD=" << (int)memory.read(0xFF00)
                       //           << " IF="   << (int)memory.read(0xFF0F)
                       //           << " IE="   << (int)memory.read(0xFFFF)
                       //           << " VRAM[8000]=" << (int)memory.read(0x8000)
                       //           << " BGP=" << (int)memory.read(0xFF47)
                       //           << " SCY=" << (int)memory.read(0xFF42) << std::endl;
                                 
                       // Debug: Check if CPU is stuck at same PC for too long
                       static uint16_t lastPC = 0;
                       static int pcCount = 0;
                       if (cpu.getPC() == lastPC) {
                           pcCount++;
                           if (pcCount > 5) {  // If stuck at same PC for more than 5 frames
                               std::cout << "*** CPU STUCK at PC=" << std::hex << cpu.getPC() 
                                         << " for " << pcCount << " frames! ***" << std::endl;
                               std::cout << "Next instruction: 0x" << std::hex << (int)memory.read(cpu.getPC()) << std::endl;
                               std::cout << "Next 8 bytes: ";
                               for (int i = 0; i < 8; i++) {
                                   std::cout << "0x" << std::hex << (int)memory.read(cpu.getPC() + i) << " ";
                               }
                               std::cout << std::endl;
                               pcCount = 0;  // Reset counter to avoid spam
                           }
                       } else {
                           lastPC = cpu.getPC();
                           pcCount = 0;
                       }
                   }
                   
                   // Simple frame rate limiting (rough 60 FPS)
                   SDL_Delay(16);
                   
               } catch (const std::exception& e) {
                   std::cout << "CPU Error: " << e.what() << std::endl;
                   break;
               }
    }
    
    std::cout << "Pokemon Red emulation ended." << std::endl;
    
    // SDL2 cleanup is handled by SDL2main

    return 0;
}