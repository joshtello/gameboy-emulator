#include <SDL2/SDL.h>
#include <iostream>
<<<<<<< HEAD
#include "memory.h"
#include "cpu.h"
=======
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90

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

<<<<<<< HEAD
    // Initialize Game Boy components
    Memory memory;
    CPU cpu(memory);
    
    std::cout << "Game Boy Emulator Started!" << std::endl;
    std::cout << "CPU and Memory initialized." << std::endl;
    
    // Print initial CPU state
    cpu.printRegisters();
    cpu.printFlags();

=======
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90
    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    // Main loop
<<<<<<< HEAD
    int stepCount = 0;
=======
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

<<<<<<< HEAD
        // Execute CPU step
        cpu.step();
        stepCount++;
        
        // Print CPU state every 100 steps for debugging
        if (stepCount % 100 == 0) {
            std::cout << "Step " << stepCount << " - PC: 0x" << std::hex << cpu.getPC() << std::dec << std::endl;
        }

=======
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Update screen
        SDL_RenderPresent(renderer);
<<<<<<< HEAD
        
        // Add small delay to prevent overwhelming the system
        SDL_Delay(1);
=======
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}