#include <SDL2/SDL.h>
#include <iostream>

#ifdef _WIN32
#include <SDL2/SDL_main.h>
#endif

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Game Boy Emulator",                  // Window title
        SDL_WINDOWPOS_CENTERED,               // Initial x position
        SDL_WINDOWPOS_CENTERED,               // Initial y position
        640,                                  // Width
        576,                                  // Height
        SDL_WINDOW_SHOWN                      // Window flags
    );

    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}