#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "Coords.h"
#include "Collision.h"

class ISprite{
public:
	virtual ~ISprite(){}
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	virtual Coords& coord()=0;
	virtual Collision& hitbox()=0;
};

class Sprite : public ISprite
{
public:
	// constructor and destructor
	Sprite();
	virtual ~Sprite();

	// methods
	virtual void tick(){}
	virtual void update(SDL_Event* ev){}
	virtual void render(SDL_Renderer* ren, Coords* position = nullptr){}
	virtual void render(SDL_Renderer& ren, Coords* position, Coords* extent){}
	virtual Coords& coord(){ return _coord; }
	virtual Collision& hitbox() { return _hitbox; }
protected:
	Coords _coord;
	Collision _hitbox;
private:
};