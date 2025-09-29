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
public:
    PPU(Memory& mem); //Constructor takes memory reference
    ~PPU();

    void init();
    void render();
    void cleanup();
    
    // Convert CPU memory to pixels
    void updateFramebuffer();
    
    // Render a single 8x8 tile
    void renderTile(uint8_t* tileData, int screenX, int screenY);
    
};

