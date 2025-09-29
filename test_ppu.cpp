#include <SDL2/SDL.h>
#include <iostream>

int main() {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window (scaled up for visibility)
    SDL_Window* window = SDL_CreateWindow("Game Boy Emulator Test",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               160 * 4, 144 * 4,  // Scaled Game Boy screen
                               SDL_WINDOW_SHOWN);
    
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    std::cout << "PPU test initialized successfully!" << std::endl;
    
    bool running = true;
    while (running) {
        SDL_Event event;
        
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Create a simple checkerboard pattern
        for (int y = 0; y < 144; y++) {
            for (int x = 0; x < 160; x++) {
                // Create a checkerboard pattern
                bool isEven = ((x / 8) + (y / 8)) % 2 == 0;
                
                // Scale up the pixel
                int scaledX = x * 4;
                int scaledY = y * 4;
                
                // Set color based on pattern
                if (isEven) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
                }
                
                // Draw the scaled pixel
                SDL_Rect pixelRect = {scaledX, scaledY, 4, 4};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
        
        // Present the rendered frame
        SDL_RenderPresent(renderer);
        
        // Small delay to prevent excessive CPU usage
        SDL_Delay(16); // ~60 FPS
    }
    
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
