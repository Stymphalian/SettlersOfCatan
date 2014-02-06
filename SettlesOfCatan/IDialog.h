#pragma once
#include <SDL.h>
#include "Checkbox.h"
class Collision;
class IView;
class Button;
class view_debug_t;
class View_Game;


class IDialog {
public:	
	IView& view;
	bool modal;
	int x, y, z, w, h;
	Collision hitbox;
	
	// constsructor and destructor
	IDialog(IView& view,int x, int y, int z, int w, int h);
	virtual ~IDialog();
	// methods
	bool isvisible();
	virtual bool open(void* data);
	virtual void* close();

	// pure virtual methods
	virtual void handle_keyboard_events(SDL_Event& ev) = 0;
	virtual void handle_mouse_events(SDL_Event& ev) = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render() = 0;

protected:
	SDL_Rect old_clip;
	SDL_Rect new_clip;
private:	
	bool _visible;
};





