#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>

class Sprite
{
public:
	//variables
	int x,y;	
	int num_frames;
	int current_frame;
	SDL_Rect* clips;
	SDL_Texture& spritesheet;
	
	// constructor and destructor
	Sprite(int x, int y, SDL_Texture& spritesheet, SDL_Rect* clip, int num_frames = 1);
	virtual ~Sprite();

	// methods
	virtual void tick()=0;
	virtual void update(SDL_Event* ev) = 0;
	virtual void draw(SDL_Renderer* ren) = 0;
private:
};