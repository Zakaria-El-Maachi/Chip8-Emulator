#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>

class Chip8;

using namespace std;
using instruction = void (Chip8::*)();

uint8_t fontset[80] =
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

class Chip8 {
    uint8_t registers[16];
    uint16_t stack[16];
    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint8_t delayTimer;
    uint8_t soundTimer;

    uint8_t memory[4096];
    uint32_t display[64 * 32];
    uint8_t keyboard[16];

    instruction decodingTable[16] = {
        &Chip8::decode0, &Chip8::x1nnn, &Chip8::x2nnn, &Chip8::x3xkk,
        &Chip8::x4xkk, &Chip8::x5xy0, &Chip8::x6xkk, &Chip8::x7xkk,
        &Chip8::decode8, &Chip8::x9xy0, &Chip8::xAnnn, &Chip8::xBnnn,
        &Chip8::xCxkk, &Chip8::xDxyn, &Chip8::decodeE, &Chip8::decodeF,
    };

    uint16_t opcode;
    uint8_t x, y, n, kk;
    uint16_t addr;

    const uint16_t fontStartAddr = 0x50;

    public: 
        Chip8() {
            pc = 0x200;
            sp = 0;
            index = 0;
            delayTimer = 0;
            soundTimer = 0;
            memset(registers, 0, sizeof(registers));
            memset(memory, 0, sizeof(memory));
            memset(display, 0, sizeof(display));
            memset(keyboard, 0, sizeof(keyboard));

            for(int offset = 0; offset < 80; offset++) {
                memory[fontStartAddr + offset] = fontset[offset];
            }
        }

        void loadProgram(string programPath) {
            fstream program(programPath, ios::binary | ios::in);
            
            if(!program.is_open()) cerr << "Program could not be opened and copied to memory" << endl;
            program.read(reinterpret_cast<char*>(memory + 0x200) , 0xFFF-0x200);
            program.close();
        }

        const uint32_t* getDisplay() const {
            return display;
        }

        uint8_t* getKeyboard() {
            return keyboard;
        }
    
    private: 
        void x00e0() {
            memset(display, 0, sizeof(display));
        }

        void x00ee() {
            pc = stack[sp];
            sp--;
        }

        void x1nnn() {
            pc = addr;
        }

        void x2nnn() {
            sp++;
            stack[sp] = pc;
            pc = addr;
        }

        void x3xkk() {
            if(registers[x] == kk) pc += 2;
        }

        void x4xkk() {
            if(registers[x] != kk) pc += 2;
        }

        void x5xy0() {
            if(registers[x] == registers[y]) pc += 2;
        }

        void x6xkk() {
            registers[x] = kk;
        }

        void x7xkk() {
            registers[x] += kk;
        }

        void x8xy0() {
            registers[x] = registers[y];
        }

        void x8xy1() {
            registers[x] |= registers[y];
        }

        void x8xy2() {
            registers[x] &= registers[y];
        }

        void x8xy3() {
            registers[x] ^= registers[y];
        }

        void x8xy4() {
            uint16_t sum = registers[x] + registers[y];

            if(sum > 0xFF) registers[0xF] = 1;
            else registers[0xF] = 0;

            registers[x] = sum;
        }

        void x8xy5() {
            if(registers[x] > registers[y]) registers[0xF] = 1;
            else registers[0xF] = 0;

            registers[x] -= registers[y];
        }

        void x8xy6() {
            if(registers[x] & 0x0001) registers[0xF] = 1;
            else registers[0xF] = 0;

            registers[x] >>= 1;
        }

        void x8xy7() {
            if(registers[y] > registers[x]) registers[0xF] = 1;
            else registers[0xF] = 0;

            registers[x] = registers[y] - registers[x];
        }

        void x8xyE() {
            if(registers[x] & 0x8000) registers[0xF] = 1;
            else registers[0xF] = 0;

            registers[x] <<= 1;
        }

        void x9xy0() {
            if(registers[x] != registers[y]) pc += 2;
        }

        void xAnnn() {
            index = addr;
        }

        void xBnnn() {
            pc = addr + registers[0x0];
        }

        void xCxkk() {            
            registers[x] = rand() & kk;
        }

