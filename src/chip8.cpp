#include "chip8.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <fstream>

const unsigned int START_ADDRESS = 0x200; // Programs start at memory location 0x200
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50; // Fontset starts at memory location 0x50
const u32 PIXEL_ON = 0xFFFFFFFF;
const u32 PIXEL_OFF = 0x00000000;

uint8_t fontset[FONTSET_SIZE] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()), randByte(0, 255U)
{
    this->pc = START_ADDRESS; // Program counter starts at 0x200

    // Load fontset into memory
    for (int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void Chip8::LoadROM(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        // Get the size of the ROM
        std::streampos size = file.tellg();
        char *buffer = new char[size];

        // Read the ROM into memory starting at 0x200
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (int i = 0; i < size; ++i)
        {
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
    else
    {
        std::cerr << "Failed to open ROM: " << filename << std::endl;
    };
}

// emulating all 34 opcodes
void Chip8::OP_00E0()
{
    // Clear the display
    memset(this->video, 0, sizeof(this->video));
}

void Chip8::OP_00EE()
{
    // Return from subroutine
    if (this->sp == 0)
        return;
    --this->sp;
    this->pc = this->stack[this->sp];
}

void Chip8::OP_1nnn()
{
    u16 address = this->opcode & 0x0FFF;
    this->pc = address;
}

void Chip8::OP_2nnn()
{
    u16 address = this->opcode & 0x0FFF;
    if (this->sp >= 16)
        return;
    this->stack[this->sp] = this->pc;
    ++this->sp;
    this->pc = address;
}

void Chip8::OP_3xkk()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 kk = this->opcode & 0x00FF;
    if (this->V[x] == kk)
        this->pc += 2;
}

void Chip8::OP_4xkk()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 kk = this->opcode & 0x00FF;
    if (this->V[x] != kk)
        this->pc += 2;
}

void Chip8::OP_5xy0()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    if (this->V[x] == this->V[y])
        this->pc += 2;
}

void Chip8::OP_6xkk()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 kk = this->opcode & 0x00FF;
    this->V[x] = kk;
}

void Chip8::OP_7xkk()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 kk = this->opcode & 0x00FF;
    this->V[x] = this->V[x] + kk;
}

void Chip8::OP_8xy0()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[x] = this->V[y];
}

void Chip8::OP_8xy1()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[x] |= this->V[y];
}

void Chip8::OP_8xy2()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[x] &= this->V[y];
}

void Chip8::OP_8xy3()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[x] ^= this->V[y];
}

void Chip8::OP_8xy4()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    u16 sum = this->V[x] + this->V[y];
    this->V[0xF] = (sum > 0xFF) ? 1 : 0;
    this->V[x] = sum & 0xFF;
}

void Chip8::OP_8xy5()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[0xF] = (this->V[x] > this->V[y]) ? 1 : 0;
    this->V[x] = this->V[x] - this->V[y];
}

void Chip8::OP_8xy6()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->V[0xF] = this->V[x] & 0x1;
    this->V[x] >>= 1;
}

void Chip8::OP_8xy7()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    this->V[0xF] = (this->V[y] > this->V[x]) ? 1 : 0;
    this->V[x] = this->V[y] - this->V[x];
}

void Chip8::OP_8xyE()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->V[0xF] = (this->V[x] & 0x80) >> 7;
    this->V[x] <<= 1;
}

void Chip8::OP_9xy0()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    if (this->V[x] != this->V[y])
        this->pc += 2;
}

void Chip8::OP_Annn()
{
    u16 address = this->opcode & 0x0FFF;
    this->I = address;
}

void Chip8::OP_Bnnn()
{
    u16 address = this->opcode & 0x0FFF;
    this->pc = address + this->V[0];
}

void Chip8::OP_Cxkk()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 kk = this->opcode & 0x00FF;
    this->V[x] = (this->randByte(this->randGen) & kk);
}

void Chip8::OP_Dxyn()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 y = (this->opcode & 0x00F0) >> 4;
    u8 height = this->opcode & 0x000F;
    u8 vx = this->V[x] % 64;
    u8 vy = this->V[y] % 32;
    this->V[0xF] = 0;

    for (int row = 0; row < height; ++row)
    {
        if (this->I + row >= 4096)
            break;
        u8 spriteByte = this->memory[this->I + row];
        for (int col = 0; col < 8; ++col)
        {
            u8 spritePixel = (spriteByte >> (7 - col)) & 0x1;
            if (spritePixel == 0)
                continue;

            u32 px = (vx + col) % 64;
            u32 py = (vy + row) % 32;
            u32 index = px + (py * 64);

            if (this->video[index] == PIXEL_ON && spritePixel == 1)
            {
                this->V[0xF] = 1;
            }

            this->video[index] = (this->video[index] == PIXEL_ON) ? PIXEL_OFF : PIXEL_ON;
        }
    }
}

