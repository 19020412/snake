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


void Close();

void NewGame();

void GameInit();

void LoadRes();

void BoardRender();

void Render();

Block MoveHead(Direction dir);

void PullBody(int rate);

void CreateFood();

SDL_Texture* CreateBlockTexture(Block type);

void Update();

int GameLoop();

int GetInput();

void Start();

void renderScoreViewport();

void Pause();

void Menu();

void Game();

void LeaderBoard();

void LoadTexure(GameTexture& t, const char* path);

void RenderMenu();

void RenderPauseScreen();

void SaveScore();

int RDice(int n, int dice[]);

int CreateBrick();

void EatFood();

void DerenderPauseScreen(); 

int main(int arg, char* args[])
{
	Start();
	return 0;
}

void Start()
{
	GameInit();
	Menu();
	Close();
}

void Render()
{
	BoardRender();
	renderScoreViewport();
	SDL_RenderPresent(renderer);
}

void Close()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void PullBody(int rate)
{
	for (int i = 0; i < rate; i++)
	{
		//snake.tail = snake.body[snake.tail];
		gameboard.type[snake.tail] = EMPTY_BLOCK;
		snake.tail = snake.body[snake.tail];
		snake.length--;
	}
}

//return the type of destination's current block
Block MoveHead()
{

	snake.length++;

	Block d_block;	//return value

	int old_x = snake.head_x;
	int old_y = snake.head_y;

	Direction dir = snake.dir;
	switch (dir)
	{
	case UP:
		snake.head_y--;
		break;
	case RIGHT:
		snake.head_x++;
		break;
	case DOWN:
		snake.head_y++;
		break;
	case LEFT:
		snake.head_x--;
		break;
	}

	//warp
	snake.head_x = (GAME_WIDTH + snake.head_x) % GAME_WIDTH;
	snake.head_y = (GAME_HEIGHGT + snake.head_y) % GAME_HEIGHGT;

	//index of old/new head coordinate
	int old_idx = idx(old_x, old_y);
	int next_idx = idx(snake.head_x, snake.head_y);

	d_block = gameboard.type[next_idx];
	gameboard.type[old_idx] = BODY_BLOCK;
	gameboard.type[next_idx] = HEAD_BLOCK;
	snake.body[old_idx] = next_idx;
	return d_block;
}


void LoadRes()
{
	font = TTF_OpenFont("res/font/minecraft.ttf", 16);

	for (int i = 0; i < 5; i++)
		CreateBlockTexture((Block)i);

	LoadTexure(menu.tex[0], "res/m1.png");
	LoadTexure(menu.tex[1], "res/m2.png");
	LoadTexure(menu.tex[2], "res/m3.png");
	LoadTexure(leader_board, "res/leaderboard.png");
	LoadTexure(pause_window, "res/pausewindow.png");
	LoadTexure(score_viewport, "res/scoreview.png");
	LoadTexure(empty_tex, "res/empty.png");

	std::ifstream is("data.bin", std::ios::in, std::ios::binary);
	for (int i = 0; i < 10; i++)
		is.read((char*)&high_score[i], sizeof(int));
	if (!is.good()) std::cout << "read file err";
	is.close();
}

void LoadTexure(GameTexture& t, const char* path)
{
	t.free();
	t.surface = IMG_Load(path);
	t.texture = SDL_CreateTextureFromSurface(renderer, t.surface);
	SDL_GetClipRect(t.surface, t.rect);
}

void CreateFood()
{
	int i;
	do
	{
		i = rand() % idx(GAME_WIDTH, GAME_HEIGHGT);
	} while (gameboard.type[i] != EMPTY_BLOCK);
	gameboard.type[i] = FOOD_BLOCK;
}


