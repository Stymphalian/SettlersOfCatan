#pragma once
#include <SDL.h>
#include <vector>
#include <string>
class Model;
class IView;

class Game{
public:
	// variables
	Uint32 start_load_time;
	int active;
	SDL_Window* win;
	SDL_Renderer* ren;
	// constructor and destructor
	Game();
	Game(const char* title,int x, int y,int w, int h);
	virtual ~Game();
	//methods
	void run();	
private:
	// variables
	int default_window_x;
	int default_window_y;
	int default_window_w;
	int default_window_h;
	std::string window_title;
	std::vector<IView*> view_stack;
	IView* current_view;

	// methods
	void construct(const char* title,int x, int y,int w, int h);
	bool is_keyboard_event(SDL_Event& e);
	bool is_mouse_event(SDL_Event& e);
};


