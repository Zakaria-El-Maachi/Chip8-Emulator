#pragma once
#include "raylib.h"
#include <stdlib.h>
#include <cstdint>

using namespace std;

class Display {

    Color* pixels;
    Texture2D emuTexture;

    int emuW;
    int emuH;
    int windowW;
    int windowH;

    public : 

        bool close = false;

        Display(int emulatorWidth, int emulatorHeight, int windowWidth, int windowHeight)  {
            emuW = emulatorWidth;
            emuH =emulatorHeight;
            windowW = windowWidth;
            windowH = windowHeight;


            InitWindow(windowW, windowH, "Chip8 Emulator");

            // Create the Pixel Buffer (CPU Side)
            // We use Color because Raylib's UpdateTexture expects an array of 4-byte Colors
            pixels = (Color *)malloc(emuW * emuH * sizeof(Color));

            // Create the Texture (GPU Side)
            // We start with an empty Image to define the format
            Image tempImage = GenImageColor(emuW, emuH, BLACK);
            emuTexture = LoadTextureFromImage(tempImage);
            UnloadImage(tempImage); // Clean up the temp image
            
            // Set filter to POINT so scaling looks sharp, not blurry
            SetTextureFilter(emuTexture, TEXTURE_FILTER_POINT);

            SetTargetFPS(60);
        }

        void update(const uint32_t* chip8Display) {
            close = WindowShouldClose();
            if(close) {
                free(pixels);
                UnloadTexture(emuTexture);
                CloseWindow();
                return;
            }
            RefreshTexture(chip8Display);
            // Draw and Scale ---
            BeginDrawing();
                // ClearBackground(DARKGRAY);

                // Define source (the whole tiny texture)
                Rectangle source = { 0, 0, (float)emuW, (float)emuH };
                // Define destination (the whole big window)
                Rectangle dest = { 0, 0, (float)windowW, (float)windowH };

                DrawTexturePro(emuTexture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            EndDrawing();
        }

        void RefreshTexture(const uint32_t* chip8Display) {

            for (int i = 0; i < emuW * emuH; i++) {
                if(chip8Display[i]) pixels[i] = WHITE;
                else pixels[i] = BLACK;
            }

            // Push the translated colors to the GPU
            UpdateTexture(emuTexture, pixels);
        }
};