#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include "Collision.h"

class DropDown{
public:
	int x, y, z, w, h;
	int cols, rows, char_w, char_h;
	int box_w, box_h;
	std::vector<std::string> items;
	int hover_item;
	int selected_item; // -1 if nothing is selected
	bool has_focus;
	bool is_open;

	// construtor and destructor
	DropDown();
	virtual ~DropDown();

	// methods
	void init(int x, int y, int z, int cols, int rows, int char_w, int char_h);
	void handle_mouse_events(SDL_Event& ev, Collision& rel_mouse);
	void handle_keyboard_events(SDL_Event& ev);
	void render(SDL_Renderer& ren, TTF_Font& font, SDL_Color font_colour,SDL_Rect& clip);
	Collision& get_collision();
	bool append_item(std::string item);
	bool add_item(std::string item, int pos);
	bool remove_item(int pos);
	int num_items();
	std::string get_selected_value();
	void set_selected_index(int index);
	std::string get_value_at_index(int index);	
	void set_value_at_index(int index, std::string value);	
private:
	Collision open_hitbox; // only contains the hitboxes of the 'dropdown' so not the initial bar.
	Collision open_convenience_hitbox;
	Collision closed_hitbox_button;
	Collision closed_hitbox;
	// configurables
	int _button_w;
	int _button_h;
	int _convenience_padding;
	
	// methods
	void off_focus();
	void validate_selected();
	void fill_open_hitbox();
};

