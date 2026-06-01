# CHIP-8 Emulator

A simple CHIP-8 emulator written in C++ with SDL2 for graphics and input.

## Requirements

- C++17 compiler
- CMake 3.16+
- SDL2 development package

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/chip8_emulator <scale> <delay_ms> <rom_path>
```

Example:

```bash
./build/chip8_emulator 10 2 games/PONG
```

## Controls

CHIP-8 keypad mapping:

```
1 2 3 C    -> 1 2 3 4
4 5 6 D    -> Q W E R
7 8 9 E    -> A S D F
A 0 B F    -> Z X C V
```

Press `Esc` to quit.
