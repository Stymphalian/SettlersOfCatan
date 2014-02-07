
#include <SDL.h>
#include "View_Play.h"
#include "IDialog.h"
#include "Timer.h"
#include "Util.h"
#include "Configuration.h"
#include "Logger.h"
#include "Collision.h"
#include "TextField.h"
#include "DropDown.h"


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

	dropdown.init(50, 0, 0, 25, 1, 9, 10);
	dropdown.append_item("jordan");
	dropdown.append_item("rin");
	dropdown.append_item("jordan2");
	dropdown.append_item("rin2");
	dropdown.append_item("jordan3");
	dropdown.append_item("rin3");

	dropdown2.init(300, 30, 0, 25, 1, 9, 10);
	dropdown2.append_item("stupid");
	dropdown2.append_item("lovely");
	dropdown2.append_item("ugly");
	dropdown2.append_item("beautiful");
	dropdown2.append_item("hateful");
	dropdown2.append_item("sexy");

	dropdown3.init(0, 250, 0, 25,2, 9, 10);
	dropdown3.append_item("stupid");
	dropdown3.append_item("lovely");
	dropdown3.append_item("ugly");
	dropdown3.append_item("beautiful");
	dropdown3.append_item("hateful");
	dropdown3.append_item("sexy");

	dropdown4.init(200,180, 0, 25, 1, 9, 10);
	dropdown4.append_item("stupid");
	dropdown4.append_item("lovely");
	dropdown4.append_item("ugly");
	dropdown4.append_item("beautiful");
	dropdown4.append_item("hateful");
	dropdown4.append_item("sexy");

	selected_dropdown = nullptr;
	
	dropdown_list.push_back(&dropdown);
	dropdown_list.push_back(&dropdown2);
	dropdown_list.push_back(&dropdown3);
	dropdown_list.push_back(&dropdown4);
	


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
	if(selected_dropdown != nullptr && selected_dropdown->has_focus){
		selected_dropdown->handle_keyboard_events(ev);
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
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	selected_dropdown = nullptr;
	for(int i = 0; i < (int) dropdown_list.size(); ++i){
		dropdown_list[i]->handle_mouse_events(ev, mouse_intersect);
		if(dropdown_list[i]->has_focus){
			selected_dropdown = dropdown_list[i];
		}
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
}

void View_Play::render(){
	SDL_RenderClear(&ren);

	SDL_Rect clip = { 0, 0, disp_w, disp_h };
	for(int i = 0; i < (int)dropdown_list.size(); ++i){
		dropdown_list[i]->render(ren, *font_carbon_12, font_carbon_12_colour,clip);
	}
	
	SDL_RenderPresent(&ren);
}

