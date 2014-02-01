/******************************************************************************/
/* display.h: header file for displaying chess game graphics                  */
/******************************************************************************/
/* Author: Ryan M. Morison                                                    */
/*                                                                            */
/* History:                                                                   */
/*          12/22/13 initial file creation; draw window with checkerboard     */
/*          12/28/13 draw pieces on board                                     */
/*          01/18/14 draw main menu                                           */
/******************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>

/* function to display the main menu to the screen */
int drawMainMenu(SDL_Window *window, SDL_Renderer *renderer, int *screenMode);

/* function to display the one player menu to the screen */
int drawOnePlayerMenu(SDL_Window *window, SDL_Renderer *renderer, int *screenMode);

/* function to display the one player menu to the screen */
int drawTwoPlayerMenu(SDL_Window *window, SDL_Renderer *renderer, int *screenMode);

/* function to display the advanced menu to the screen */
void drawAdvancedMenu(SDL_Window *window, SDL_Renderer *renderer);

/* function to display the gameplay window to the screen */
void drawGameplayScreen(SDL_Window *window, SDL_Renderer *renderer);

/* function to display the left player window */
void drawLeftPlayerWindow(SDL_Renderer *renderer, int tileSize, int playerMode);

/* function to display the right player window */
void drawRightPlayerWindow(SDL_Renderer *renderer, int tileSize, int playerMode);

/* function to display the left player window */
void drawRightPlayerWindow(SDL_Renderer *renderer, int tileSize, int playerMode);

/* function to display a blank chessboard to the screen */
void drawChessboard(SDL_Renderer *renderer);

/* function to display the pieces in beginning position on the chessboard */
void drawPieces(SDL_Renderer *renderer);

void drawBox(SDL_Renderer *boxRenderer, int x, int y, int w, int h);

void clearBox(SDL_Renderer *boxRenderer, int x, int y, int w, int h);

/* function to uninitialize SDL systems */
void clean();

#endif
