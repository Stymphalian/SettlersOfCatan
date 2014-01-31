#include <cstdio>
#include <SDL.h>
#include <SDL_image.h>
#include "SpriteFactory.h"
#include "Sprite.h"
#include "Logger.h"

SpriteFactory& SpriteFactory::get(SDL_Renderer* ren){
	static SpriteFactory instance(ren);
	return instance;
}

Sprite* SpriteFactory::make(std::string sprite){
	if(sprite.compare("ball") == 0){
		return 0;
	} else if(sprite.compare("box") == 0){
		return 0;
	}
	return 0;
}

SpriteFactory::SpriteFactory(SDL_Renderer* ren){
	render = ren;
}
SpriteFactory::~SpriteFactory(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "SpriteFactory destructor");
}



