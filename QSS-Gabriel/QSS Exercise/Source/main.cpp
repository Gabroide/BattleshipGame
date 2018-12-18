// ----------------------------------------------------------------
// LICENSE
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non - commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain.We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors.We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
// ----------------------------------------------------------------

#include<vector>
#include "SDL\include\SDL.h"
#include "SDL_image\include\SDL_image.h"
#include "SDL_mixer\include\SDL_mixer.h"

/* ----------------------------------------------------------------
SDL_Init(SDL_INIT_EVERYTHING); // init all SDl subsystems
SDL_CreateWindow(); // creates a window
SDL_CreateRenderer(); // Creates a renderer inside a window, use SDL_RENDERER_PRESENTVSYNC
SDL_DestroyRenderer();
SDL_DestroyWindow();
SDL_Quit();

IMG_Init(IMG_INIT_PNG); // we only need png
IMG_Quit();
SDL_CreateTextureFromSurface(renderer, IMG_Load("ship.png")); // returns a pointer to the texture we can later draw
SDL_DestroyTexture(tex);
SDL_QueryTexture(tex, nullptr, nullptr, &w, &h); // query width and height of a loaded texture
SDL_RenderCopy(renderer, tex, &section, &destination); // section of the texture and destination on the screen
SDL_RenderPresent(renderer); // swap buffers, stall if using vsync

Mix_Init(MIX_INIT_OGG); // we only need ogg
Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // standard values
music = Mix_LoadMUS("music.ogg"); // returns pointer to music
Mix_PlayMusic(music, -1); // play music
fx = Mix_LoadWAV("laser.wav"); // load short audio fx
Mix_PlayChannel(-1, fx, 0); // play audio fx
SDL_GetTicks(); // for timers, returns ms since app start

Mix_FreeMusic(music);
Mix_FreeChunk(fx_shoot);
Mix_CloseAudio();
Mix_Quit();

SDL_PollEvent(&event); // query all input events, google possible contents of "event"
SDL_HasIntersection(&react_a, &react_b); // checks for quad overlap
*/

/* ------------ SOUND CLASS -------------- */
class Sound
{
public:
	Sound(const char* music_file, const char* fx_file)
	{
		Mix_Init(MIX_INIT_OGG); // we only need ogg
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // standard values
		music = Mix_LoadMUS(music_file); // returns pointer to music
		fx_shoot = Mix_LoadWAV(fx_file); // load short audio fx
	}

	~Sound()
	{
		Mix_FreeMusic(music);
		Mix_FreeChunk(fx_shoot);
		Mix_CloseAudio();
		Mix_Quit();
	}

	void PlayMusic() { 	Mix_PlayMusic(music, -1); }
	void PlayFX() {	Mix_PlayChannel(-1, fx_shoot, 0); }

	Mix_Music* music;
	Mix_Chunk* fx_shoot;
};

class Timer
{
public:
	Timer()
	{
		start = SDL_GetTicks();
		last = SDL_GetTicks();
	}

	void Update() 	{ last = SDL_GetTicks(); }
	unsigned int Time() { return SDL_GetTicks() - start; }
	unsigned int DeltaTime() { return SDL_GetTicks() - last; }

	unsigned int start = 0U;
	unsigned int last = 0U;
};

// ----------------------------------------------------------------

void Blit(SDL_Texture* texture, int x, int y, SDL_Rect* section, SDL_Renderer* renderer)
{
	bool ret = true;
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	SDL_RenderCopy(renderer, texture, section, &rect);
}

SDL_Texture* Load(const char* path, SDL_Renderer* renderer)
{
	return SDL_CreateTextureFromSurface(renderer, IMG_Load(path)); // returns a pointer to the texture we can later draw
}

#define SPEED 10
#define SHOTSPEED 20

struct shotStruct {
	int x;
	int y;
};
std::vector<shotStruct> shots;

struct enemyStruct {
	float x;
	float y;
	int iniTicks;
};
std::vector<enemyStruct> enemies;