SDL_Texture* CreateBlockTexture(Block type)
{
	//destroy old texture
	block_tex[type].free();

	//create drawing surface
	SDL_Rect* rect = new SDL_Rect{
		0,
		0,
		BLOCK_SIZE,
		BLOCK_SIZE };
	SDL_Surface* s = SDL_CreateRGBSurface(0, BLOCK_SIZE, BLOCK_SIZE, 32,
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	//draw 
	Uint32 color_code = 0;
	switch (type)
	{
	case EMPTY_BLOCK:
		color_code = SDL_MapRGBA(s->format, 0, 0, 0, 255);
		break;
	case FOOD_BLOCK:
		color_code = SDL_MapRGBA(s->format, 0, 255, 0, 255);
		break;
	case BRICK_BLOCK:
		color_code = SDL_MapRGBA(s->format, 255, 255, 255, 255);
		break;
	case BODY_BLOCK:
		color_code = SDL_MapRGBA(s->format, 255, 255, 255, 255);
		break;
	case HEAD_BLOCK:
		color_code = SDL_MapRGBA(s->format, 0, 0, 255, 255);
		break;
	}
	SDL_FillRect(s, rect, color_code);

	//create texture
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, s);
	block_tex[type].texture = tex;
	block_tex[type].surface = s;
	block_tex[type].rect = rect;

	return tex;
}

void BoardRender()
{
	SDL_RenderSetViewport(renderer, &board_viewport);
	SDL_Rect srcrect{
		0,
		0,
		BLOCK_SIZE,
		BLOCK_SIZE
	};

	int type;
	for (int i = 0; i < GAME_WIDTH * GAME_HEIGHGT; i++)
	{
		type = gameboard.type[i];
		SDL_RenderCopy(renderer, block_tex[type].texture, block_tex[type].rect, &gameboard.rect[i]);
	}

}

void NewGame()
{
	gState.Reset();
	for (int i = 0; i < GAME_WIDTH * GAME_HEIGHGT; i++)
		gameboard.type[i] = EMPTY_BLOCK;

	//create snake
	snake.dir = DOWN;
	snake.length = 1;
	snake.tail = idx(1, 1);
	snake.head_x = 1;
	snake.head_y = 1;
	snake.killed = false;
	for (int i = 1; i < 4; i++)
		MoveHead();

	CreateFood();
}

void GameInit()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	IMG_Init(IMG_INIT_PNG);

	window = SDL_CreateWindow("snake", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGTH, 0);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_TARGETTEXTURE);

	TTF_Init();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 256);
	srand(time(0));
	LoadRes();
}

void Update()
{
	if (snake.killed) return;
	switch (MoveHead())
	{
	case EMPTY_BLOCK:
		PullBody(1);
		break;

	case BODY_BLOCK:
		PullBody(1);
		snake.killed = true;
		break;
	case BRICK_BLOCK:
		PullBody(1);
		snake.killed = true;
		break;
	case FOOD_BLOCK:
		EatFood();
		break;
	}
}

int GameLoop()
{
	int running = 0;
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return -1;
	}

	running = GetInput();

	Uint32 time = timer.GetTicks();

	while (time >= timer.next_update)
	{
		Update();
		timer.next_update += 1000 / GAME_SPEED;
	}

	if (time >= timer.next_render)
	{
		Render();
		timer.next_render = time + 1000 / FRAME_RATE;
	}

	Uint32 w = std::min(timer.next_render, timer.next_update) - time;
	SDL_Delay(w);

	return running;
}

int GetInput()
{
	const Uint8* keyStates = SDL_GetKeyboardState(NULL);
	if (keyStates[SDL_SCANCODE_UP])
	{
		if (snake.dir != DOWN)
			snake.dir = UP;
	}
	else if (keyStates[SDL_SCANCODE_DOWN])
	{
		if (snake.dir != UP)
			snake.dir = DOWN;
	}
	else if (keyStates[SDL_SCANCODE_LEFT])
	{
		if (snake.dir != RIGHT)
			snake.dir = LEFT;
	}
	else if (keyStates[SDL_SCANCODE_RIGHT])
	{
		if (snake.dir != LEFT)
			snake.dir = RIGHT;
	}
	else if (keyStates[SDL_SCANCODE_ESCAPE])
	{
		if (!snake.killed) return 1;
		else return 2;
	}
	return 0;
}

