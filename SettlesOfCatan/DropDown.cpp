#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

#include "Logger.h"
#include "Util.h"
#include "Collision.h"
#include "DropDown.h"


DropDown::DropDown(){
	Logger::getLog().log(Logger::DEBUG, "DropDown constructor");
	init(0, 0, 0, 0, 0, 0, 0);	
}

DropDown::~DropDown()
{
	items.clear();
	open_hitbox.clear();
	closed_hitbox_button.clear();
	closed_hitbox.clear();

	Logger::getLog().log(Logger::DEBUG, "DropDown destructor");
}

void DropDown::init(int x, int y, int z, int cols, int rows, int char_w, int char_h){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = char_w*cols;
	this->h = 2*char_h*rows;
	this->cols = cols;
	this->rows = rows;
	this->char_w = char_w;
	this->char_h = char_h;
	this->box_w = this->w;
	this->box_h = this->h;	

	this->items.clear();
	this->selected_item = -1;
	this->hover_item = -1;
	this->has_focus = false;
	this->is_open = false;

	// configurables
	_button_w = this->w/6;
	_button_h = this->h;
	_convenience_padding = 30;
	_background_colour = { 0, 0, 0, 0 };
	_foreground_colour = { 0, 0, 0, 0 };

	// hitboxes
	open_hitbox.clear();
	open_hitbox.hook(&this->x, &this->y, &this->z);

	open_convenience_hitbox.clear();
	open_convenience_hitbox.hook(&this->x, &this->y, &this->z);

	closed_hitbox.clear();
	closed_hitbox.hook(&this->x, &this->y, &this->z);
	closed_hitbox.add_rect(0, 0, 0, this->w, this->h);

	closed_hitbox_button.clear();
	closed_hitbox_button.hook(&this->x, &this->y, &this->z);
	closed_hitbox_button.add_rect(this->w - _button_w, 0, 0, _button_w, this->h);		
}

void DropDown::handle_mouse_events(SDL_Event& ev, Collision& rel_mouse){
	hover_item = -1;

	if(is_open)
	{
		// O P E N   state
		// we have lost focus, so get out.
		if(rel_mouse.collides(open_hitbox) == false &&
			rel_mouse.collides(open_convenience_hitbox) == false &&
			rel_mouse.collides(closed_hitbox) == false &&
			rel_mouse.collides(closed_hitbox_button) == false)					
		{
			off_focus();
			return;
		}

		hover_item = open_hitbox.get_index_of_hit();
		if(ev.type == SDL_MOUSEBUTTONDOWN){		
			if(rel_mouse.collides(closed_hitbox_button)){
				is_open = false;
			} else{
				selected_item=open_hitbox.get_index_of_hit();
				is_open = false;
			}			
		} 
	} 
	else
	{
		// C L O S E D   state
		if(rel_mouse.collides(closed_hitbox) == false){
			off_focus();
			return;
		}
		
		if(ev.type == SDL_MOUSEBUTTONDOWN){
			has_focus = true;

			if(rel_mouse.collides(closed_hitbox_button) ||
				rel_mouse.collides(closed_hitbox))
			{
				is_open = true;
			}
		} 
	}
}
void DropDown::handle_keyboard_events(SDL_Event& ev){
	if(has_focus == false){ return; }
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);

	if(keyboard[SDL_SCANCODE_ESCAPE]){
		off_focus();		
		return;
	}

	if(ev.type == SDL_KEYDOWN){	
		is_open = true;
		// handle arrow key navigation
		do{
			if(items.size() == 0){ break; }

			if(ev.key.keysym.scancode == SDL_SCANCODE_DOWN){
				hover_item = (hover_item + 1) % items.size();
			} else if(ev.key.keysym.scancode == SDL_SCANCODE_UP){
				hover_item--;
				if(hover_item < 0){ hover_item = items.size() - 1; }				
			} else if(ev.key.keysym.scancode == SDL_SCANCODE_RETURN){
				selected_item = hover_item;
				off_focus();
			}
		} while(false);

	}
}

