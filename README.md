# Game Boy Emulator

A Game Boy emulator written in C++. This project aims to emulate the original Game Boy hardware while learning C++ and computer architecture.

## 🎮 Current Progress

### ✅ Completed Features
- **CPU Emulation**: Full Z80-like processor with 8 registers (AF, BC, DE, HL, PC, SP)
- **Memory System**: 64KB addressable space with ROM loading
- **Graphics System**: SDL2-based PPU with 160x144 window
- **Instruction Set**: 20+ CPU instructions implemented
- **Build System**: CMake configuration with SDL2 integration

### 🚀 Recent Achievements
- **Graphics Pipeline**: Working SDL2 window with pixel-level rendering
- **CPU Instructions**: LD, PUSH, POP, CALL, JR, RET, DI, and more
- **Memory Management**: Proper resource cleanup and error handling
- **Cross-Platform**: Windows build system with Visual Studio

## 🏗️ Architecture Overview

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│    CPU      │    │    PPU      │    │   SDL2      │
│ (Emulator)  │───▶│ (Graphics)  │───▶│ (Window)    │
│             │    │             │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
```

## 📁 Project Structure

```
gameboy_emulator/
├── main.cpp          - Main program and CPU loop
├── cpu.h/cpp         - CPU emulation (registers, instructions)
├── memory.h          - Memory system (64KB addressable space)
├── ppu.h/cpp         - Graphics system (SDL2 rendering)
├── CMakeLists.txt    - Build configuration
├── SDL2.dll          - SDL2 runtime library
├── SDL2.lib          - SDL2 static library
├── SDL2main.lib      - SDL2 main library
└── cpu_instrs.gb     - Test ROM for CPU instructions
```

## 🎯 Core Components

### 1. CPU System (cpu.h/cpp)
```cpp
class CPU {
private:
    // 8-bit registers
    uint8_t A, F, B, C, D, E, H, L;
    // 16-bit registers  
    uint16_t PC, SP;
    // Flags
    bool Z, N, H, C;
    
public:
    void step();           // Execute one instruction
    void reset();          // Reset to initial state
    void printRegisters(); // Debug output
};
```

**Implemented Instructions:**
- `NOP` (0x00) - No operation
- `LD` instructions - Load values into registers
- `PUSH/POP` - Stack operations
- `CALL` - Function calls with conditions
- `JR` - Relative jumps
- `RET` - Return from functions
- `DI` - Disable interrupts

### 2. Memory System (memory.h)
```cpp
class Memory {
private:
    std::array<uint8_t, 0x10000> memory;  // 64KB
    
public:
    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t value);
    uint16_t readWord(uint16_t address);
    void writeWord(uint16_t address, uint16_t value);
    void loadRom(const std::string& filename);
};
```

**Memory Map:**
```
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

### 3. Graphics System (ppu.h/cpp)
```cpp
class PPU {
private:
    SDL_Window* window;        // The actual window on screen
    SDL_Renderer* renderer;    // Draws pixels to the window
    uint32_t* framebuffer;     // 160x144 pixel data
    
public:
    void init();               // Initialize SDL2 and create window
    void render();             // Draw pixels to screen
    void cleanup();            // Clean up resources
    void updateFramebuffer(); // Convert CPU memory to pixels
};
```

**Graphics Features:**
- **160x144 window** (Game Boy screen size)
- **Pixel-level rendering** with SDL2
- **Framebuffer system** for graphics data
- **Resource management** with proper cleanup

## 🔧 Building the Project

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
cmake --build . --config Debug

# Copy SDL2.dll to the executable directory
copy ..\SDL2.dll Debug\
```

### Running
```bash
cd Debug
gameboy_emulator.exe
```

## 🎮 What You'll See

When you run the emulator:
1. **Console output** showing CPU register states
2. **160x144 window** with checkerboard pattern
3. **"PPU initialized successfully!"** message
4. **CPU instruction execution** with register updates

## 🧠 C++ Concepts Used

### Classes and Objects
```cpp
class CPU {
private:
    uint8_t A, F, B, C, D, E, H, L;  // 8-bit registers
    uint16_t PC, SP;                 // 16-bit registers
    
public:
    void step();                     // Execute one instruction
    void reset();                    // Reset to initial state
};
```

### Memory Management
```cpp
// Stack operations
void push(uint16_t value) {
    SP -= 2;
    writeWord(SP, value);
}

uint16_t pop() {
    uint16_t value = readWord(SP);
    SP += 2;
    return value;
}
```

### SDL2 Graphics
```cpp
// Initialize graphics
void PPU::init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Game Boy Emulator", 
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               160, 144, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}
```

## 🚀 Next Steps

### 🎯 Immediate Goals
1. **Event Handling** - Close window, keyboard input
2. **CPU Memory Connection** - Display actual Game Boy graphics
3. **Tile Rendering** - Convert Game Boy tiles to pixels
4. **Proper Colors** - 4 shades of gray like real Game Boy

### 🎮 Future Features
- **Sprite Rendering** - Moving objects
- **Background Scrolling** - Parallax effects
- **Sound System** - Game Boy audio
- **Input Handling** - Gamepad/button support
- **Game Loading** - Run actual Game Boy ROMs

## 📚 Learning Resources

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

## 🎉 What You've Accomplished

**You now have:**
- ✅ **Working CPU emulator** with 20+ instructions
- ✅ **Memory system** with 64KB addressable space
- ✅ **Graphics pipeline** with SDL2 integration
- ✅ **160x144 window** (Game Boy screen size)
- ✅ **Pixel-level drawing** capability
- ✅ **Build system** with CMake and SDL2
- ✅ **Foundation** for running Game Boy games

**This is a HUGE milestone!** You've built the core systems that will eventually display Pokemon, Mario, and all your favorite Game Boy games! 🎮✨

## 🤝 Contributing

This is a learning project, but suggestions and improvements are welcome! Feel free to:
- Report bugs or issues
- Suggest new features
- Share your own implementations
- Ask questions about the code

## 📝 License

This project is for educational purposes. Game Boy is a trademark of Nintendo.