#include "Logger.h"
#include "Util.h"
#include <SDL.h>
#include "Coords.h"
#include "Collision.h"
#include "Sprite.h"

Sprite::Sprite(): ISprite()
{
	//Logger::getLog().log(Logger::DEBUG, "Sprite constructor");
	coord.init(0, 0, 0, 0, 0);
	hitbox.hook(&coord);	
	hitbox.add_mutable_rect(&coord);
}

Sprite::~Sprite(){
	//Logger::getLog().log(Logger::DEBUG, "Sprite destructor");
	hitbox.clear();
	// clear coordinates?
}