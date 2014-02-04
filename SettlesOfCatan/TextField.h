#pragma once
#include <SDL.h>
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
	int left_pos;
	int right_pos;
	//bool only_col_chars;
	
	TextField();
	~TextField();

	void init(int x, int y, int z, int cols, int rows,
				int char_w, int char_h, int padding = 10);
	void set_char_w(int char_w);
	void set_char_h(int char_h);
	void set_num_columns(int cols);
	void set_num_rows(int rows);

	void handle_mouse_events(SDL_Event& ev);
	void handle_keyboard_events(SDL_Event& ev);
	void tick();


	int _anchor_pos;
	int _magnitude;
	int _text_range;
	int _tick;
	int _tick_interval;
private:
	Collision _relative_mouse;
	Uint32 _mouse_buttons;
	int _mouse_x;
	int _mouse_y;
	
	void clear_text();
	void append_character(char c);
	void erase_character_right();	
	void erase_character_left();
	void erase_character_range();
	void on_keyboard_focus();
	void off_keyboard_focus();
	void assign_left_right_from_anchor();
};

