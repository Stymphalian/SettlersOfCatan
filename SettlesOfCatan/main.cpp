#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "Game.h"
#include "Logger.h"
#include "Graph.h"

bool init();
void shutdown();

int main(int argc, char** argv){
	if(init() == false){ return EXIT_FAILURE; }	
	Game game;
	game.run();
	shutdown();
	return 0;
}

bool init(){
	Logger& logger = Logger::getLog("jordan.log");
	logger.set_level(Logger::DEBUG);
	logger.set_deepest_level_allowed(Logger::DEBUG);	
	logger.log(Logger::DEBUG, "SDL init");
	
	// initialize SDL for using modules
	if(SDL_Init(SDL_INIT_EVERYTHING)){
		logger.SDL_log(Logger::ERROR, "SDL_INIT_EVERYTHING");
		return false;
	}
	// initialize PNG SDL_Image
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
		logger.IMG_log(Logger::ERROR, "IMG_INIT(IMG_INIT_PNG)");
		return false;
	}
	// initialize JPG SDL_Image
	if((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG){
		logger.IMG_log(Logger::ERROR, "IMG_INIT(IMG_INIT_JPG)");
		return false;
	}
	// Initialize the TTF font module
	if(TTF_Init() != 0){
		logger.TTF_log(Logger::ERROR, "TTF_Init");
		return false;
	}
	// Initialize the Mixer for MP3 usage
	int flags = MIX_INIT_MP3;
	if((Mix_Init(flags) & flags) != flags){
		logger.Mix_log(Logger::ERROR, "MIX_INIT_MP3");
		return false;
	}
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
		logger.Mix_log(Logger::ERROR, "Mix_OpenAudio");
		return false;
	}
	return true;
}

void shutdown(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "SDL shutdown");
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}