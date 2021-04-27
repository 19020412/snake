#pragma once
#include "SDL.h"
#include "gamebase.h"

void Close();

void GameInit();

void SaveScore();

void LoadRes();

SDL_Texture* CreateBlockTexture(Block type);

void LoadTexure(GameTexture& t, const char* path);

void SaveScore();