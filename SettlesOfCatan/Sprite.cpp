#include "Logger.h"
#include "Util.h"
#include <SDL.h>
#include "Sprite.h"

Sprite::Sprite(int x, int y,SDL_Texture& tex, SDL_Rect* clips,int num_frames)
: spritesheet(tex)
{
	this->x = x;
	this->y = y;
	this->num_frames = num_frames;
	this->clips = clips;
}

Sprite::~Sprite(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Sprite destructor");
}