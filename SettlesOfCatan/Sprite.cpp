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
	_visible_flag = true;
}

Sprite::~Sprite(){
	//Logger::getLog().log(Logger::DEBUG, "Sprite destructor");
	_hitbox.clear();
	// clear coordinates?
}
void Sprite::tick(){}
void Sprite::update(SDL_Event& ev){}
void Sprite::render(SDL_Renderer& ren){
	SDL_Rect extent = {
		0, 0, coord().w(), coord().h()
	};
	render(ren, coord().disp_x(), coord().disp_y(), &extent);
}
void Sprite::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){}
bool Sprite::isvisible(){ return _visible_flag; }
void Sprite::setvisible(bool value){_visible_flag = value;}
Coords& Sprite::coord(){ return _coord; }
Collision& Sprite::hitbox(){ return _hitbox; }
