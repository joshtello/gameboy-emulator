# Game Boy Emulator

A Game Boy emulator written in C++. This project aims to emulate the original Game Boy hardware while learning C++ and computer architecture.

## Current Progress

### Completed Features
- Basic SDL2 window setup
- Memory system implementation (64KB addressable space)

### Components Overview

#### 1. Display System (main.cpp)
```cpp
// Creates a window that will display our Game Boy screen
SDL_Window* window = SDL_CreateWindow(
    "Game Boy Emulator",    // Window title
    SDL_WINDOWPOS_CENTERED, // Center on screen
    SDL_WINDOWPOS_CENTERED,
    640,                    // Width (will change to GB size)
    576,                    // Height
    SDL_WINDOW_SHOWN
);
```

#### 2. Memory System (memory.h)
The Game Boy has 64KB of addressable memory space, divided into regions:
```
Memory Map:
0000-3FFF: ROM Bank 0     (16KB ROM - Game data)
4000-7FFF: ROM Bank N     (16KB ROM - Switchable)
8000-9FFF: Video RAM      (8KB VRAM - Graphics)
A000-BFFF: External RAM   (8KB External RAM - Save data)
C000-DFFF: Work RAM       (8KB RAM - General use)
E000-FDFF: Echo RAM       (Mirror of C000-DDFF)
FE00-FE9F: Sprite RAM     (Sprite data)
FF00-FF7F: I/O Registers  (Hardware controls)
FF80-FFFE: High RAM       (127 bytes - Fast access)
FFFF:      Interrupt Enable Register
```

## Project Structure

```
gameboy_emulator/
├── main.cpp          - Window creation and main loop
├── memory.h          - Memory system implementation
├── include/          - Header files
│   └── SDL2/         - SDL2 headers
├── SDL2.dll          - SDL2 runtime
└── CMakeLists.txt    - Build configuration
```

## Building the Project

### Required Tools
1. **C++ Compiler** - Visual Studio Build Tools 2022
2. **CMake** (version 3.10 or higher)
3. **SDL2** - For graphics and input handling

### Build Steps
```bash
# Create build directory
mkdir build
cd build

# Generate build files
cmake .. -A x64

# Build the project
cmake --build . --config Release

# Copy SDL2.dll to the executable directory
copy ..\SDL2.dll Release\
```

### Running
```bash
cd Release
gameboy_emulator.exe
```

## C++ Concepts Used

### Classes and Objects
```cpp
class Memory {
private:
    std::array<uint8_t, 0x10000> memory;  // 64KB memory array
public:
    uint8_t read(uint16_t address);       // Read from memory
    void write(uint16_t address, uint8_t value); // Write to memory
};
```

### Data Types
- `uint8_t`: 8-bit unsigned integer (0-255)
- `uint16_t`: 16-bit unsigned integer (0-65535)
- `std::array`: Modern C++ fixed-size array

### Memory Management
- Using `std::array` for safe, managed memory
- Proper cleanup with destructors
- No manual memory allocation needed yet

## Next Steps

1. **CPU Emulation**
   - Implement Z80-like processor
   - Register system
   - Instruction decoder

2. **Graphics System**
   - Implement tile-based graphics
   - Sprite handling
   - Game Boy's 4-color palette

3. **Input System**
   - D-pad implementation
   - A/B buttons
   - Start/Select buttons

## Learning Resources

### C++ Basics
- [C++ Reference](https://en.cppreference.com/)
- [LearnCpp.com](https://www.learncpp.com/)

### Game Boy Architecture
- [Pan Docs](https://gbdev.io/pandocs/) - Technical reference
- [Game Boy CPU Manual](http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf)
- [Game Boy Architecture](https://www.copetti.org/writings/consoles/game-boy/)

### Development Tools
- [Visual Studio Code](https://code.visualstudio.com/) - Recommended IDE
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [SDL2 Wiki](https://wiki.libsdl.org/SDL2/FrontPage)

## Contributing

This is a learning project, but suggestions and improvements are welcome!