void renderScoreViewport()
{
	int hscore;
	SDL_Color tcolor;
	if (gState.score < high_score[0]) {
		tcolor = { 255, 255, 255, 255 };
		hscore = high_score[0];
	}
	else {
		tcolor = { 0, 255, 0, 255 };
		hscore = gState.score;
	}

	SDL_Rect rect = {0, 0, 900, 105};
	SDL_RenderSetViewport(renderer, &rect);
	SDL_RenderCopy(renderer, score_viewport.texture, NULL, NULL);
	std::stringstream sscore;
	sscore << std::setfill('0') << std::setw(6) << gState.score;
	std::string score = sscore.str();

	std::stringstream sh;
	sh << "HI  ";
	sh << std::setfill('0') << std::setw(6) << hscore;
	std::string his = sh.str();

	int w, h;
	TTF_SizeText(font, his.c_str(), &w, &h);


	SDL_Surface* tsurface = TTF_RenderText_Solid(font, his.c_str(), tcolor);
	SDL_Texture* ttexture = SDL_CreateTextureFromSurface(renderer, tsurface);
	SDL_Rect drect = { 80, 42, (w * 25) / h, 25 };
	SDL_RenderCopy(renderer, ttexture, NULL, &drect);
	SDL_FreeSurface(tsurface);
	SDL_DestroyTexture(ttexture);

	TTF_SizeText(font, score.c_str(), &w, &h);
	tsurface = TTF_RenderText_Solid(font, score.c_str(), tcolor);
	ttexture = SDL_CreateTextureFromSurface(renderer, tsurface);
	drect = { 720, 42, (w * 25) / h, 25 };
	SDL_RenderCopy(renderer, ttexture, NULL, &drect);
	SDL_FreeSurface(tsurface);
	SDL_DestroyTexture(ttexture);
}

void Menu()
{
	RenderMenu();
	bool select = false;
	SDL_Event e;
	while (!select)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				return;
			}
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_UP:
					menu.state--;
					break;
				case SDLK_DOWN:
					menu.state++;
					break;
				case SDLK_RETURN:
					select = true;
					break;
				default:
					break;
				}
				if (menu.state < 0) menu.state = 0;
				else if (menu.state >= 2) menu.state = 2;
				RenderMenu();
			}
		}
		SDL_Delay(1000 / 60);
	}

	if (menu.state == 0)
	{
		NewGame();
		Game();
	}
	else if (menu.state == 1) LeaderBoard();
	else if (menu.state == 2) return;
}

void Game()
{
	int running = 0;
	timer.Reset();
	while (running == 0)
	{
		running = GameLoop();
	}
	SaveScore();
	if (running == -1) return;
	else if (running == 1) Pause();
	else if (running == 2) Menu();
}

void RenderPauseScreen()
{
	SDL_Rect viewport = { 750, 110, 150, 600 };
	SDL_RenderSetViewport(renderer, &viewport);
	SDL_RenderCopy(renderer, pause_window.texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void DerenderPauseScreen()
{
	SDL_Rect viewport = { 750, 110, 150, 600 };
	SDL_RenderSetViewport(renderer, &viewport);
	SDL_RenderCopy(renderer, empty_tex.texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Pause()
{
	RenderPauseScreen();
	SDL_Event e;
	int select = 0;
	while (select == 0)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				return;
			}
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_1:
					select = 1;
					break;
				case SDLK_2:
					select = 2;
					break;
				default:
					break;
				}
			}
		}
		SDL_Delay(1000 / 60);
	}
	DerenderPauseScreen();
	if (select == 1) Game();
	if (select == 2) Menu();
}

void LeaderBoard()
{
	SDL_RenderSetViewport(renderer, NULL);
	SDL_RenderCopy(renderer, leader_board.texture, NULL, NULL);


	SDL_Color tcolor = { 255, 255, 255, 255 };


	for (int i = 0; i < 10; i++)
	{
		std::stringstream ss;
		ss << i << " - ";
		ss << std::setfill('0') << std::setw(6) << high_score[i];
		std::string s = ss.str();

		int w, h;
		TTF_SizeText(font, s.c_str(), &w, &h);
		SDL_Surface* tsurface = TTF_RenderText_Solid(font, s.c_str(), tcolor);
		SDL_Texture* ttexture = SDL_CreateTextureFromSurface(renderer, tsurface);

		w = (22 * w) / h;
		h = 22;

		SDL_Rect drect{
			(SCREEN_WIDTH - w) / 2,
			(150 + i * (15 + 22)),
			w,
			h
		};

		SDL_RenderCopy(renderer, ttexture, NULL, &drect);
		SDL_FreeSurface(tsurface);
		SDL_DestroyTexture(ttexture);
	}
	SDL_RenderPresent(renderer);
	SDL_Event e;
	bool keypressed = false;
	while (!keypressed)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				return;
			}
			if (e.type == SDL_KEYDOWN)
			{
				keypressed = true;
			}
		}
		SDL_Delay(1000 / 60);
	}
	Menu();
}

