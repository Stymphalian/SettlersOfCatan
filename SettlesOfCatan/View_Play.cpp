
#include <SDL.h>
#include "View_Play.h"
#include "Timer.h"
#include "Util.h"
#include "Logger.h"


View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren): IView(win,ren){
	srand((unsigned)(time(NULL)));
	disp_w = UTIL_DISP_W;
	disp_h = UTIL_DISP_H;
	fps_timer = TimerFactory::get().make(1000 /UTIL_FPS);
	fps_timer->stop();

	font_carbon_12 = TTF_OpenFont("data/carbon.ttf", 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		Logger::getLog("jordan.log").TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
	}
}

View_Play::~View_Play(){
	delete fps_timer;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	TTF_CloseFont(font_carbon_12);
}

void View_Play::on_start(SDL_Event& ev){
	fps_timer->start();	
}

void View_Play::on_switch(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::on_close(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::handle_keyboard_events(SDL_Event& ev){
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			SDL_Event ev;
			SDL_zero(ev);
			ev.type = SDL_QUIT;
			ev.quit.type = SDL_QUIT;
			ev.quit.timestamp = SDL_GetTicks();
			SDL_PushEvent(&ev);
			return;
		}

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	if(ev.type == SDL_MOUSEBUTTONDOWN){

	} else if(ev.type == SDL_MOUSEBUTTONUP){

	} else if(ev.type == SDL_MOUSEMOTION){

	}

}
void View_Play::handle_user_events(SDL_Event& ev){
	if(ev.type >= SDL_USEREVENT){
		// handle a frame per 1/30 the of a second
		if(ev.user.type == TimerFactory::get().event_type()){
			draw_flag = true;
		}
	}
}
void View_Play::update(SDL_Event& ev){
	// do something every for every user event
}
void View_Play::render(){
	SDL_RenderClear(&ren);

	for(int i = 0; i < 100; ++i){
		Util::render_text(&ren, font_carbon_12, 0, i, font_carbon_12_colour,
			"Message %s %d,%d, %s %d %d %d, %d",
			"asldfkj a;lsdkf alkfja s;lfkja d;flkaj df;lakds f;aldk fa;ldskf a",
			12, 12515, "alkds a osjordan ias ", 123, 15, 126, 12319231);
	}

	SDL_RenderPresent(&ren);
}