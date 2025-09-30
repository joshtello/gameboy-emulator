#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include "memory.h"

class PPU {
private:
    SDL_Window* window;        // The actual window on your screen
    SDL_Renderer* renderer;    // Draws pixels to the window
    uint32_t* framebuffer;     // Our 160x144 pixel data
    Memory& memory; // Reference to CPU memory
    
    static constexpr int SCALE_FACTOR = 4;  // Scale Game Boy screen 4x
    
    // LCD timing variables
    int cycleCounter = 0;      // Current cycle count for current line
    bool vblankFlag = false;   // VBlank interrupt flag
    
    // LCD timing constants (DMG)
    static constexpr int CYCLES_PER_LINE = 456;
    static constexpr int CYCLES_OAM = 80;
    static constexpr int CYCLES_TRANSFER = 172;
    static constexpr int CYCLES_HBLANK = 204;
    static constexpr int VISIBLE_LINES = 144;
    static constexpr int VBLANK_LINES = 10;
    static constexpr int TOTAL_LINES = 154;  // 144 visible + 10 VBlank
public:
    PPU(Memory& mem); //Constructor takes memory reference
    ~PPU();

    void init();
    void render();
    void cleanup();
    
    // PPU timing system
    void step(int cycles);
    bool beginFrame();
    bool endFrame();
    
    // Convert CPU memory to pixels
    void updateFramebuffer();
    
    // Render a single 8x8 tile
    void renderTile(uint8_t* tileData, int screenX, int screenY);
    
    // Decode Game Boy VRAM tile into pixel array
    // Each tile is 16 bytes (8x8 pixels), each row is 2 bytes (bitplanes)
    // Returns 8x8 array of color indices (0-3)
    std::array<std::array<uint8_t, 8>, 8> decodeTile(const uint8_t* tileData);
    
    // Map 2-bit color index to SDL color using BGP register
    // BGP register: bits 0-1 = color 0, bits 2-3 = color 1, bits 4-5 = color 2, bits 6-7 = color 3
    SDL_Color getColorFromBGP(uint8_t colorIndex, uint8_t bgpRegister);
    
};

