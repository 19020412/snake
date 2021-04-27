#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>    
#include <time.h>  
#include <math.h>
#include <sstream>

const int GAME_WIDTH = 30;
const int GAME_HEIGHGT = 30;

int idx(int x, int y)
{
	return x + y * GAME_WIDTH;
}

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGTH = 710;
const int BLOCK_SIZE = 20;

const int FRAME_RATE = 60;
const double GAME_SPEED = 20;

const SDL_Rect board_viewport
{
	(SCREEN_WIDTH - (BLOCK_SIZE * GAME_WIDTH)) / 2,
	SCREEN_HEIGTH - (BLOCK_SIZE * GAME_HEIGHGT) - 5,
	GAME_WIDTH * BLOCK_SIZE,
	GAME_HEIGHGT * BLOCK_SIZE
};

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

enum Block
{
	EMPTY_BLOCK,
	FOOD_BLOCK,
	BRICK_BLOCK,
	BODY_BLOCK,
	HEAD_BLOCK
};

struct Timer {
	Uint32 start;
	Uint32 current;
	Uint32 next_update;
	Uint32 next_render;

	Timer();
	void Reset();
	Uint32 GetTicks();
} timer;


int high_score[10] = { 0,0,0,0,0,0,0,0,0,0 };
struct Gamestate
{
	double speed;
	int score = 0;
	Uint32 tTime;
	int brick_count;

	Gamestate();
	void Reset();
	double Time();
} gState;

struct Gameboard
{
	//type of blocks
	Block type[GAME_WIDTH * GAME_HEIGHGT];
	//draw areas of blocks
	SDL_Rect rect[GAME_WIDTH * GAME_HEIGHGT];

	Gameboard();
} gameboard;

struct Snake
{
	Direction dir;
	int length; //include head
	int tail;
	int head_x;
	int head_y;
	//pointer connect from tail -> head
	int body[GAME_WIDTH * GAME_HEIGHGT];
	bool killed;
} snake;

struct GameTexture
{
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Rect* rect;
	void free();
	~GameTexture();
};

SDL_Window* window;
SDL_Renderer* renderer;

struct Menu
{
	int state = 0;
	GameTexture tex[3];
} menu;

GameTexture block_tex[5];

GameTexture score_viewport;
GameTexture leader_board;
GameTexture pause_window;
GameTexture empty_tex;

TTF_Font* font;