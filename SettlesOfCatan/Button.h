#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "Sprite.h"
#include "Collision.h"
class Model;
class View_Game;

class Button {
	enum frames {MOUSEOUT,PRESSED, NOTPRESSED, MOUSEOVER, num_frames };
	typedef void(*button_action)(View_Game& view, Model& model);
public:
	// variables
	std::string text;
	Collision hitbox;
	int x, y,z, w, h;
	int pad_x, pad_y;
	bool hit_flag;
	
	// constructor and destructor
	Button();
	Button(const char* text, int x, int y, int z, int w, int h);
	virtual ~Button(); 
	void action(View_Game& view, Model& model);
	void set_action(Button::button_action baction);
	void unset_action();
	void init(const char* text, int x, int y,int z, int w, int h);
	void set_pad(int padx, int pady);
private:
	button_action baction;
};

void exit_button_action(View_Game& view,Model& model);
void end_turn_action(View_Game& view, Model& model);
void roll_action(View_Game& view, Model& model);
void add_road_action(View_Game& view, Model& model);
void add_settlement_action(View_Game& view, Model& model);
void add_city_action(View_Game& view, Model& model);
void buy_dev_card_action(View_Game& view, Model& model);
void play_dev_card_action(View_Game& view, Model& model);
void trade_action(View_Game& view, Model& model);