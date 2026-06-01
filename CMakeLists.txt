cmake_minimum_required(VERSION 3.16)

project(chip8_emulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)

add_executable(chip8_emulator
	src/chip8.cpp
    src/chip8.hpp
    src/platform.hpp
    src/main.cpp
)

target_include_directories(chip8_emulator PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8_emulator PRIVATE ${SDL2_LIBRARIES})
