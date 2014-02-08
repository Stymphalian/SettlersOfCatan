#pragma once
#include <SDL.h>
#include "IDialog.h"
#include "Button.h"
#include "ComboBox.h"
#include "DropDown.h"
#include "TextField.h"
class View_Game;
class Collision;
class Model;
class View_Game_Trade_Dialog;


class View_Game_Trade_Dialog_DropDown : public DropDown{
public:
	View_Game_Trade_Dialog_DropDown();
	virtual ~View_Game_Trade_Dialog_DropDown();
private:
};

class View_Game_Trade_Dialog_ComboBox : public  ComboBox{
public:
	View_Game_Trade_Dialog_ComboBox();
	virtual ~View_Game_Trade_Dialog_ComboBox();
private:
};


// basically nothing is special about these textfields.s
class View_Game_Trade_Dialog_TextField : public TextField{
public:
	View_Game_Trade_Dialog_TextField();
	virtual ~View_Game_Trade_Dialog_TextField();
private:
};

class View_Game_Trade_Dialog_Button : public Button{
	typedef void (*button_action)(View_Game_Trade_Dialog* dialog, Model* model);
public:
	View_Game_Trade_Dialog_Button();
	virtual ~View_Game_Trade_Dialog_Button();
	void action(View_Game_Trade_Dialog* dialog, Model* model);
	void set_action(button_action baction);
	void unset_action();

	// buttons actions that we can hook
	static void submit_button_action(View_Game_Trade_Dialog* dialog, Model* model);
	static void cancel_button_action(View_Game_Trade_Dialog* dialog, Model* model);
private:	
	button_action _baction;
};


class View_Game_Trade_Dialog : public IDialog{
	View_Game& view_game;
public:
	View_Game_Trade_Dialog(View_Game& view, int x, int y, int z, int w, int h);
	virtual ~View_Game_Trade_Dialog();

	virtual bool open(void* data);
	virtual void* close();
	virtual void handle_keyboard_events(SDL_Event& ev);
	virtual void handle_mouse_events(SDL_Event& ev);
	virtual void update(SDL_Event& ev);
	virtual void render();
private:
	friend class View_Game_Trade_Dialog_Button;

	int _mouse_x, _mouse_y;
	Uint32 _mouse_buttons;
	Collision mouse_hitbox;
	Model* _model;

	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;

	// buttons
	View_Game_Trade_Dialog_Button* selected_button;
	View_Game_Trade_Dialog_Button submit_button;
	View_Game_Trade_Dialog_Button cancel_button;
	std::vector<View_Game_Trade_Dialog_Button*> button_list;

	TextField* selected_textfield;
	// left pane
	View_Game_Trade_Dialog_TextField left_brick_textfield;
	View_Game_Trade_Dialog_TextField left_ore_textfield;
	View_Game_Trade_Dialog_TextField left_sheep_textfield;
	View_Game_Trade_Dialog_TextField left_wheat_textfield;
	View_Game_Trade_Dialog_TextField left_wood_textfield;
	std::vector<View_Game_Trade_Dialog_TextField*> left_textfields;
	// right pane
	View_Game_Trade_Dialog_DropDown* selected_dropdown;
	View_Game_Trade_Dialog_DropDown right_dropdown;	
	std::vector<View_Game_Trade_Dialog_DropDown*> right_dropdown_list;

	View_Game_Trade_Dialog_TextField right_brick_textfield;
	View_Game_Trade_Dialog_TextField right_ore_textfield;
	View_Game_Trade_Dialog_TextField right_sheep_textfield;
	View_Game_Trade_Dialog_TextField right_wheat_textfield;
	View_Game_Trade_Dialog_TextField right_wood_textfield;
	std::vector<View_Game_Trade_Dialog_TextField*> right_textfields;
	
};