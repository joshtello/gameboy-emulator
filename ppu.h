#pragma once
#include <SDL2/SDL.h>
#include <cstdint>

class PPU {
private:
    SDL_Window* window;        // The actual window on your screen
    SDL_Renderer* renderer;    // Draws pixels to the window
    uint32_t* framebuffer;     // Our 160x144 pixel data
    
public:
    PPU();
    ~PPU();
    
    void init();
    void render();
    void cleanup();
    
    // Convert CPU memory to pixels
    void updateFramebuffer();
};