void Chip8::OP_Ex9E()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 keyIndex = this->V[x];
    if (keyIndex < 16 && this->keypad[keyIndex])
        this->pc += 2;
}

void Chip8::OP_ExA1()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 keyIndex = this->V[x];
    if (keyIndex >= 16 || !this->keypad[keyIndex])
        this->pc += 2;
}

void Chip8::OP_Fx07()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->V[x] = this->delay_timer;
}

void Chip8::OP_Fx0A()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    bool keyPressed = false;
    for (u8 i = 0; i < 16; ++i)
    {
        if (this->keypad[i])
        {
            this->V[x] = i;
            keyPressed = true;
            break;
        }
    }
    if (!keyPressed)
    {
        this->pc -= 2; // repeat this instruction
    }
}

void Chip8::OP_Fx15()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->delay_timer = this->V[x];
}

void Chip8::OP_Fx18()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->sound_timer = this->V[x];
}

void Chip8::OP_Fx1E()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->I = this->I + this->V[x];
}

void Chip8::OP_Fx29()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    this->I = FONTSET_START_ADDRESS + (this->V[x] * 5);
}

void Chip8::OP_Fx33()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    u8 value = this->V[x];
    if (this->I + 2 >= 4096)
        return;
    this->memory[this->I + 2] = value % 10;
    value /= 10;
    this->memory[this->I + 1] = value % 10;
    value /= 10;
    this->memory[this->I] = value % 10;
}

void Chip8::OP_Fx55()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    for (u8 i = 0; i <= x; ++i)
    {
        if (this->I + i >= 4096)
            break;
        this->memory[this->I + i] = this->V[i];
    }
}

void Chip8::OP_Fx65()
{
    u8 x = (this->opcode & 0x0F00) >> 8;
    for (u8 i = 0; i <= x; ++i)
    {
        if (this->I + i >= 4096)
            break;
        this->V[i] = this->memory[this->I + i];
    }
}

void Chip8::Cycle()
{
    this->opcode = (this->memory[this->pc] << 8) | this->memory[this->pc + 1];
    this->pc += 2;

    // decode instruction
    switch (this->opcode & 0xF000)
    {
    case 0x0000:
        switch (this->opcode & 0x00FF)
        {
        case 0xE0:
            this->OP_00E0();
            break;
        case 0xEE:
            this->OP_00EE();
            break;
        default:
            break;
        }
        break;
    case 0x1000:
        this->OP_1nnn();
        break;
    case 0x2000:
        this->OP_2nnn();
        break;
    case 0x3000:
        this->OP_3xkk();
        break;
    case 0x4000:
        this->OP_4xkk();
        break;
    case 0x5000:
        this->OP_5xy0();
        break;
    case 0x6000:
        this->OP_6xkk();
        break;
    case 0x7000:
        this->OP_7xkk();
        break;
    case 0x8000:
        switch (this->opcode & 0x000F)
        {
        case 0x0:
            this->OP_8xy0();
            break;
        case 0x1:
            this->OP_8xy1();
            break;
        case 0x2:
            this->OP_8xy2();
            break;
        case 0x3:
            this->OP_8xy3();
            break;
        case 0x4:
            this->OP_8xy4();
            break;
        case 0x5:
            this->OP_8xy5();
            break;
        case 0x6:
            this->OP_8xy6();
            break;
        case 0x7:
            this->OP_8xy7();
            break;
        case 0xE:
            this->OP_8xyE();
            break;
        default:
            break;
        }
        break;
    case 0x9000:
        this->OP_9xy0();
        break;
    case 0xA000:
        this->OP_Annn();
        break;
    case 0xB000:
        this->OP_Bnnn();
        break;
    case 0xC000:
        this->OP_Cxkk();
        break;
    case 0xD000:
        this->OP_Dxyn();
        break;
    case 0xE000:
        switch (this->opcode & 0x00FF)
        {
        case 0x9E:
            this->OP_Ex9E();
            break;
        case 0xA1:
            this->OP_ExA1();
            break;
        default:
            break;
        }
        break;
    case 0xF000:
        switch (this->opcode & 0x00FF)
        {
        case 0x07:
            this->OP_Fx07();
            break;
        case 0x0A:
            this->OP_Fx0A();
            break;
        case 0x15:
            this->OP_Fx15();
            break;
        case 0x18:
            this->OP_Fx18();
            break;
        case 0x1E:
            this->OP_Fx1E();
            break;
        case 0x29:
            this->OP_Fx29();
            break;
        case 0x33:
            this->OP_Fx33();
            break;
        case 0x55:
            this->OP_Fx55();
            break;
        case 0x65:
            this->OP_Fx65();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (this->delay_timer > 0)
        --this->delay_timer;
    if (this->sound_timer > 0)
        --this->sound_timer;
}
