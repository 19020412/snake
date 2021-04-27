#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "gamebase.h"
#include "render.h"
#include "gamesystem.h"

void Start();

void Close();

void Menu();

void LeaderBoard();

void Game();

int GameLoop();

void Pause();

int GetInput();

void NewGame();

void Update();

Block MoveHead(Direction dir);

void PullBody(int rate);

void CreateFood();

int RDice(int n, int dice[]);

int CreateBrick();

void EatFood();