int main(int argc, char* args[])
{
	// Init
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG); // we only need png
	int width = 640;
	int height = 490;


	SDL_Window* window =  SDL_CreateWindow("game",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,width,height,SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	Sound* sound = new Sound("assets/music.ogg", "assets/laser.wav");
	sound->PlayMusic();
	sound->PlayFX();

	Timer* timer = new Timer();

	SDL_Rect background;
	background.x = 0;
	background.y = 0;
	background.w = width;
	background.h = height;

	SDL_Texture* backgroundTexture = Load("assets/bg0.png", renderer);

	SDL_Rect far_mountain;
	far_mountain.x = 0;
	far_mountain.y = 0;
	far_mountain.w = 640;
	far_mountain.h = 200;

	SDL_Rect shipRect;
	shipRect.x = 0;
	shipRect.y = 0;
	shipRect.w = 32;
	shipRect.h = 32;

	SDL_Rect shotRect;
	shotRect.x = 0;
	shotRect.y = 0;
	shotRect.w = 32;
	shotRect.h = 32;

	SDL_Texture* far_mountainTexture = Load("assets/bg1.png", renderer);
	SDL_Texture* close_mountainTexture = Load("assets/bg2.png", renderer);
	SDL_Texture* treesTexture = Load("assets/bg3.png", renderer);
	SDL_Texture* ship = Load("assets/ship.png", renderer);
	SDL_Texture* shot = Load("assets/shot.png",renderer);
	SDL_Texture* enemy = Load("assets/enemy.png", renderer);

	int lastEnemiesSpawned = 0;
	float speed = 1.0f;
	float bg1X = 0.f, bg2X = 0.f, bg3X = 0.f;
	int shipX = 100, shipY = 100;
		
		// Loop
	while (true)
	{

		timer->Update();
		//background
		Blit(backgroundTexture, 0, 0, &background, renderer);

		Blit(far_mountainTexture, bg1X , 300, &far_mountain, renderer);
		Blit(far_mountainTexture, bg1X + far_mountain.w, 300, &far_mountain, renderer);
		if (bg1X < -640)
			bg1X = 0.f;
		
		bg1X -= 0.1f;

		Blit(close_mountainTexture, bg2X, 300, &far_mountain, renderer);
		Blit(close_mountainTexture, bg2X + far_mountain.w, 300, &far_mountain, renderer);
		if (bg2X < -640)
			bg2X = 0.f;
		bg2X -= 0.8f;
		
		Blit(treesTexture, bg3X, 340, &far_mountain, renderer);
		Blit(treesTexture, bg3X + far_mountain.w, 340, &far_mountain, renderer);
		if (bg3X < -640)
			bg3X = 0.f;
		bg3X -= 2.1f;
		
		//ship
		SDL_Event event;
		SDL_PollEvent(&event);
		Blit(ship, shipX, shipY, &shipRect, renderer);
		if (shipY < 32)
			shipY = 32;
		if (shipY > 455)
			shipY = 455;

		if (shipX < 32)
			shipX = 32;
		if (shipX > 600)
			shipX = 600;

		static int dirY = 0;
		static int dirX = 0;

	
		if (event.key.type == SDL_KEYDOWN) 
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_W)
			{
				dirY = -1;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S)
			{
				dirY = 1;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_A)
			{
				dirX  = -1;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_D)
			{
				dirX = 1;
			}
			
		}
		if (event.key.type == SDL_KEYUP)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_W)
			{
				dirY = 0;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S)
			{
				dirY = 0;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_A)
			{
				dirX = 0;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_D)
			{
				dirX = 0;
			}	
		}
		if (event.key.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
			{

				shotStruct newShot;
				newShot.x = shipX;
				newShot.y = shipY;
				shots.push_back(newShot);
			}
		}
		shipY += SPEED * dirY;
		shipX += SPEED * dirX;


		// Add enemies
		if (SDL_GetTicks() - lastEnemiesSpawned > 3 * 1000) {
			lastEnemiesSpawned = SDL_GetTicks();

			//if array of enemies is empty, program explodes
			for (int i = 0; i < 5; ++i) {
				enemyStruct newEnemy;
				newEnemy.x = width + i * 35;
				newEnemy.y = height / 2;
				newEnemy.iniTicks = SDL_GetTicks();
				enemies.push_back(newEnemy);
			}

		}

		//Update Shots
		int shotsSpeed = 5;
		for (int i = 0; i < shots.size(); ++i) {
			shots[i].x += shotsSpeed;

			if (shots[i].x > width) shots.erase(shots.begin() + i);
		}
		//Update enemies
		float enemySpeed = 3;
		for (int i = 0; i < enemies.size(); ++i) {
			enemies[i].x -= enemySpeed;
			enemies[i].y += sin(enemies[i].x/10)*10;

			//check collision of projectiles and enemies
			for (int j = 0; j < shots.size(); ++j) {
				if (shots[j].x <= enemies[i].x + 15 && shots[j].x >= enemies[i].x - 15 &&
					shots[j].y <= enemies[i].y + 15 && shots[j].y >= enemies[i].y - 15) {

					shots.erase(shots.begin() + j);
					enemies.erase(enemies.begin() + i);
					
				}
			}
		}



		// Draw Shots
		for (int i = 0; i < shots.size(); ++i) {
			Blit(shot, shots[i].x, shots[i].y, &shipRect, renderer);
		}

		// Draw enemies
		for (int i = 0; i < enemies.size(); ++i) {
			Blit(enemy, enemies[i].x, enemies[i].y, &shipRect, renderer);
		}

		SDL_RenderPresent(renderer); // swap buffers, stall if using vsync
	}


	// CleanUp
	delete sound;
	SDL_DestroyTexture(backgroundTexture);
	SDL_DestroyTexture(far_mountainTexture);
	SDL_DestroyTexture(close_mountainTexture);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();

	return(0); // EXIT_SUCCESS
}