#include <SDL.h>
#include <SDL_image.h>
#include "Logger.h"
#include "Util.h"
#include "Sprite.h"
#include "Button.h"

#include "Player.h"
#include "Model.h"
#include "View_Game.h"
#include "IDialog.h"

Button::Button(){ hit_flag = false; }
Button::Button(const char* text, int x, int y, int z, int w, int h){
	init(text, x, y, z,w, h);
}
Button::~Button(){
	Logger::getLog().log(Logger::DEBUG, "Button destructor");
	baction = nullptr;
}
void Button::action(View_Game& view,Model& model){
	if(baction != nullptr){
		baction(view,model);
	}
}
void Button::set_action(Button::button_action baction){
	this->baction = baction;
}
void Button::unset_action(){
	baction = nullptr;
}
void Button::init(const char* text, int x, int y, int z,int w, int h){
	this->text = text;
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
	this->h = h;
	// HACK! the 10 is a hardcoded value for the width of the text which is
	// displayed in the button.
	if(this->w < (int)this->text.length() * 10){
		this->w = this->text.length() * 10;
	}
	this->pad_x = 0;
	this->pad_y = 0;
	hitbox.add_rect(0,0,0,this->w,this->h);
	hitbox.hook(&this->x, &this->y,&this->z);
	hit_flag = false;
	Logger::getLog().log(Logger::DEBUG, "Button::init(text=%s,x=%d,y=%d,z=%d,w=%d,h=&d,hitflag=%d)",
		this->text.c_str(), this->x, this->y,this->z, this->w, this->h, this->hit_flag);
}
void Button::set_pad(int pad_x, int pad_y){
	this->pad_x = pad_x;
	this->pad_y = pad_y;
}

void exit_button_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "exit_button_action");
	SDL_Event ev;
	SDL_zero(ev);
	ev.type = SDL_QUIT;
	ev.quit.type = SDL_QUIT;
	ev.quit.timestamp = SDL_GetTicks();
	SDL_PushEvent(&ev);
}
void end_turn_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "end_turn_action");
	model.end_turn();
	view.set_state(View_Game::state_e::NORMAL);
	//Util::get().push_userev(Util::get().get_userev("view_switch_event"),0,0,0);
}
void add_road_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "add_road_action");
	view.set_state(View_Game::BUILD_ROAD);
}
void roll_action(View_Game& view, Model& model){
	// TODO: Fuck this hack. I need a static stirng to set the message text.

	static std::string msg = "Roll = ";
	Logger::getLog().log(Logger::DEBUG, "roll_action");
	model.roll(2, 6);
	model.give_resources_from_roll(model.get_roll_value());
	
	msg = "Roll =";
	msg += std::to_string(model.get_roll_value());

	view.set_message_pane_text(msg.c_str());
}
void enable_debug_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "enabe_debug_action");
	//view.debug = (view.debug) ? false : true;
	view.open_debug_dialog();
}
void add_settlement_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "add_settlement_action");
	view.set_state(View_Game::BUILD_SETTLEMENT);
}
void add_city_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "add_city_action");
	view.set_state(View_Game::BUILD_CITY);
}
void buy_dev_card_action(View_Game& view, Model& model){
	static std::string message = "";
	Logger::getLog().log(Logger::DEBUG, "buy_dev_card_action");
	if(model.buy_dev_card(model.get_current_player()) ){
		Player* p = model.get_player(model.get_current_player());
		dev_cards_t* card = (dev_cards_t*) p->dev_cards.back();
		message = card->title();
		view.set_message_pane_text(message.c_str());
	}
}
void play_dev_card_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "play_dev_card_action");
	view.set_state(View_Game::PLAY_DEV_CARD);
}
void trade_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "trade_action");
	view.set_state(View_Game::state_e::TRADING);
	view.open_trade_dialog();
}
void empty_action(View_Game& view, Model& model){
	Logger::getLog().log(Logger::DEBUG, "empty_action");
	view._debug.panel_page = (view._debug.panel_page + 1) % 3;
	model.reset();
}

/*
Button::Button(SDL_Renderer* ren, int x, int y, int w, int h){
	m_box = { x, y, w, h };
	m_clip = &m_frames[MOUSEOUT];
	m_image = Util::load_texture("data/button_sprite_sheet.png", ren);
	m_frames[MOUSEOUT] = { 0, 0, clip_w, clip_h }; // red
	m_frames[PRESSED] = { clip_w, 0, clip_w, clip_h }; //green
	m_frames[NOTPRESSED] = { clip_w*2, 0, clip_w, clip_h }; //purple
	m_frames[MOUSEOVER] = { clip_w*3, 0, clip_w, clip_h }; // blue
}
Button::~Button(){
	SDL_DestroyTexture(m_image);
}
void Button::draw(SDL_Renderer* ren){
	Util::render_texture(ren, m_image, m_box.x, m_box.y, m_box.w, m_box.h, m_clip);
}
void Button::handle_event(SDL_Event* ev){
	if(ev->type == SDL_MOUSEBUTTONDOWN ||
		ev->type == SDL_MOUSEBUTTONUP ||
		ev->type == SDL_MOUSEMOTION)
	{
		Logger& logger = Logger::getLog();

		// Get the current state of the mouse
		SDL_Rect mouse = { 0, 0, 1, 1 };
		SDL_Rect result;
		Uint32 buttons;
		buttons = SDL_GetMouseState(&mouse.x, &mouse.y);

		// Determine the next m_clip state
		if(SDL_IntersectRect(&mouse, &m_box, &result) == SDL_TRUE){
			if(ev->type == SDL_MOUSEMOTION){
				// We have just mouse overed the button
				m_clip = &m_frames[MOUSEOVER];
			}
			else{
				if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)){
					// The button was just clicked
					m_clip = &m_frames[PRESSED];
				}else{
					// The button was not pressed
					m_clip = &m_frames[NOTPRESSED];
				}
			}
		}
		else{
			// We are not even over the button
			m_clip = &m_frames[MOUSEOUT];
		}
	}
}
*/