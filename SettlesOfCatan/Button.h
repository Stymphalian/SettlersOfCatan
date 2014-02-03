#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "Sprite.h"
#include "Collision.h"
class Model;
class View_Game;

class Button {
	enum frames {MOUSEOUT,PRESSED, NOTPRESSED, MOUSEOVER, num_frames };
	
public:
	// variables
	std::string text;
	Collision hitbox;
	int x, y,z, w, h;
	int pad_x, pad_y;
	bool hit_flag;
	
	// constructor and destructor
	Button();
	Button(const char* text, int x, int y, int z, int w, int h);
	virtual ~Button(); 
	void init(const char* text, int x, int y,int z, int w, int h);
	void set_pad(int padx, int pady);
private:
};