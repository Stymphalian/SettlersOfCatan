#pragma once
#include <SDL.h>
#include "CheckBox.h"
#include "IDialog.h"
class Collision;
class IView;
class Button;
class view_debug_t;
class View_Game;

// ---------------------------------------------------------------
//  V I E W _ G A M E _ D E B U G _ D I A L O G _ C H E C K B O X
// ---------------------------------------------------------------

class View_Game_Debug_Dialog_CheckBox : public CheckBox{
public:
	View_Game_Debug_Dialog_CheckBox();
	~View_Game_Debug_Dialog_CheckBox();
	void set_data(void* data);
private:
	bool* _monitor_value;
	void on_tick_action();
	void off_tick_action();
};

// ---------------------------------------------------------
//  V I E W _ G A M E _ D E B U G _ D I A L O G 
// ---------------------------------------------------------
class View_Game_Debug_Dialog : public IDialog{
	View_Game& view_game;
public:
	View_Game_Debug_Dialog(View_Game& view, int x, int y, int z, int w, int h);
	//View_Game_Debug_Dialog(View_Play& view, int x, int y, int z, int w, int h);	
	virtual ~View_Game_Debug_Dialog();

	virtual bool open(void* data);
	virtual void* close();
	virtual void handle_keyboard_events(SDL_Event& ev);
	virtual void handle_mouse_events(SDL_Event& ev);
	virtual void update(SDL_Event& ev);
	virtual void render();
private:
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;

	view_debug_t* _data;
	CheckBox *_selected_checkbox;

	View_Game_Debug_Dialog_CheckBox _board_tiles;
	View_Game_Debug_Dialog_CheckBox _board_vertices;
	View_Game_Debug_Dialog_CheckBox _board_faces;
	View_Game_Debug_Dialog_CheckBox _selected_tile_vertices;
	View_Game_Debug_Dialog_CheckBox _selected_tile_faces;
	View_Game_Debug_Dialog_CheckBox _selected_vertex_vertices;
	View_Game_Debug_Dialog_CheckBox _selected_vertex_faces;
	View_Game_Debug_Dialog_CheckBox _selected_vertex_tiles;
	View_Game_Debug_Dialog_CheckBox _selected_face_vertices;
	View_Game_Debug_Dialog_CheckBox _selected_face_faces;
	View_Game_Debug_Dialog_CheckBox _selected_face_tiles;
	View_Game_Debug_Dialog_CheckBox _selected_pane;
	View_Game_Debug_Dialog_CheckBox _item_numbering;
	std::vector<CheckBox*> _checkboxes;

	CheckBox_Group _board_group;
	CheckBox_Group _selected_tile_group;
	CheckBox_Group _selected_vertex_group;
	CheckBox_Group _selected_face_group;
};

