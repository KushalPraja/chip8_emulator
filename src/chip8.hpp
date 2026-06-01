#include <cstdint>
#include <random>

#pragma once

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

class Chip8
{

public:
    u8 V[16]{};                                      // registers
    u8 memory[4096]{};                               // memory
    u16 I{};                                         // index register
    u16 pc{};                                        // program counter
    u8 sp{};                                         // stack pointer
    u16 stack[16]{};                                 // stack
    u8 delay_timer{};                                // delay timer
    u8 sound_timer{};                                // sound timer
    u8 keypad[16]{};                                 // keys
    u32 video[64 * 32]{};                            // video
    u16 opcode{};                                    // current opcode
    std::default_random_engine randGen;              // random number generator
    std::uniform_int_distribution<uint8_t> randByte; // random number distribution

    // function pointer tables for opcode handling

    typedef void (Chip8::*Chip8Func)();
    Chip8Func TablePointer[0xF + 1]{&Chip8::OP_NULL};
    Chip8Func Table0Pointer[0xE + 1]{&Chip8::OP_NULL};
    Chip8Func Table8Pointer[0xE + 1]{&Chip8::OP_NULL};
    Chip8Func TableEPointer[0xE + 1]{&Chip8::OP_NULL};
    Chip8Func TableFPointer[0x65 + 1]{&Chip8::OP_NULL};

    Chip8();
    void LoadROM(const char *filename);
    void OP_00E0(); // Clear the display
    void OP_00EE(); // Return from a subroutine
    void OP_1nnn(); // Jump to address nnn
    void OP_2nnn(); // Call subroutine at nnn
    void OP_3xkk(); // Skip next instruction if Vx == kk
    void OP_4xkk(); // Skip next instruction if Vx != kk
    void OP_5xy0(); // Skip next instruction if Vx == Vy
    void OP_6xkk(); // Set Vx = kk
    void OP_7xkk(); // Set Vx = Vx + kk
    void OP_8xy0(); // Set Vx = Vy
    void OP_8xy1(); // Set Vx = Vx OR Vy
    void OP_8xy2(); // Set Vx = Vx AND Vy
    void OP_8xy3(); // Set Vx = Vx XOR Vy
    void OP_8xy4(); // Set Vx = Vx + Vy, set
    void OP_8xy5(); // Set Vx = Vx - Vy, set VF = NOT borrow
    void OP_8xy6(); // Set Vx = Vx SHR 1
    void OP_8xy7(); // Set Vx = Vy - Vx, set
    void OP_8xyE(); // Set Vx = Vx SHL 1
    void OP_9xy0(); // Skip next instruction if Vx != Vy
    void OP_Annn(); // Set I = nnn
    void OP_Bnnn(); // Jump to location nnn + V0
    void OP_Cxkk(); // Set Vx = random byte AND kk
    void OP_Dxyn(); // Display n-byte sprite starting at memory location I
    void OP_Ex9E(); // Skip next instruction if key with the value of Vx is pressed
    void OP_ExA1(); // Skip next instruction if key with the value of V
    void OP_Fx07(); // Set Vx = delay timer value
    void OP_Fx0A(); // Wait for a key press, store the value of the key in Vx
    void OP_Fx15(); // Set delay timer = Vx
    void OP_Fx18(); // Set sound timer = Vx
    void OP_Fx1E(); // Set I = I + Vx
    void OP_Fx29(); // Set I = location of sprite for digit Vx
    void OP_Fx33(); // Store BCD representation of Vx in memory locations I
    void OP_Fx55(); // Store registers V0 through Vx in memory starting at location I
    void OP_Fx65(); // Read registers V0 through Vx from memory starting at
    void OP_NULL() {};
    // table

    void Table0()
    {
        ((*this).*(Table0Pointer[this->opcode & 0x000F]))();
    };

    void Table8()
    {
        ((*this).*(Table8Pointer[this->opcode & 0x000F]))();
    }
    void TableE()
    {
        ((*this).*(TableEPointer[this->opcode & 0x000F]))();
    }
    void TableF()
    {
        ((*this).*(TableFPointer[this->opcode & 0x000F]))();
    }

    // cycle through emulation
    void Cycle();
};