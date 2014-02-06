#pragma once
#include <SDL.h>
#include<SDL_ttf.h>
#include <string>
#include "Collision.h"

class TextField{
public:
	Collision hitbox;
	int x, y, z, w, h;
	int rows, cols, char_w, char_h;
	int padding;
	std::string text;
	bool has_focus;
	bool cursor_blink;
	//bool only_col_chars;

	// constructor and destructor
	TextField(int tickinterval =15);
	virtual ~TextField();

	// methods
	void init(int x, int y, int z, int cols, int rows,
				int char_w, int char_h, int padding = 10);
	virtual void handle_mouse_events(SDL_Event& ev,Collision& rel_mouse);
	virtual void handle_keyboard_events(SDL_Event& ev);	
	virtual void render(SDL_Renderer& ren, TTF_Font& font,SDL_Color& font_colour, SDL_Rect& clip);
	virtual void tick();

	int right_pos();
	int left_pos();
	int get_cursor();
	int get_anchor();
	void set_text(const char* text);
	void clear_text();
	void set_selected(int left, int right);	
	void set_max_num_chars(int cap);
	void set_cursor(int pos);
	void set_char_w(int char_w);
	void set_char_h(int char_h);
	void set_num_columns(int cols);
	void set_num_rows(int rows);
	void get_viewable_range(int* start, int* end);
	//void set_image();
	//void set_background_colour(SDL_Color);
	//void set_foreground_colour(SDL_Color);
	//void set_font(font, char_w, char_h);
private:	
	// variables
	int _anchor_pos;
	int _magnitude;
	int _tick;
	int _tick_interval;
	int _win_left;
	int _win_right;
	int _capacity;

	//methods
	void append_character(char c);
	void erase_character_right();	
	void erase_character_left();
	void erase_character_range();
	void on_keyboard_focus();
	void off_keyboard_focus();
	void validate_anchor();
	void validate_mag();
	void check_for_wall_hit();
};

