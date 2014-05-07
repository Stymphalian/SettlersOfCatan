#pragma once
#include <SDL.h> // SDL_Event
class Coords;

class IMouseHandler{
public:
	virtual bool mouse_buttondown(SDL_Event& ev,Coords* ref= nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref= nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev,Coords* ref= nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref= nullptr) = 0;
};
class MouseHandler:  public IMouseHandler{
public:
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref= nullptr) {return false;}
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref= nullptr) 	 {return false;}
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref= nullptr) 	 {return false;}
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref= nullptr)		 {return false;}
};