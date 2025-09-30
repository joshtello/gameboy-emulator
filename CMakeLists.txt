cmake_minimum_required(VERSION 3.10)
project(gameboy_emulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL2 paths
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR})

# Add executable
add_executable(gameboy_emulator main.cpp cpu.cpp ppu.cpp debug.cpp)

# Add test executable
add_executable(cpu_test test_cpu.cpp cpu.cpp memory.cpp debug.cpp ppu.cpp)

# Add simple memory test (no SDL dependency)
add_executable(memory_test test_memory_simple.cpp memory.cpp)

# Link SDL2 and SDL2main (use SDL_main instead of main)
target_link_libraries(gameboy_emulator SDL2main SDL2)
target_link_libraries(cpu_test SDL2main SDL2)