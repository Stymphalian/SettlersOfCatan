
#include <SDL.h>
#include "View_Play.h"
#include "IDialog.h"
#include "Timer.h"
#include "Util.h"
#include "Logger.h"
#include "Collision.h"


View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren): IView(win,ren){
	srand((unsigned)(time(NULL)));
	disp_w = UTIL_DISP_W;
	disp_h = UTIL_DISP_H;
	fps_timer = TimerFactory::get().make(1000 /UTIL_FPS);
	fps_timer->stop();
	button = 0;
	mouse_x = 0;
	mouse_y = 0;

	mouse_intersect.hook(&mouse_x, &mouse_y);
	mouse_intersect.add_rect(0, 0, 0, 1, 1);
	// MY STUFF STARt
	texture = Util::load_texture("data/hextiles_spritesheet.png", &ren);
	amount = 100000;
	// MY STUFF END
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

		// MY STUFF START
		if(keyboard[SDL_SCANCODE_1]){
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1 : 1;
			amount += dir;
		}
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	button = SDL_GetMouseState(&mouse_x,&mouse_y);
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
		} else if(ev.user.type == Util::get().get_userev("dialog_close_event")){
			
		} else{
		}
	}
}

void View_Play::update(SDL_Event& ev){
	

	// do something for every user event
}

void View_Play::render(){
	SDL_RenderClear(&ren);
	//Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour, "ticks = %5f      amount = %d", (float)SDL_GetTicks()/1000,amount);

	Uint32 start = SDL_GetTicks();
	for(int i = 0; i < amount; ++i){
		Uint8 r, g, b, a;
		SDL_GetTextureColorMod(texture, &r, &g, &b);
		SDL_GetTextureAlphaMod(texture, &a);
		SDL_SetTextureColorMod(texture, 200,200,200);
		SDL_SetTextureAlphaMod(texture, 180);
		SDL_SetTextureColorMod(texture, r,g,b);
		SDL_SetTextureAlphaMod(texture, a);
	}
	Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour, "ticks = %5f      amount = %d", (float)(SDL_GetTicks()-start) / 1000, amount);

	
	SDL_RenderPresent(&ren);
}