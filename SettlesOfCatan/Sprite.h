#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "Coords.h"
#include "Collision.h"

class ISprite{
public:
	Coords coord;
	Collision hitbox;

	virtual ~ISprite(){}
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
};

class Sprite : public ISprite
{
public:
	//variables	
	//int num_frames;
	//int current_frame;
	//SDL_Rect* clips;
		
	// constructor and destructor
	Sprite();
	virtual ~Sprite();

	// methods
	virtual void tick(){}
	virtual void update(SDL_Event* ev){}
	virtual void render(SDL_Renderer* ren){}
private:
};