void RenderMenu()
{
	SDL_RenderSetViewport(renderer, NULL);
	SDL_RenderCopy(renderer, menu.tex[menu.state].texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void SaveScore()
{
	int s = gState.score;
	for (int i = 0; i < 10; i++)
		if (s > high_score[i])
		{
			int m = high_score[i];
			high_score[i] = s;
			s = m;
		}

	std::ofstream os("data.bin", std::ios::out | std::ios::binary);
	for (int i = 0; i < 10; i++)
	{
		os.write((char*)&high_score[i], sizeof(int));
	}
	os.close();
}

int RDice(int n, int dice[])
{
	int s = 0;
	for (int i = 0; i < n; i++) s += dice[i];
	int c = 1 + rand() % s;
	for (int i = 0; i < n; i++)
	{
		if (c - dice[i] <= 0) return i;
		else c -= dice[i];
	}
	return 0;
}

int CreateBrick()
{
	int n;
	{
		int dice[] = { 18 ,4, 3, 2 };
		n = RDice(4, dice);
	}
	int  count = 0;
	while (count < n)
	{

		int xy[2] = { 0, 0 };
		for (int i = 0; i < 2; i++)
		{
			int dice[] = { 5, 2, 1 , 1, 2, 5 };
			int s[] = { 2, 4, 9, 9, 4, 2 };
			int p = RDice(6, dice);
			for (int t = 0; t < p; t++)
				xy[i] += s[t];
			xy[i] += rand() % s[p];
		}
		if (snake.dir == DOWN || snake.dir == UP)
		{
			if (xy[0] == snake.head_x) continue;
		} 
		else 
		{
			if (xy[1] == snake.head_y) continue;
		}
		gameboard.type[idx(xy[0], xy[1])] = BRICK_BLOCK;
		count++;
	}
	return count;
}

void EatFood()
{
	gState.score += gState.speed * (snake.length + gState.brick_count) / GAME_SPEED;
	gState.speed++;
	snake.length;
	gState.brick_count += CreateBrick();
	CreateFood();
}

Timer::Timer()
{
	Reset();
}
void Timer::Reset()
{
	start = SDL_GetTicks();
	current = start;
	next_update = start + 1000 / GAME_SPEED;
	next_render = start + 1000 / FRAME_RATE;
}
Uint32 Timer::GetTicks()
{
	current = SDL_GetTicks();
	return current;
}

Gamestate::Gamestate()
{
	Reset();
}
void Gamestate::Reset()
{
	speed = GAME_SPEED;
	score = 0;
	timer.Reset();
	tTime = SDL_GetTicks();
	brick_count = 0;
}
double Gamestate::Time()
{
	Uint32 time = (timer.current - timer.start) / 100;
	return time / (double)10;
}

Gameboard::Gameboard()
{
	SDL_Rect* ptr;
	for (int y = 0; y < GAME_HEIGHGT; y++)
		for (int x = 0; x < GAME_WIDTH; x++)
		{
			ptr = &rect[idx(x, y)];
			ptr->x = x * BLOCK_SIZE;
			ptr->y = y * BLOCK_SIZE;
			ptr->w = BLOCK_SIZE;
			ptr->h = BLOCK_SIZE;
		}
}

void GameTexture::free()
{
	if (surface != NULL)
	{
		SDL_FreeSurface(surface);
		surface = NULL;
	}
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
	if (rect != NULL) delete rect;
	rect = NULL;
}
GameTexture::~GameTexture()
{
	free();
}