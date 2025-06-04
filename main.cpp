#include "Pacman.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "dave_game.h"

using namespace pacman;
using namespace dave_game;
using namespace std;

int main() {
    DaveGame daveGame;
    if (daveGame.valid())
        daveGame.run();
    return 0;
}

