#pragma once
#include <SDL.h> // SDL_Event

class IKeyboardHandler{
public:
	virtual bool keyboard_keydown(SDL_Event& ev) = 0;
	virtual bool keyboard_keyup(SDL_Event& ev) = 0;
};

class KeyboardHandler: public IKeyboardHandler{
public:
	virtual bool keyboard_keydown(SDL_Event& ev){};
	virtual bool keyboard_keyup(SDL_Event& ev){};
};