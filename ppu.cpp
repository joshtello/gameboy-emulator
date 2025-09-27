#include "ppu.h"
#include <iostream>

// Constructor
PPU::PPU() {
    window = nullptr;
    renderer = nullptr;
    framebuffer = nullptr;
}

// Destructor
PPU::~PPU() {
    cleanup();
}

// Initialize SDL2 and create window
void PPU::init() {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create window
    window = SDL_CreateWindow("Game Boy Emulator",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               160, 144,  // Game Boy screen size
                               SDL_WINDOW_SHOWN);
    
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Allocate framebuffer (160x144 pixels)
    framebuffer = new uint32_t[160 * 144];
    
    std::cout << "PPU initialized successfully!" << std::endl;
}

// Render the framebuffer to screen
void PPU::render() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer);
    
    // For now, just show a simple pattern
    // We'll implement proper tile rendering later
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            // Simple pattern: alternating colors
            if ((x + y) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White
            } else {
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);  // Gray
            }
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    
    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

// Clean up resources
void PPU::cleanup() {
    if (framebuffer) {
        delete[] framebuffer;
        framebuffer = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}

// Convert CPU memory to pixels (placeholder for now)
void PPU::updateFramebuffer() {
    // This will be implemented later when we connect to CPU memory
    // For now, just a placeholder
}
