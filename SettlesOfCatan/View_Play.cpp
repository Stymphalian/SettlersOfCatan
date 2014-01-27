
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
	
	dialog = nullptr;

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
	if(dialog != nullptr && dialog->isvisible()){
		dialog->handle_keyboard_events(ev);
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
			//dialog = new View_Game_Debug_Dialog(*this, 50, 50, 0, 400,300);
			//dialog->open(&debug);
		}
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){

	// the dialog will capture all the events.
	if(dialog != nullptr && dialog->isvisible()){
		dialog->handle_mouse_events(ev);
		return;
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
			IDialog* d =  (IDialog*) ev.user.data1;
			if(d != nullptr){
				//void* data = d->close();
				if(d == dialog){
					delete dialog;
					dialog = nullptr;
				}				
			}
		} else{
		}
	}
}

void View_Play::update(SDL_Event& ev){
	if(dialog != nullptr && dialog->isvisible()){
		dialog->update(ev);
	}

	// do something for every user event
}

void View_Play::render(){

	if(dialog != nullptr  && dialog->isvisible() && dialog->modal){
		dialog->render();
	} else{

		// we only call a clear inside here, because the modal
		// dialog should freeze everything in the background.
		SDL_RenderClear(&ren);
		// M Y   S T U F F    S T A R T 
		SDL_Rect rect = { 0, 0, 400, 400 };
		SDL_Color  colour = {
			rand() % 255,
			rand() % 255,
			rand() % 255,
			255
		};
		Util::render_rectangle(&ren, &rect, colour);
		// M Y   S T U F F   E N D		

		if(dialog != nullptr && dialog->isvisible()){
			dialog->render();
		}
	}

	SDL_RenderPresent(&ren);
}