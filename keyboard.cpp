#pragma once
#include <raylib.h>
#include <cstdint>

class Keyboard {

    const KeyboardKey keys[16] {
        KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR,
        KEY_Q, KEY_W, KEY_E, KEY_R,
        KEY_A, KEY_S, KEY_D, KEY_F,
        KEY_Z, KEY_X, KEY_C, KEY_V
    };
    public: 
        void setInput(uint8_t* keyboard) {
            for(int i = 0; i < 16; i++) {
                if(IsKeyDown(keys[i])) keyboard[i] = 1;
                else if(IsKeyUp(keys[i])) keyboard[i] = 0;
            }
        }
};