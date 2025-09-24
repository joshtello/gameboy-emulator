cmake_minimum_required(VERSION 3.10)
project(gameboy_emulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL2 paths
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR})

# Add executable
<<<<<<< HEAD
add_executable(gameboy_emulator main.cpp cpu.cpp)

# Link SDL2 and SDL2main
target_link_libraries(gameboy_emulator SDL2main SDL2)
=======
add_executable(gameboy_emulator main.cpp)

# Link SDL2 and SDL2main
target_link_libraries(gameboy_emulator SDL2main SDL2)
>>>>>>> 0ac8910690b61c2ab57e67f9d285d9d29c426d90
