#pragma once
#include <SDL.h>

/*
	Abstract interface for a View.
*/
class IView{
public:
	//variables
	SDL_Window& win;
	SDL_Renderer& ren;
	bool draw_flag;

	IView(SDL_Window& win, SDL_Renderer& ren);
	virtual ~IView(){};
	virtual void on_start(SDL_Event& ev) = 0; // what you call when you start the view
	virtual void on_switch(SDL_Event& ev) = 0; // what you call when you are switching views, but not closing
	virtual void on_close(SDL_Event& ev) = 0;
	virtual void handle_keyboard_events(SDL_Event& ev) = 0;
	virtual void handle_mouse_events(SDL_Event& ev) = 0;
	virtual void handle_user_events(SDL_Event& ev) = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render() = 0;
};

/* 
Concrete class for the Start Screen
*/
class View_StartScreen : public IView{
public:
	View_StartScreen(SDL_Window& win, SDL_Renderer& ren);
	virtual ~View_StartScreen();
	void on_start(SDL_Event& ev){};
	void on_switch(SDL_Event& ev) {};
	void on_close(SDL_Event& ev) {};
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_evnets(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
private:
};

/*
Concrete Class for the Settings and Options page
*/
class View_Options: public IView{
public:
	View_Options(SDL_Window& win, SDL_Renderer& ren);
	virtual ~View_Options();
	void on_start(SDL_Event& ev){};
	void on_switch(SDL_Event& ev) {};
	void on_close(SDL_Event& ev) {};
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_evnets(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
private:
};
