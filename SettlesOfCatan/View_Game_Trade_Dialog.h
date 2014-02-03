#pragma once
#include "IDialog.h"
class View_Game;

class View_Game_Trade_Dialog : public IDialog{
	View_Game& view_game;
public:
	View_Game_Trade_Dialog(View_Game& view, int x, int y, int z, int w, int h);
	virtual ~View_Game_Trade_Dialog();

	void handle_keyboard_events(SDL_Event& ev){}
	void handle_mouse_events(SDL_Event& ev){}
	void update(SDL_Event& ev){}
	void render(){}
};