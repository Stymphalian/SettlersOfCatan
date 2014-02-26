#include "Logger.h"
#include "Util.h"
#include <SDL.h>
#include "Coords.h"
#include "Collision.h"
#include "Sprite.h"

Sprite::Sprite(): ISprite()
{
	//Logger::getLog().log(Logger::DEBUG, "Sprite constructor");
	_coord.init(0, 0, 0, 0, 0);
	_hitbox.hook(&_coord);	
	_hitbox.add_mutable_rect(&_coord);
}

Sprite::~Sprite(){
	//Logger::getLog().log(Logger::DEBUG, "Sprite destructor");
	_hitbox.clear();
	// clear coordinates?
}