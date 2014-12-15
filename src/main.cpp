#ifdef _WIN32
	#include <stdio.h>
	#include <stdlib.h>
	#include <SDL.h>
	#include <SDL_image.h>
	#include "Vector2D.h"
#elif linux
	#include <stdio.h>
	#include <stdlib.h>
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_ttf.h>
	#include <SDL2/SDL_mixer.h>
	#include "Vector2D.h"
#endif

using namespace std;

const int SIZE = 11;
const int SCREEN_WIDTH = SIZE*60;
const int SCREEN_HEIGHT = SIZE*30;
const int TAXA_CRESCIMENTO = 4;
const int AREA = SCREEN_WIDTH*SCREEN_HEIGHT;
const int FPS = 20;
const int FRAME_PERIOD = 1000.0f / FPS;

typedef struct{

	SDL_Texture *segmentTexture;
	SDL_Surface *segmentImage;
	SDL_Rect segmento[AREA];
	SDL_Rect sprite[2];
	int frame;
	
	char dir;
	int totalSegmento;
	int dimensao;

} Snake;

typedef struct{

	SDL_Texture *foodTexture;
	SDL_Surface *foodImage;
	SDL_Rect foodPos;

} Food;

bool InitWindow();
void Close();
void InitSnake();
void NewRound();

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *background = NULL;
SDL_Texture *pause = NULL;
SDL_Surface *backgroundImage = NULL;
SDL_Surface *pauseImage = NULL;

SDL_Event event;
SDL_Scancode keypressed = SDL_SCANCODE_UNKNOWN;
bool pauseGame;

const Uint8 *currentKeyStates;

Snake snake;
Food food;

bool InitWindow(){

	int imgFlag = IMG_INIT_PNG;

	IMG_Init(imgFlag);

	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("ERRO\n");
		return false;
	}
	else{

		window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (window == NULL){
			printf("ERRO\n");
			return false;
		}
		else{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		}

	}

	if ((backgroundImage = SDL_LoadBMP("files/bg.bmp")) == NULL) printf("ERRO %s", SDL_GetError());
	background = SDL_CreateTextureFromSurface(renderer, backgroundImage);

	if ((snake.segmentImage = SDL_LoadBMP("files/segment.bmp")) == NULL) printf("ERRO %s", SDL_GetError());
	snake.segmentTexture = SDL_CreateTextureFromSurface(renderer, snake.segmentImage);

	if ((food.foodImage = SDL_LoadBMP("files/food.bmp")) == NULL) printf("ERRO %s", SDL_GetError());
	food.foodTexture = SDL_CreateTextureFromSurface(renderer, food.foodImage);

	if((pauseImage = IMG_Load("files/pause.png")) == NULL) printf("ERRO %s", IMG_GetError());
	pause = SDL_CreateTextureFromSurface(renderer, pauseImage);

	return true;
}

void InitSprite(){

	snake.sprite[0].x = snake.sprite[0].y = 0;
	snake.sprite[0].h = snake.sprite[0].w = SIZE;

	snake.sprite[1].x = SIZE;
	snake.sprite[1].y = 0;
	snake.sprite[1].w = snake.sprite[1].h = SIZE;

}

bool Pressed(SDL_Scancode key){

	return (keypressed == key);

}

bool GetKeys(){

	while (SDL_PollEvent(&event) != 0){

		if (event.type == SDL_QUIT){
			return 1;
		}
		if (event.type == SDL_KEYDOWN){

			keypressed = event.key.keysym.scancode;

			switch (keypressed){
				case SDL_SCANCODE_P: pauseGame = !pauseGame; break;
				case SDL_SCANCODE_ESCAPE: return 1;
			}
		}
	}

	if (!pauseGame){

		if (Pressed(SDL_SCANCODE_UP) || Pressed(SDL_SCANCODE_W)){
			if (snake.dir != 'D')
				snake.dir = 'U';
		}
		if (Pressed(SDL_SCANCODE_DOWN) || Pressed(SDL_SCANCODE_S)){
			if (snake.dir != 'U')
				snake.dir = 'D';
		}
		if (Pressed(SDL_SCANCODE_LEFT) || Pressed(SDL_SCANCODE_A)){
			if (snake.dir != 'R')
				snake.dir = 'L';
		}
		if (Pressed(SDL_SCANCODE_RIGHT) || Pressed(SDL_SCANCODE_D)){
			if (snake.dir != 'L')
				snake.dir = 'R'; 
		}
		if (Pressed(SDL_SCANCODE_R)){
			
		}

	}

	return 0;
}

