#include "iostream"
#include "chip8.cpp"
#include "display.cpp"
#include "keyboard.cpp"

const int emuW = 64;
const int emuH = 32;
const int windowW = 800;
const int windowH = 400;

int main(int argc, char** argv) {
    if(argc < 2) {
        cerr << "No Program path provided" << endl;
        return 1;
    }
    cout << "Instantiating Emulator ..." << endl;
    Chip8 cpu{};
    Display display{emuW, emuH, windowW, windowH};
    Keyboard keyboard;

    cout << "Loading Program..." << endl;
    cpu.loadProgram(argv[1]);

    cout << "Running Emulator..." << endl;
    while(!display.close) {
        keyboard.setInput(cpu.getKeyboard());
        cpu.cycle();
        display.update(cpu.getDisplay());
    }

    return 0;
}