        void xDxyn() {
            uint8_t collision = 0;

            uint8_t byte, bit;
            uint64_t bitScaled;
            uint16_t pixel;

            registers[0xF] = 0;
            
            for(uint8_t offset = 0; offset < n; offset++) {
                byte = memory[index + offset];
                
                for(uint8_t position = 0; position < 8; position++) {
                    bit = ((0x80u >> position) & byte)? 1:0;
                    pixel = ((registers[y] + offset) % 32) * 64 + ((registers[x] + position) % 64);

                    if(bit) bitScaled = 0xFFFFFFFFu;
                    else bitScaled = 0u;

                    collision |= display[pixel] & bitScaled;
                    display[pixel] ^= bitScaled;
                }
            }

            if(collision) registers[0xF] = collision;
        }

        void xEx9E() {            
            if(keyboard[registers[x]]) pc += 2;
        }

        void xExA1() {
            if(!keyboard[registers[x]]) pc += 2;
        }

        void xFx07() {
            registers[x] = delayTimer;
        }

        void xFx0A() {
            for(uint8_t key = 0; key < 16; key++) {
                if(keyboard[key]) {
                    pc += 2;
                    registers[x] = key;
                    break;
                }
            }

            pc -= 2;
        }

        void xFx15() {
            delayTimer = registers[x];
        }

        void xFx18() {
            soundTimer = registers[x];
        }

        void xFx1E() {
            index += registers[x];
        }

        void xFx29() {
            index = fontStartAddr + 5 * registers[x];
        }

        void xFx33() {
            uint8_t value = registers[x];
            for(int offset = 2; offset >= 0; offset--) {
                memory[index+offset] = value%10;
                value /= 10;
            }
        }

        void xFx55() {
            for(uint8_t Vi = 0; Vi <= x; Vi++) {
                memory[index + Vi] = registers[Vi];
            }
        }

        void xFx65() {
            for(uint8_t Vi = 0; Vi <= x; Vi++) {
                registers[Vi] = memory[index + Vi];
            }
        }


        void decode0() {
            uint8_t filter = opcode & 0x000Fu;
            if(filter == 0x0u) return x00e0();
            else if(filter == 0xEu) return x00ee();
        }

        void decode8() {
            uint8_t filter = opcode & 0x000Fu;
            switch(filter) {
                case 0x0u:
                    return x8xy0();
                case 0x1u:
                    return x8xy1();
                case 0x2u:
                    return x8xy2();
                case 0x3u:
                    return x8xy3();
                case 0x4u:
                    return x8xy4();
                case 0x5u:
                    return x8xy5();
                case 0x6u:
                    return x8xy6();
                case 0x7u:
                    return x8xy7();
                case 0xEu:
                    return x8xyE();
            }
        }

        void decodeE() {
            uint8_t filter = opcode & 0x000Fu;
            if(filter == 0xEu) return xEx9E();
            else if(filter == 0x1u) return xExA1();
        }

        void decodeF() {
            uint8_t filter = opcode & 0x00FFu;
            switch(filter) {
                case 0x07u:
                    return xFx07();
                case 0x0Au:
                    return xFx0A();
                case 0x15u:
                    return xFx15();
                case 0x18u:
                    return xFx18();
                case 0x1Eu:
                    return xFx1E();
                case 0x29u:
                    return xFx29();
                case 0x33u:
                    return xFx33();
                case 0x55u:
                    return xFx55();
                case 0x65u:
                    return xFx65();
            }
        }

        instruction decode() {

            addr = opcode & 0x0FFFu;
            x = (opcode & 0x0F00u) >> 8;
            y = (opcode & 0x00F0u) >> 4;
            kk = opcode & 0x00FFu;
            n = opcode & 0x000Fu;

            uint8_t filter = (opcode & 0xF000u) >> 12;

            return decodingTable[filter];
        }

        void decrementTimers() {
            if(delayTimer) delayTimer--;
            if(soundTimer) soundTimer--;
        }

    
    public: 
        void cycle() {
            opcode = (memory[pc]<<8) + memory[pc+1]; // fetch
            pc += 2; // increment pc
            instruction ist = decode(); // decode
            (this->*ist)(); // execute instruction
            decrementTimers(); // decrementTimers
        }

};