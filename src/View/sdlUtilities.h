#ifndef SDLUTILITIES_H
#define SDLUTILITIES_H

#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "render.h"
#include "display.h"
#include "constants.h"
#include "sdlUtilities.h"
/* main events handling function */
void eventMain();

/* main menu events handling */
void mainMenuEvents(SDL_Window *window, SDL_Renderer *renderer);

#endif
