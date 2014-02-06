#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "Collision.h"
#include "Logger.h"
#include "Util.h"

#include "TextField.h"


TextField::TextField(int tick_interval)
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->w = 0;
	this->h = 0;
	this->rows = 0;
	this->cols = 0;
	this->char_w = 0;
	this->char_h = 0;
	this->padding = 0;
	this->has_focus = false;
	this->cursor_blink = false;

	_anchor_pos = 0;
	_magnitude = 0;
	_tick = 0;
	_tick_interval = tick_interval; // Make this configurable?
	_win_left = 0;
	_win_right = 0;
	_capacity = -1;

	Logger::getLog().log(Logger::DEBUG, "TextField constructor");
}

TextField::~TextField(){
	Logger::getLog().log(Logger::DEBUG, "TextField destructor");
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

	_anchor_pos = 0;
	_magnitude = 0;
	_tick = 0;
	// _tick_interval is unchanged		
	_win_left = 0;
	_win_right = this->cols;
	_capacity = -1;

	// add the hitbox for the text field.
	hitbox.rects.clear();
	hitbox.hook(&this->x, &this->y, &this->z);
	hitbox.add_rect(0,0,0,this->w, this->h);
}

void TextField::handle_mouse_events(SDL_Event& ev,Collision& rel_mouse){
	if(rel_mouse.collides(hitbox)){
		if(ev.type == SDL_MOUSEBUTTONDOWN){
			on_keyboard_focus();			
		}
	} else{
		if(ev.type == SDL_MOUSEBUTTONDOWN){
			off_keyboard_focus();		
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

		if(ev.key.keysym.sym == SDLK_BACKSPACE  || ev.key.keysym.sym == SDLK_DELETE){
			// Handle delete using backsapce or delete
			int amount = 1;
			bool at_end_of_str = (get_cursor() >= (int)text.length() - this->cols);

			if(_magnitude != 0){
				amount = abs(_magnitude);
				erase_character_range();
			} else if(ev.key.keysym.sym == SDLK_BACKSPACE){
				erase_character_left();
			} else {
				erase_character_right();
			}

			// we are in the last segment, therfore we have
			// special behavriou in what we should be viewing
			if( at_end_of_str){
				_win_left  -= amount;
				_win_right -= amount;
			}

		} else if(ev.key.keysym.sym >= SDLK_a && ev.key.keysym.sym <= SDLK_z){
			// handle a-z keys
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym == SDLK_SPACE || ev.key.keysym.sym == SDLK_TAB){
			// handle tab and space characters
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym >= SDLK_0 && ev.key.keysym.sym <= SDLK_9){
			// handle 0-9
			append_character(ev.key.keysym.sym);
		} else if(ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT){
			// handle left and right arrow keys
			int dir = (ev.key.keysym.sym == SDLK_LEFT) ? -1 : 1;
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				// move 'selection' cursor one character over.
				_magnitude += dir;
				validate_mag();
			} else{
				// if the we have somethign selected, then the cursor
				// should appear the far-left or far-right of the selection
				// depending on the direction.
				if(_magnitude != 0){
					_anchor_pos = _anchor_pos + _magnitude;
				}

				// move the cursor one character over.
				_anchor_pos += dir;
				_magnitude = 0;
				validate_anchor();
			}
		} else if(ev.key.keysym.sym == SDLK_HOME){
			// handle the HOME key
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				// select everything from the cursor to the beginning of the string
				_magnitude = -(_anchor_pos);
				validate_mag();
			} else{
				// move the cursor to the beginning of the string
				// select nothing
				_anchor_pos = 0;
				_magnitude = 0;
			}
		} else if(ev.key.keysym.sym == SDLK_END){
			// handle they END key
			if(keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT]){
				// select all the way to the end of the string
				_magnitude = text.length() - _anchor_pos;
				validate_mag();
			} else{
				// move the cursor to the end of the string.
				// select nothing.
				_anchor_pos = text.length();
				_magnitude = 0;

			}
		}

		// check to see if the cursor position causes a shift in the 
		// view window.
		check_for_wall_hit();
	}
}

void TextField::render(SDL_Renderer& ren,TTF_Font& font,SDL_Color& font_colour,SDL_Rect& clip){
	SDL_Color red = { 255, 0, 0, 150 };
	SDL_Color green = { 0, 255, 0, 180 };
	SDL_Color blue = { 0, 0, 255, 250 };
	SDL_Rect rect = { this->x + clip.x,
							this->y + clip.y,
							this->w,this->h };

	// draw bounding box which is bounding the text field
	Util::render_rectangle(&ren, &rect, red);

	// these variables hold the character positions
	// of the visible charcters in the text field.
	int start_pos = 0;
	int end_pos = 0;
	get_viewable_range(&start_pos, &end_pos);


	// show the "selected" text ( i.e high the selected par of the text)
	int start_select = left_pos();
	int end_select = right_pos();
	// Only show the text if something is ACTUALLY selected.
	if(end_select - start_select != 0){
		if(start_select < start_pos){ start_select = start_pos; }
		if(end_select > end_pos) { end_select = end_pos; }

		//normalize to fit into the range of the text field
		start_select -= start_pos;
		end_select -= start_pos;

		int start_px = start_select*this->char_w + this->x + this->padding / 2;
		int end_px = end_select*this->char_w + this->x + this->padding / 2;

		SDL_Rect select_rect = {
			clip.x + start_px,
			clip.y + this->y + 5,
			end_px - start_px,
			this->h - 10
		};
		
		Util::render_fill_rectangle(&ren, &select_rect, green);
	}

	// display the text.
	if(this->text.length() != 0){
		std::string temp_str = this->text.substr(start_pos, this->cols);
		Util::render_text(&ren, &font,
			rect.x + this->padding/2, 
			rect.y + this->h - 5 - 10,
			font_colour, "%s", temp_str.c_str());
	}

	// display the blinking cursor
	if(this->cursor_blink){
		// show the blinking cursor.
		int cursor_pos = get_cursor() - start_pos;
		int cursor_px = cursor_pos* this->char_w + this->x + this->padding / 2;
		Util::render_line(&ren, red,
			clip.x + cursor_px, 
			clip.y + this->y + 5,
			clip.x +  cursor_px, 
			clip.y + this->y + this->h - 5);
	}

}


