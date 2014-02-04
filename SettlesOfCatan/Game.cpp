#include "Game.h"
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "Configuration.h"
#include "Util.h"
#include "Logger.h"
#include "IView.h"
#include "View_Game.h"
#include "View_Play.h"
#include "Model.h"

/*
start_screen
start
game_screen_page
options
exit

game_screen_page
num_of_player
player_name				colour
.
.
player_name				colour

back						start

options
resolution [widthxheight]
sound fx volume
music volume
back					save
*/

Game::Game(){
	construct("Settlers of Catan",0,0,640,640);
}
Game::Game(const char* title,int x, int y,int w, int h){
	construct(title,x,y,w, h);	
}
Game::~Game(){
	Logger::getLog().log(Logger::DEBUG, "Game destructor");
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
}

void Game::construct(const char* title,int x, int y,int w, int h){
	Logger& logger = Logger::getLog();
	start_load_time = SDL_GetTicks();
	active = true;
	window_title = title;
	default_window_x = x;
	default_window_y = y;
	default_window_w = w;
	default_window_h = h;		

	// Create a window
	win = SDL_CreateWindow(window_title.c_str(),
									default_window_x, default_window_y,
									default_window_w, default_window_h,
									SDL_WINDOW_SHOWN);
	if(win == nullptr){
		logger.SDL_log(Logger::ERROR, "SDL_CreateWindow");
		active = false;
	}

	// Assign a renderer for the window
	ren = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC);
	if(ren == nullptr) {
		logger.SDL_log(Logger::ERROR, "SDL_CreateRenderer");
		active = false;
	}
	
	// set the current view to nothing
	current_view = nullptr;
}

bool Game::is_keyboard_event(SDL_Event& e){
	return (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP ||
		e.type == SDL_TEXTEDITING || e.type == SDL_TEXTINPUT);
}
bool Game::is_mouse_event(SDL_Event& e){
	return (e.type == SDL_MOUSEBUTTONDOWN ||
		e.type == SDL_MOUSEBUTTONUP ||
		e.type == SDL_MOUSEMOTION);
}

// every game has a window and rendered associated with it.
// the window and renderer are then passed to whatever view we create
void Game::run(){
	Logger& logger = Logger::getLog();
	logger.log(Logger::DEBUG, "Game::run()");
	
	// HACK !^0^!
	Model model(1);
	View_Game game_view(model,*win,*ren);
	if(game_view.active == false){ return; }

	View_Play play_view(*win, *ren);

	current_view = &game_view;
	current_view = &play_view;
	Util::get().push_userev(Util::get().get_userev("view_switch_event"),0,nullptr,nullptr);
	
	// WEIRD
	Uint32 end_load_time = SDL_GetTicks();
	//printf("Game::run() Load time = %5f s", ((float)end_load_time - start_load_time) / 1000);
	Logger::getLog().log(Logger::DEBUG, "Game::run() Load time = %5f s", ((float)end_load_time - start_load_time)/1000);

	// main event loop
	SDL_Event e;
	bool exit_flag = false;	
	for(;;){
		// wait for an event
		while(SDL_PollEvent(&e) && exit_flag == false){
			if(e.type == SDL_QUIT){
				exit_flag = true;
			} else if(is_keyboard_event(e)){
				current_view->handle_keyboard_events(e);
			} else if(is_mouse_event(e)){
				current_view->handle_mouse_events(e);
			} else if(e.type >= SDL_USEREVENT){
				
				if(e.user.type == Util::get().get_userev("view_switch_event"))
				{ // handle pushing on a new view onto the stack
					current_view->on_switch(e);
					current_view->on_start(e);
					// current_view->on_switch(e);					
					// push the current view onto the stack
					// pop off the next view
					// assign the new view to the current view
					// run any start-up events										
				}
				else if(e.user.type == Util::get().get_userev("view_close_event"))
				{  // handle closing the top view
					current_view->on_close(e);
					current_view->on_start(e);
					// close the view
					// delete the view and the model
					// pop off the next view
					// assign the new view to the current view
					// run any start-up events;					
				} 
				else
				{ // Have the current view handle all the other user events
					current_view->handle_user_events(e);
					current_view->update(e); // check for collisions, and update model state
				}
			} else {		
				// any uncaught events
				//logger.log(Logger::DEBUG, "Uncaught event %u", e.type);
			}
		}
		if(exit_flag){ break; }
		if(current_view->draw_flag){
			current_view->draw_flag = false;
			current_view->render();
		}
	}	
}