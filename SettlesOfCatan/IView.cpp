#include "IView.h"
#include <SDL.h>

IView::IView(SDL_Window& win, SDL_Renderer& ren) :
win(win), ren(ren){
	draw_flag = drag_flag = false;
}

View_StartScreen::View_StartScreen(SDL_Window& win, SDL_Renderer& ren) : IView(win,ren){}
View_StartScreen::~View_StartScreen(){}
void View_StartScreen::handle_keyboard_events(SDL_Event& ev){}
void View_StartScreen::handle_mouse_events(SDL_Event& ev){}
void View_StartScreen::handle_user_evnets(SDL_Event& ev){}
void View_StartScreen::update(SDL_Event& ev){}
void View_StartScreen::render(){}

View_Options::View_Options(SDL_Window& win, SDL_Renderer& ren) : IView(win,ren){}
View_Options::~View_Options(){}
void View_Options::handle_keyboard_events(SDL_Event& ev){}
void View_Options::handle_mouse_events(SDL_Event& ev){}
void View_Options::handle_user_evnets(SDL_Event& ev){}
void View_Options::update(SDL_Event& ev){}
void View_Options::render(){}
