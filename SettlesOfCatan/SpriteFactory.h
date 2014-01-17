#pragma once
#include <string>
#include "Sprite.h"

class SpriteFactory{
public:
	// factory
	static SpriteFactory& get(SDL_Renderer* ren);
	Sprite* make(std::string sprite);
	// methods
private:
	// variables
	SDL_Renderer* render;

	// constructor and destructors
	SpriteFactory() {};
	SpriteFactory(SDL_Renderer* ren);
	SpriteFactory(const SpriteFactory&);
	virtual ~SpriteFactory();
	void operator= (SpriteFactory const&);
};
