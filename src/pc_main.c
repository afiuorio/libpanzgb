#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gb-emu/gb.h"

#define NUM_OP_60HZ (GB_CLOCK / 59.7) * 1

#define SCALE 5

void makeSaveState(gb *cpu) {
    FILE *f = fopen((char *)cpu->cartridge + NAME_CART, "wb");

    fwrite(cpu, sizeof(gb), 1, f);
    printf("Created save state\n");
    fclose(f);
}

void loadSaveState(gb *cpu) {
    FILE *f = fopen((char *)cpu->cartridge + NAME_CART, "rb");
    if (f == NULL)
        return;

    fread(cpu, sizeof(gb), 1, f);
    printf("Loaded save state\n");
    setGbBanking(cpu);
    fclose(f);
}

void doScreenshoot(SDL_Renderer *renderer) {
    time_t name;
    name = time(NULL);
    SDL_Surface *sshot =
        SDL_CreateRGBSurface(0, 160 * SCALE, 144 * SCALE, 32, 0x00ff0000,
                             0x0000ff00, 0x000000ff, 0xff000000);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                         sshot->pixels, sshot->pitch);
    SDL_SaveBMP(sshot, ctime(&name));
    SDL_FreeSurface(sshot);
}

void renderScreen(gb *cpu, SDL_Renderer *rend, SDL_Surface *surface) {
    int x, y;
    int j;
    BYTE color = 0x00;
    SDL_Rect rectangle;
    rectangle.h = SCALE;
    for (y = 0; y < 144; y++) {
        for (x = 0; x < 160; x++) {
            BYTE col = cpu->screenData[y][x];
            for (j = x + 1; col == cpu->screenData[y][j] && j < 160; j++)
                ;
            switch (col) {
            case 0:
                color = 0xff;
                break;
            case 1:
                color = 0xcc;
                break;
            case 2:
                color = 0x77;
                break;
            case 3:
                color = 0x00;
                break;
            }
            rectangle.w = SCALE * (j - x);
            rectangle.h = SCALE;
            rectangle.x = x * SCALE;
            rectangle.y = y * SCALE;
            x = j - 1;
            SDL_FillRect(surface, &rectangle,
                         SDL_MapRGBA(surface->format, color, color, color, 0));
        }
    }
    /*I probably can do better than this every frame*/
    SDL_Texture *texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_RenderCopy(rend, texture, NULL, NULL);
    SDL_RenderPresent(rend);
    SDL_DestroyTexture(texture);
}

void getInput(gb *cpu, SDL_Renderer *rend) {
    SDL_Event event;
    SIGNED_BYTE key = 0;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(EXIT_SUCCESS);
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
            case SDLK_z:
                key = 4;
                break;
            case SDLK_x:
                key = 5;
                break;
            case SDLK_RETURN:
                key = 7;
                break;
            case SDLK_SPACE:
                key = 6;
                break;
            case SDLK_UP:
                key = 2;
                break;
            case SDLK_LEFT:
                key = 1;
                break;
            case SDLK_RIGHT:
                key = 0;
                break;
            case SDLK_DOWN:
                key = 3;
                break;

            case SDLK_f:
                doScreenshoot(rend);
                return;
            case SDLK_s:
                makeSaveState(cpu);
                return;
            case SDLK_d:
                loadSaveState(cpu);
                return;

            default:
                return;
            }
            if (event.type == SDL_KEYUP) {
                keyReleased(cpu, key);
            } else {
                keyPressed(cpu, key);
            }
        }
    }
}
gb *gameboy;

int main(int argc, char **argv) {
    gameboy = malloc(sizeof(gb));
    if (!gameboy) {
        fprintf(stderr, "Error on memory allocation\n");
        exit(EXIT_FAILURE);
    }
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("pangb", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, 160 * SCALE, 144 * SCALE,
                              SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *surface =
        SDL_CreateRGBSurface(0, 160 * SCALE, 144 * SCALE, 32, 0, 0, 0, 0);
    loadROM(gameboy, argv[1]);
    initGameBoy(gameboy);
    unsigned int numOperation = 0;
    while (1) {
        unsigned int timeStartFrame = SDL_GetTicks();
        getInput(gameboy, renderer);
        while (numOperation < NUM_OP_60HZ)
            numOperation += executeGameBoy(gameboy);
        numOperation -= NUM_OP_60HZ;
        renderScreen(gameboy, renderer, surface);
        float deltaT =
            (float)1000 / (60) - (float)(SDL_GetTicks() - timeStartFrame);
        // printf("delta %f\n",deltaT);
        if (deltaT > 0)
            SDL_Delay((unsigned int)deltaT);
    }
}
