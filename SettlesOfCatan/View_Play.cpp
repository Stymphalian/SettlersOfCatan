
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
	field.init(0, 0, 0, 20,2,9,10);

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
	if(field.has_focus){
		field.handle_keyboard_events(ev);
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
			field.set_num_columns(field.cols + dir);
		}
		else if(keyboard[SDL_SCANCODE_2]){
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1:1;
			field.set_num_rows(field.rows + dir);
		}
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	field.handle_mouse_events(ev);

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
	field.tick();
}

void View_Play::render(){
	SDL_RenderClear(&ren);
	//Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour, "ticks = %5f      amount = %d", (float)SDL_GetTicks()/1000,amount);

	SDL_Rect rect = {
		field.x,field.y,
		field.w,field.h
	};
	SDL_Color colour = { 255,0,0,150 };
	Util::render_rectangle(&ren, &rect, colour);
	if(field.text.length() != 0){
		int left_pos = field.right_pos - field.cols;
		if(left_pos < 0) { left_pos = 0; }

		std::string temp_str = field.text.substr(left_pos, field.cols);
		Util::render_text(&ren, font_carbon_12, rect.x + 5, field.y + field.h - 5 -10, font_carbon_12_colour, "%s", temp_str.c_str());
	}
	if(field.cursor_blink){
		int cursor_pos = field.right_pos;
		if(cursor_pos >= field.cols){ cursor_pos = field.cols; }

		int x_pos = cursor_pos * field.char_w + field.x + 4;
		Util::render_line(&ren, colour,
			x_pos, field.y + 5, x_pos, field.y + field.h - 5);


		cursor_pos = field.left_pos;
		if(cursor_pos >= field.cols){ cursor_pos = field.cols; }
		x_pos = cursor_pos*field.char_w + field.x + 4;
		SDL_Color blue = { 0, 0, 255, 150 };
		Util::render_line(&ren,blue,
			x_pos, field.y + 7, x_pos, field.y + field.h - 7);
	}

	Util::render_text(&ren, font_carbon_12, 0, rect.h + 5, font_carbon_12_colour,
		"has_focus=%d left_pos=%d right_pos=%d text_range=%d, r=%d,c=%d",
		field.has_focus, field.left_pos, field.right_pos, field._text_range,field.rows,field.cols);
	Util::render_text(&ren, font_carbon_12, 0, rect.h + 25, font_carbon_12_colour,
		"cursor_blink=%d tick=%d interval=%d",
		field.cursor_blink, field._tick, field._tick_interval);
	
	SDL_RenderPresent(&ren);
}