void DropDown::render(SDL_Renderer& ren, TTF_Font& font, SDL_Color font_colour, SDL_Rect& clip){
	SDL_Rect bound = {
		this->x + clip.x,
		this->y + clip.y,
		this->w, this->h };
	SDL_Rect temp_rect = { 0, 0, 0, 0 };

	// Draw the normal hitbox
	temp_rect = {
		closed_hitbox.rects[0].x + bound.x,
		closed_hitbox.rects[0].y + bound.y,
		closed_hitbox.rects[0].w,
		closed_hitbox.rects[0].h
	};
	// Draw how it looks like if it is closed
	Util::render_fill_rectangle(&ren, &temp_rect,_background_colour);
	Util::render_rectangle(&ren, &temp_rect, Util::colour_red());

	// draw the little arrow button box thing.
	temp_rect.x += closed_hitbox.rects[0].w - _button_w;
	temp_rect.w = _button_w;
	Util::render_rectangle(&ren, &temp_rect, Util::colour_blue());

	if(is_open == false){
		// C L O S E D
		if(selected_item != -1){
			Util::render_text(&ren, &font, bound.x + 5, bound.y, font_colour, "%s", items[selected_item].c_str());
		}

	} else{
		// O P E N
		// draw the background colour for all the open boxes..
		temp_rect = {
			open_hitbox.rects[0].x + bound.x,
			open_hitbox.rects[0].y + bound.y,			
			open_hitbox.rects[0].w,
			open_hitbox.rects[items.size() -1].y
		};
		//printf("%d,%d,%d,%d\n", temp_rect.x, temp_rect.y,temp_rect.w, temp_rect.h);
		Util::render_fill_rectangle(&ren, &temp_rect, _background_colour);

		// draw the hovered item.
		if(hover_item != -1){
			int padding = 3;
			temp_rect = {
				open_hitbox.rects[hover_item].x + bound.x + padding,
				open_hitbox.rects[hover_item].y + bound.y + padding,
				open_hitbox.rects[hover_item].w - 2*padding,
				open_hitbox.rects[hover_item].h - 2*padding
			};
			Util::render_fill_rectangle(&ren, &temp_rect, Util::colour_orange());
		}

		for(int i = 0; i < (int) items.size(); ++i){
			// draw the bounding box
			temp_rect = {
				open_hitbox.rects[i].x + bound.x,
				open_hitbox.rects[i].y + bound.y,
				open_hitbox.rects[i].w,
				open_hitbox.rects[i].h
			};
			
			Util::render_rectangle(&ren, &temp_rect, Util::colour_green());
			// draw the inner text.
			Util::render_text(&ren, &font, temp_rect.x + 5, temp_rect.y, font_colour, "%s", items[i].c_str());
		}


		if(items.size() != 0 && false){
			temp_rect = {
				open_convenience_hitbox.rects[0].x + bound.x,
				open_convenience_hitbox.rects[0].y + bound.y,
				open_convenience_hitbox.rects[0].w,
				open_convenience_hitbox.rects[0].h
			};
			Util::render_rectangle(&ren, &temp_rect, Util::colour_orange());
		}
		
	}
	/*
	Util::render_text(&ren, &font, 0, 480, font_colour,
		"hover_item=%d selected_item=%d has_focus=%d is_open=%d",
		hover_item, selected_item, has_focus, is_open
		);
	*/
}

Collision& DropDown::get_collision(){
	return (is_open) ? open_hitbox : closed_hitbox;
}

bool DropDown::append_item(std::string item){
	return add_item(item, items.size());
}
bool DropDown::add_item(std::string item, int pos){
	if(pos <0 || pos > (int)items.size()){ return false; }
	items.insert(items.begin() + pos, item);

	this->selected_item = -1;
	// reorder all the hitboxes...
	fill_open_hitbox();
	// modify the the height;
	this->rows = items.size() + 1;
	this->h = this->rows*box_h;
	return true;
}
// 0 is the first item
// items.size() -1 is the last
bool DropDown::remove_item(int pos){
	if(pos < 0 || pos >= (int)items.size()){ return false; }
	items.erase(items.begin() + pos);

	this->selected_item = -1;
	// reorder all the hitboxes...	
	fill_open_hitbox();
	// modify the height;
	this->rows = items.size() + 1;
	this->h = this->rows*box_h;
	return true;
}
int DropDown::num_items(){
	return (int)items.size();
}

std::string DropDown::get_selected_value(){
	return (selected_item == -1) ? "" : items[selected_item];
}

int DropDown::get_selected_index(){
	return this->selected_item;
}
void DropDown::set_selected_index(int index){
	if(index == -1){
		selected_item = -1;
		return; // important that we return here.
	}
	selected_item = index;
	validate_selected(); // 0 < index < items.size()-1
}
std::string DropDown::get_value_at_index(int index){
	if(index < 0 || index >= (int)items.size()){ return ""; }
	return items[index];
}
void DropDown::set_value_at_index(int index, std::string newvalue){
	if(index < 0 || index >= (int)items.size()){ return; }
	items[index] = newvalue;
}

void DropDown::set_background_colour(SDL_Color colour){
	_background_colour = colour;
}

void DropDown::set_foreground_colour(SDL_Color colour){
	_foreground_colour = colour;
}

// ----------------------------------------------------------
// -- - - - - -  P R I V A T E   M E T H O D S - - - - - - - - 
// ----------------------------------------------------------

void DropDown::off_focus(){
	hover_item = -1;
	has_focus = false;
	is_open = false;
}

void DropDown::validate_selected(){
	if(selected_item < 0){ selected_item = 0; }
	else if(selected_item >= (int) items.size()){ selected_item = items.size()-1; }
}

void DropDown::fill_open_hitbox(){
	open_hitbox.clear();

	int x_offset = 0;
	int y_offset = this->box_h;
	for(int i = 0; i < (int) items.size(); ++i){
		open_hitbox.add_rect(x_offset, y_offset, 0, this->box_w, this->box_h);

		x_offset += 0;
		y_offset += this->box_h;		
	}

	// We add a small bouding box around the dropdown
	// such that the user has more room to manouver
	open_convenience_hitbox.clear();
	if(items.size() != 0){	
		open_convenience_hitbox.add_rect(
			open_hitbox.rects[0].x - _convenience_padding,
			open_hitbox.rects[0].y,
			0,
			open_hitbox.rects[items.size() - 1].w + 2*_convenience_padding,
			open_hitbox.rects[items.size() - 1].y + open_hitbox.rects[items.size() - 1].h + _convenience_padding/2
			);
	}
}	