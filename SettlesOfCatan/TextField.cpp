#include <SDL.h>
#include <string>
#include "Collision.h"
#include "Logger.h"

#include "TextField.h"


TextField::TextField()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->w = 0;
	this->h = 0;
	this->has_focus = false;
	this->cursor_blink = false;
	this->left_pos = 0;
	this->right_pos = 0;
	_anchor_pos = 0;
	_magnitude = 0;
	_mouse_buttons = 0;
	_mouse_x = 0;
	_mouse_y = 0;	
	_text_range = 0;
	_tick = 0;
	_tick_interval = 15;
	_relative_mouse.hook(&_mouse_x, &_mouse_y);
	_relative_mouse.add_rect(0, 0, 0,1, 1);
}

TextField::~TextField(){

}

void TextField::init(int x, int y, int z, int cols, int rows, int char_w, int char_h,int padding){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = cols*char_w + padding;
	this->h = rows*char_h;
	this->rows = rows;
	this->cols = cols;
	this->char_w = char_w;
	this->char_h = char_h;
	this->padding = padding;

	this->has_focus = false;
	this->cursor_blink = false;
	//this->only_col_chars = true;
	
	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_buttons = 0;
	left_pos = 0;
	right_pos = 0;
	_text_range = 0;
	_tick = 0;
	_magnitude = 0;

	hitbox.rects.clear();
	hitbox.hook(&this->x, &this->y, &this->z);
	hitbox.add_rect(0,0,0,this->w, this->h);
}

void TextField::handle_mouse_events(SDL_Event& ev){
	_mouse_buttons = SDL_GetMouseState(&_mouse_x, &_mouse_y);
	_mouse_x -= this->x;
	_mouse_y -= this->y;	
	if(_relative_mouse.collides(hitbox)){
		if(ev.type == SDL_MOUSEBUTTONDOWN){
			on_keyboard_focus();
			has_focus = true;
			_tick = 0;
			cursor_blink = false;
		}
	} else{
		if(ev.type == SDL_MOUSEBUTTONDOWN){
			off_keyboard_focus();
			has_focus = false;
			cursor_blink = false;
			_tick = 0;
		}
	}
	
}
void TextField::handle_keyboard_events(SDL_Event& ev){
	if(has_focus == false){ return; }
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			off_keyboard_focus();	
			return;
		}

		if(ev.key.keysym.sym == SDLK_BACKSPACE){
			if(_text_range != 0){
				erase_character_range();
			} else{
				erase_character_left();
			}
		} else if( ev.key.keysym.sym == SDLK_DELETE){
			if(_text_range != 0){
				erase_character_range();
			} else{
				erase_character_right();
			}
		//} else if(ev.key.keysym.sym >= SDLK_RETURN && ev.key.keysym.sym < SDLK_a){
			//append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym >= SDLK_a && ev.key.keysym.sym <= SDLK_z){
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym == SDLK_SPACE || ev.key.keysym.sym == SDLK_TAB){
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym >= SDLK_0 && ev.key.keysym.sym <= SDLK_9){
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT){
			int dir = (ev.key.keysym.sym == SDLK_LEFT) ? -1 : 1;
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				left_pos += dir;
				if(left_pos < 0){ left_pos = 0; }
			} else{
				_anchor_pos += dir;
				if(_anchor_pos < 0){ _anchor_pos = 0; }
				if(_anchor_pos >= (int)text.length()) { _anchor_pos = text.length(); }
				assign_left_right_from_anchor();
			}
		} else if(ev.key.keysym.sym == SDLK_HOME){
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				left_pos = 0;
			} else{
				right_pos = 0;
				left_pos = 0;
			}
			_text_range = right_pos - left_pos;
		} else if(ev.key.keysym.sym == SDLK_END){
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				right_pos = text.length();
			} else{
				right_pos = text.length();
				left_pos = text.length();
			}
			_text_range = right_pos - left_pos;
		}
	}
}
void TextField::tick(){
	if(has_focus){		
		// continue ticking, changing the cursor blink if necessary.
		_tick = (_tick + 1) % _tick_interval;
		if(_tick == 0){ cursor_blink = !cursor_blink; }
	}
}

void TextField::clear_text(){
	text.clear();
	left_pos = 0;
	right_pos = 0;
	_text_range = 0;
}
void TextField::append_character(char c){
	if(_text_range != 0){ erase_character_range(); }
	std::string left = text.substr(0, left_pos);
	std::string right = text.substr(right_pos,text.length());
	text = left;
	text += c;
	text += right;

	right_pos++;
	left_pos = right_pos;
	_text_range = right_pos - left_pos;
}
void TextField::erase_character_left(){
	if(text.length() > 0){
		if(left_pos == right_pos ){
			if(right_pos - 1 < 0){ return; }
			text.erase(right_pos-1, 1);
			// adjust the cursor position.
			right_pos--;
			if(right_pos < 0){ right_pos = 0; }
			left_pos = right_pos;
		} 
		_text_range = right_pos - left_pos;
	}
}

void TextField::erase_character_right(){
	if(text.length() > 0){
		if(left_pos == right_pos ){
			if(right_pos  >= (int) text.length()){ return; }
			text.erase(right_pos , 1);
			// adjust the cursor position.
			right_pos--;
			if(right_pos < 0){ right_pos = 0; }
			left_pos = right_pos;
		}
		_text_range = right_pos - left_pos;
	}
}

void TextField::erase_character_range(){
	if(text.length() > 0){
		if(_text_range != 0 ){
			text.erase(left_pos, _text_range);
			// adjust the cursor position.
			right_pos -= _text_range;
			if(right_pos < 0){ right_pos = 0; }
			left_pos = right_pos;
		}
		_text_range = right_pos - left_pos;
	}
}
void TextField::set_char_w(int char_w){
	hitbox.remove_rect(0, 0, 0, this->w, this->h);
	this->char_w = char_w;
	this->w = this->char_w*this->cols;
	hitbox.add_rect(0, 0, 0, this->w, this->h);
}
void TextField::set_char_h(int char_h){
	hitbox.remove_rect(0, 0, 0, this->w, this->h);
	this->char_h = char_h;
	this->h = this->char_h*this->rows;
	hitbox.add_rect(0, 0, 0, this->w, this->h);
}
void TextField::set_num_columns(int cols){
	hitbox.remove_rect(0, 0, 0, this->w, this->h);
	this->cols = cols;
	this->w = this->char_w*this->cols;
	hitbox.add_rect(0, 0, 0, this->w, this->h);
}
void TextField::set_num_rows(int rows){
	hitbox.remove_rect(0, 0, 0, this->w, this->h);
	this->rows = rows;
	this->h = this->char_h*this->rows;
	hitbox.add_rect(0, 0, 0, this->w, this->h);
}
void TextField::on_keyboard_focus(){
	has_focus = false;
	cursor_blink = false;
	_tick = 0;
}
void TextField::off_keyboard_focus(){
	has_focus = false;
	cursor_blink = false;
	_tick = 0;
}

/*
Assuming _magnitude and _anchor are valid.
*/
void TextField::assign_left_right_from_anchor(){
	if(_magnitude ==0 ){
		this->left_pos = _anchor_pos;
		this->right_pos = _anchor_pos;
		return;
	} else{
		if(_magnitude < 0){
			this->left_pos = _anchor_pos + _magnitude;
			this->right_pos = _anchor_pos - _magnitude;
		} else{
			this->left_pos = _anchor_pos - _magnitude;
			this->right_pos = _anchor_pos + _magnitude;
		}
	}
	_text_range = this->right_pos - this->left_pos;
}