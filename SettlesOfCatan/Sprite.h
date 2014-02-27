#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "Coords.h"
#include "Collision.h"

//----------------------------------------------------
// I S P R I T E
//----------------------------------------------------
class ISprite{
public:
	virtual ~ISprite(){}
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	virtual bool isvisible()=0;
	virtual void setvisible(bool value)=0;
	virtual Coords& coord()=0;
	virtual Collision& hitbox()=0;
};


//----------------------------------------------------
// S P R I T E
//----------------------------------------------------
class Sprite : public ISprite{
public:
	// constructor and destructor
	Sprite();
	virtual ~Sprite();
	// methods
	virtual void tick();
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) ;
	virtual bool isvisible();
	virtual void setvisible(bool value);
	virtual Coords& coord();
	virtual Collision& hitbox();
protected:
	Coords _coord;
	Collision _hitbox;
	bool _visible_flag;
};


//----------------------------------------------------
// N U L L _ S P R I T E
//----------------------------------------------------
class NULL_Sprite : public ISprite{
public:
	// constructor and destructor
	static NULL_Sprite& get(){
		static NULL_Sprite instance;
		return instance;
	}
	static bool isDummy(void* object){
		return (object == &NULL_Sprite::get());
	}	
	// methods
	virtual void tick(){}
	virtual void update(SDL_Event& ev){}
	virtual void render(SDL_Renderer& ren){}
	virtual void render(SDL_Renderer& ren,int x, int y, SDL_Rect* extent ){}
	virtual bool isvisible() { return false; }
	virtual void setvisible(bool value){}
	virtual Coords& coord(){ return _coord; }
	virtual Collision& hitbox() { return _hitbox; }
protected:
	Coords _coord;
	Collision _hitbox;	
private:
	NULL_Sprite(){}
	virtual ~NULL_Sprite(){}
	NULL_Sprite(const NULL_Sprite&);
	void operator= (NULL_Sprite const&);
};