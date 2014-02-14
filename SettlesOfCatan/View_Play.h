#pragma once
#include <algorithm>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "Util.h"
#include "Timer.h"
#include "IView.h"
#include "IDialog.h"
#include "CheckBox.h"
#include "View_Game.h"
#include "TextField.h"
#include "DropDown.h"

#include "mouse.h"
#include "ObserverPattern.h"

#include "Coords.h"
#include "Collision.h"
#include "Sprite.h"
#include "Pane.h"



class Page : public Pane{
public:
	Page();
	virtual ~Page();
	bool selected;
	
	virtual bool handle_keyboard_events(SDL_Event& ev);
	virtual bool handle_mouse_events(SDL_Event& ev);
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
};

class Book : public Pane {
public:
	Book();
	virtual ~Book();
	bool selected;

	virtual bool handle_keyboard_events(SDL_Event& ev);
	virtual bool handle_mouse_events(SDL_Event& ev);
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);

	int num_panes;
	Page pages[5];
	Page* selected_page;
};


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
	int board_size;
	int* _board;
	int graph_size;
	int* graph;
	int target;
	int target_x;
	int target_y;
	int offsets[8][2];

	Mouse mouse;
	Book book;

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

	void solve();
	int bvalue(int* board, int n, int col, int row);
	bool _solve(int* board, int n, int col, int row,int* count);
	void graph_that_shit();
	void render_graph();
	void render_panes();
};

