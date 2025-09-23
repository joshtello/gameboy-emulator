cmake_minimum_required(VERSION 3.10)
project(gameboy_emulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL3 paths
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR})

# Add executable
add_executable(gameboy_emulator main.cpp)

# Link SDL3
target_link_libraries(gameboy_emulator SDL3)