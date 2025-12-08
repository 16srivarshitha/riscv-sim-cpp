cmake_minimum_required(VERSION 3.10)
project(RISCV_Simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Source files
set(SOURCES
    src/main.cpp
    src/cpu.cpp
    src/memory.cpp
    src/instruction.cpp
    src/decoder.cpp
    src/loader.cpp
)

# Create executable
add_executable(riscv-sim ${SOURCES})

# Include directories
target_include_directories(riscv-sim PRIVATE src)

# Installation
install(TARGETS riscv-sim DESTINATION bin)