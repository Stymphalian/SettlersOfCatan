#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "Timer.h"
#include "IView.h"
#include "IDialog.h"
#include "CheckBox.h"
#include "View_Game.h"

class View_Play : public IView
{
public:
	// variables
	int disp_w;
	int disp_h;
	Timer* fps_timer;
	SDL_Surface* surface;	
	SDL_Texture* texture;
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;

	Collision mouse_intersect;
	Uint32 button;
	int mouse_x, mouse_y;


	// M Y  S T U F F   S T A R T

	// M Y   S T U F F  E N D 

	// constructor and destructor
	View_Play(SDL_Window& win, SDL_Renderer& ren);
	~View_Play();
	// methods
	void on_start(SDL_Event& ev);
	void on_switch(SDL_Event& ev);
	void on_close(SDL_Event& ev);
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_events(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
};

