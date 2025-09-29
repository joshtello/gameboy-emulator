#include "ppu.h"
#include <iostream>

// Constructor
PPU::PPU(Memory& mem) : memory(mem) {
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
    
    // Create window (scaled up for visibility)
    window = SDL_CreateWindow("Game Boy Emulator",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               160 * SCALE_FACTOR, 144 * SCALE_FACTOR,  // Scaled Game Boy screen
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Load actual tile data from ROM
    uint8_t tileData[16];
    
    // Try reading from different ROM locations where graphics might be stored
    // Start with address 0x1000 (after the ROM header)
    for (int i = 0; i < 16; i++) {
        tileData[i] = memory.read(0x1000 + i);
    }
    
    // Print the tile data for debugging (only first few times)
    static int debugCount = 0;
    if (debugCount < 3) {
        std::cout << "Tile data: ";
        for (int i = 0; i < 16; i++) {
            std::cout << std::hex << (int)tileData[i] << " ";
        }
        std::cout << std::endl;
        debugCount++;
    }
    
    // Pokemon Red is loading graphics directly from ROM, not VRAM
    // Let's read from ROM locations where we can see actual tile data
    
    // Debug: Print VRAM status
    static int vramDebugCount = 0;
    if (vramDebugCount < 10) {  // Print first 10 times
        std::cout << "VRAM 0x8000-0x800F: ";
        for (int i = 0; i < 16; i++) {
            printf("%02x ", memory.read(0x8000 + i));
        }
        std::cout << " | TileMap 0x9800-0x980F: ";
        for (int i = 0; i < 16; i++) {
            printf("%02x ", memory.read(0x9800 + i));
        }
        std::cout << std::endl;
        vramDebugCount++;
    }
    
    // Render a smaller grid using known graphics locations
    // Use the ROM addresses where we know there's actual graphics data
    
    // TEMPORARY: Draw a simple test pattern to verify PPU works
    // This will help us confirm the rendering pipeline is functional
    
    // Create a simple checkerboard pattern
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            // Create a checkerboard pattern
            bool isEven = ((x / 8) + (y / 8)) % 2 == 0;
            
            // Scale up the pixel
            int scaledX = x * SCALE_FACTOR;
            int scaledY = y * SCALE_FACTOR;
            
            // Set color based on pattern
            if (isEven) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
            }
            
            // Draw the scaled pixel
            SDL_Rect pixelRect = {scaledX, scaledY, SCALE_FACTOR, SCALE_FACTOR};
            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }
    
    // Present the rendered frame
    SDL_RenderPresent(renderer);
    
    // Debug: Print first 64 bytes of VRAM every frame
    static int frameCount = 0;
    frameCount++;
    if (frameCount <= 10) {  // Print first 10 frames
        std::cout << "\n=== FRAME " << frameCount << " - VRAM 0x8000-0x803F ===" << std::endl;
        for (int i = 0; i < 64; i++) {
            if (i % 16 == 0) {
                std::cout << "0x" << std::hex << (0x8000 + i) << ": ";
            }
            std::cout << std::hex << static_cast<int>(memory.read(0x8000 + i)) << " ";
            if (i % 16 == 15) {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
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

// Render a single 8x8 tile
void PPU::renderTile(uint8_t* tileData, int screenX, int screenY) {
    // Game Boy color palette (4 shades of gray)
    SDL_Color colors[4] = {
        {255, 255, 255, 255},  // Color 0: White
        {192, 192, 192, 255},  // Color 1: Light gray
        {96,  96,  96,  255},  // Color 2: Dark gray
        {0,   0,   0,   255}   // Color 3: Black
    };
    
    // Process each row of the tile
    for (int row = 0; row < 8; row++) {
        uint8_t lowByte = tileData[row * 2];      // Low bits
        uint8_t highByte = tileData[row * 2 + 1]; // High bits
        
        // Process each pixel in the row
        for (int pixel = 0; pixel < 8; pixel++) {
            // Extract 2-bit color value
            uint8_t colorIndex = 0;
            if (lowByte & (0x80 >> pixel)) {
                colorIndex |= 1;  // Set bit 0
            }
            if (highByte & (0x80 >> pixel)) {
                colorIndex |= 2;  // Set bit 1
            }
            
            // Draw the pixel (scaled up)
            int x = (screenX + pixel) * SCALE_FACTOR;
            int y = (screenY + row) * SCALE_FACTOR;
            
            // Only draw if within screen bounds
            if (x >= 0 && x < 160 * SCALE_FACTOR && y >= 0 && y < 144 * SCALE_FACTOR) {
                SDL_Color color = colors[colorIndex];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                
                // Draw a scaled block for each pixel
                SDL_Rect pixelRect = {x, y, SCALE_FACTOR, SCALE_FACTOR};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
    }
}