void TextField::tick(){
	if(has_focus){		
		// continue ticking, changing the cursor blink if necessary.
		_tick = (_tick + 1) % _tick_interval;
		if(_tick == 0){ cursor_blink = !cursor_blink; }
	}
}

int TextField::right_pos(){
	return (_magnitude < 0) ? _anchor_pos : _anchor_pos + _magnitude;
}
int TextField::left_pos(){
	return (_magnitude < 0) ? _anchor_pos + _magnitude : _anchor_pos;	
}
int TextField::get_cursor(){
	return _anchor_pos + _magnitude;
}
int TextField::get_anchor(){
	return _anchor_pos;
}

void TextField::set_text(const char* text){
	this->text = text;
	if(_capacity >= 0){
		if((int)this->text.length() > _capacity){
			this->text = this->text.substr(0, _capacity);
		}
	}
}
void TextField::clear_text(){
	text.clear();
	_anchor_pos = 0;
	_magnitude = 0;
	check_for_wall_hit();
}

void TextField::set_selected(int left, int right){
	_anchor_pos = left;
	_magnitude = right - left;
	validate_anchor();
	validate_mag();
	check_for_wall_hit();
}

void TextField::set_max_num_chars(int cap){
	_capacity = cap;
	if(_capacity >= 0){
		if((int)this->text.length() > _capacity){
			this->text = this->text.substr(0, _capacity);
		}
	}
}

void TextField::set_cursor(int pos){
	_anchor_pos = pos;
	_magnitude = 0;
	validate_anchor();
	check_for_wall_hit();
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

void TextField::get_viewable_range(int* start, int* end){
	int s = 0;
	int e = 0;

	s = _win_left;
	e = _win_left + this->cols;
	if(e > (int)text.length()){ e = (int)text.length(); }

	// assign the values
	if(start != NULL){ *start = s; }
	if(end != NULL){ *end = e; }
}


//----------------------
// P R I V A T E -- M E T H O D S
// ---------------------
void TextField::append_character(char c){
	if(_magnitude != 0){ erase_character_range(); }
	if(_capacity >= 0 && (int)text.length() + 1 > _capacity){ return; }

	std::string left = text.substr(0, left_pos());
	std::string right = text.substr(right_pos(), text.length());
	text = left;
	text += c;
	text += right;

	_anchor_pos++;
	_magnitude = 0;
	validate_anchor();
}

void TextField::erase_character_left(){
	if(text.length() > 0){
		int lpos = left_pos();
		int rpos = right_pos();
		if(lpos == rpos){
			if(rpos - 1 < 0){ return; }
			text.erase(rpos - 1, 1);
			// adjust the cursor position.
			_anchor_pos--;
			validate_anchor();
		}
	}
}

void TextField::erase_character_right(){
	if(text.length() > 0){
		int lpos = left_pos();
		int rpos = right_pos();
		if(lpos == rpos){
			if(rpos >= (int)text.length()){ return; }
			text.erase(rpos, 1);
			// adjust the cursor position.
			_anchor_pos;
			validate_anchor();
		}
	}
}

void TextField::erase_character_range(){
	if(text.length() > 0){
		if(_magnitude != 0){
			text.erase(left_pos(), abs(_magnitude));
			// adjust the cursor position.
			_anchor_pos = left_pos();
			_magnitude = 0;
			validate_anchor();
		}
	}
}

void TextField::on_keyboard_focus(){
	has_focus = true;
	cursor_blink = false;
	_tick = 0;
}
void TextField::off_keyboard_focus(){
	has_focus = false;
	cursor_blink = false;
	_tick = 0;
	_anchor_pos = get_cursor();
	_magnitude = 0;
	check_for_wall_hit();
}

void TextField::validate_anchor(){
	if(_anchor_pos < 0){ _anchor_pos = 0; } else if(_anchor_pos >(int) text.length()){ _anchor_pos = text.length(); }
}
void TextField::validate_mag(){
	if(_anchor_pos + _magnitude < 0){ _magnitude = -(_anchor_pos); } else if(_anchor_pos + _magnitude >(int)text.length()){ _magnitude = (text.length() - _anchor_pos); }

}

void TextField::check_for_wall_hit(){
	int anchor = _anchor_pos + _magnitude;
	if(anchor >= _win_right){
		_win_left += anchor - _win_right;
		_win_right += anchor - _win_right;
	} else if(anchor < _win_left){
		_win_right -= _win_left - anchor;
		_win_left -= _win_left - anchor;
	}

	if(_win_left < 0){
		_win_left = 0;
		_win_right = this->cols;
	} else{

	}
}