bool BodyCollision(int x, int y){

	for (int i = 0; i < snake.totalSegmento; i++)
		if ((snake.segmento[i].x == x) && (snake.segmento[i].y == y))
			return true;

	return false;
}

bool CheckCollision(int x, int y){

	if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT && !BodyCollision(x, y))
		return false;

	return true;
}

SDL_Rect CreateFood(){

	SDL_Rect *posLivre = new SDL_Rect[AREA];
	int posLivreCont = 0;
	int contX, contY;
	int posSorteada = 0;

	for (contX = 0; contX < SCREEN_WIDTH; contX += SIZE){
		for (contY = 0; contY < SCREEN_HEIGHT; contY += SIZE){
			if (!BodyCollision(contX, contY)){
				posLivreCont++;
				posLivre[posLivreCont].x = contX;
				posLivre[posLivreCont].y = contY;
			}
		}
	}

	srand(SDL_GetTicks());

	posSorteada = rand() % posLivreCont + 1;

	posLivre[posSorteada].w = posLivre[posSorteada].h = SIZE;

	return posLivre[posSorteada];
}

void PrintSnake(){

	SDL_RenderCopy(renderer, snake.segmentTexture, &snake.sprite[0], &snake.segmento[0]);

	for (int i = snake.totalSegmento; i > 0; i--){
		SDL_RenderCopy(renderer, snake.segmentTexture, &snake.sprite[0], &snake.segmento[i]);
	}

}

void PauseScreen(){

	SDL_Rect pauseRect = { SCREEN_WIDTH/3, SCREEN_HEIGHT/3, 188, 48 };

	PrintSnake();

	SDL_RenderCopy(renderer, pause, NULL, &pauseRect);

}

bool MoveSnake(){

	SDL_Rect newpos;
	
	newpos = snake.segmento[0];

	switch (snake.dir){
		case 'R': newpos.x += SIZE; break;
		case 'L': newpos.x -= SIZE; break;
		case 'U': newpos.y -= SIZE; break;
		case 'D': newpos.y += SIZE; break;
	}

	if (!CheckCollision(newpos.x, newpos.y)){
		if (snake.totalSegmento < snake.dimensao){
			snake.totalSegmento++;
		}
			
		snake.segmento[0] = newpos;

		for (int i = snake.totalSegmento; i > 0; i--){
			snake.segmento[i] = snake.segmento[i - 1];
		}

		if ((snake.segmento[0].x == food.foodPos.x) && (snake.segmento[0].y == food.foodPos.y) && (snake.totalSegmento != AREA)){
			food.foodPos = CreateFood();
			snake.dimensao += TAXA_CRESCIMENTO;
		}

	}
	else{
		
	}

	return true;
}

void Close(){

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_DestroyTexture(background);
	SDL_FreeSurface(backgroundImage);

	SDL_DestroyTexture(pause);
	SDL_FreeSurface(pauseImage);

	SDL_DestroyTexture(snake.segmentTexture);
	SDL_FreeSurface(snake.segmentImage);

	SDL_DestroyTexture(food.foodTexture);
	SDL_FreeSurface(food.foodImage);

	SDL_DestroyRenderer(renderer);

	IMG_Quit();
	SDL_Quit();

}

void InitSnake(){

	snake.frame = 0;
	snake.segmento[0] = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SIZE, SIZE };
	snake.dir = 'R';
	snake.dimensao = TAXA_CRESCIMENTO;
	snake.totalSegmento = 1;

}

void InitFood(){

	food.foodPos = CreateFood();
	food.foodPos.h = food.foodPos.w = SIZE;

}

void NewRound(){

	InitSprite();
	InitSnake();
	InitFood();
	pauseGame = false;

}

void GameLoop(bool &quit){

	quit = GetKeys();

	SDL_RenderClear(renderer);

	SDL_RenderCopy(renderer, background, NULL, NULL);

	if (!pauseGame)
		MoveSnake();
	else
		PauseScreen();

	PrintSnake();

	SDL_RenderCopy(renderer, food.foodTexture, NULL, &food.foodPos);

	SDL_RenderPresent(renderer);


}

int main(int argc, char *argv[]){

	bool quit = false;
	Uint32 frameStart, frameTime;

	InitWindow();

	NewRound();

	while (!quit){

		frameStart = SDL_GetTicks();

		GameLoop(quit);

		frameTime = SDL_GetTicks() - frameStart;

		if (frameTime < FRAME_PERIOD){
			SDL_Delay((int)(FRAME_PERIOD - frameTime));
		}

	}


	Close();
	return 0;
}
