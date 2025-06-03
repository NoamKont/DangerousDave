#include "Pacman.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

using namespace pacman;
using namespace std;

int runDave();

int main() {
	// PacMan p;
	// if (p.valid())
	// 	p.run();
	// return 0;
    return runDave();
}
int runDave() {
	SDL_FRect srcDaveWalk2 = { 52, 12.2, 22, 15 };
    SDL_FRect srcDaveWalk1 = { 27, 12.2, 22, 15 };
    SDL_FRect srcPlatform = { 69, 88, 16, 16 };
    SDL_FRect* src; //pointer for current src

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Init Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* win = nullptr;
    SDL_Renderer* ren = nullptr;
    if (!SDL_CreateWindowAndRenderer("Dave", 800, 600, 0, &win, &ren)) {
        cout << SDL_GetError() << endl;
        return 1;
    }

    SDL_Surface* surf = IMG_Load("res/dave.png");
    if (!surf)
    {
        cout << "Image Load Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_DestroySurface(surf);
    if (!tex)
    {
        cout << "Texture Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Surface* surf2 = IMG_Load("res/dave2.png");
    if (!surf2)
    {
        cout << "Platform Image Load Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Texture* tex2 = SDL_CreateTextureFromSurface(ren, surf2);
    SDL_DestroySurface(surf2);
    if (!tex2)
    {
        cout << "Platform Texture Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_FRect r = {0, 300, 64, 64};
    SDL_FRect dstPlatform = { 0, 380, 64, 64 };

    float speed = 2.5f;

    for (int i = 0; i < 1000; ++i) ///Demonstrate the transition between levels
    {
        SDL_Event event;

        // Handle OS events (required on macOS to allow window to render and respond properly)
        // Without this loop, the window may freeze or not display correctly on macOS
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                i = 1000;
                break;
            }
        }

        r.x += speed;

        if((i / 10) % 2 == 0)
        {
            src = &srcDaveWalk1;
        }
        else
        {
            src = &srcDaveWalk2;
        }

        SDL_RenderClear(ren);

        for(int j = 0 ; j < 800 ; j+= 64)
        {
            ///Draw blocks above Dave
            dstPlatform = { float(j), 220, 64, 64 };
            SDL_RenderTexture(ren, tex2, &srcPlatform, &dstPlatform);

            ///Draw blocks below Dave
            dstPlatform = { float(j), 380, 64, 64 };
            SDL_RenderTexture(ren, tex2, &srcPlatform, &dstPlatform);
        }

        ///Draw dave
        SDL_RenderTexture(ren, tex, src, &r);
        SDL_RenderPresent(ren);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyTexture(tex2);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
