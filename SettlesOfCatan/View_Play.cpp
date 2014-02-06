
#include <SDL.h>
#include "View_Play.h"
#include "IDialog.h"
#include "Timer.h"
#include "Util.h"
#include "Configuration.h"
#include "Logger.h"
#include "Collision.h"
#include "TextField.h"


View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren): IView(win,ren){
	srand((unsigned)(time(NULL)));
	disp_w = Configuration::DISP_W;
	disp_h = Configuration::DISP_H;
	fps_timer = TimerFactory::get().make(1000 /Configuration::FPS);
	fps_timer->stop();
	button = 0;
	mouse_x = 0;
	mouse_y = 0;

	mouse_intersect.hook(&mouse_x, &mouse_y);
	mouse_intersect.add_rect(0, 0, 0, 1, 1);
	// MY STUFF START
	field2.init(0, 0, 0, 20, 2, 9, 10);
	field3.init(0, field2.y + field2.h + 10, 0, 20, 2, 9, 10);
	field2.set_max_num_chars(2*field2.cols);
	field3.set_max_num_chars(2*field3.cols);
	

	// MY STUFF END
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(), 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
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
	if(selected_textfield != nullptr && selected_textfield->has_focus){
		selected_textfield->handle_keyboard_events(ev);
		return;
	}
	
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
			int dir = keyboard[SDL_SCANCODE_LSHIFT] ? -1:1;
			field2.set_num_columns(field2.cols + dir);
		}
		else if(keyboard[SDL_SCANCODE_2]){
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1:1;
			field2.set_num_rows(field2.rows + dir);
		}
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	field2.handle_mouse_events(ev,mouse_intersect);
	field3.handle_mouse_events(ev,mouse_intersect);
	if(field2.has_focus){
		selected_textfield = &field2;
	} else if(field3.has_focus){
		selected_textfield = &field3;
	} else{
		selected_textfield = nullptr;
	}
	

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
	if(selected_textfield != nullptr){
		selected_textfield->tick();
	}
}

void View_Play::render(){
	SDL_RenderClear(&ren);

	SDL_Rect clip = { 0, 0, disp_w, disp_h };
	field2.render(ren, *font_carbon_12, font_carbon_12_colour, clip);
	field3.render(ren, *font_carbon_12, font_carbon_12_colour, clip);
	
	SDL_RenderPresent(&ren);
}

