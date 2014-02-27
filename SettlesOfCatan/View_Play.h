#pragma once
#include <algorithm>
#include <map>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "Util.h"
#include "Timer.h"
#include "IView.h"
#include "IDialog.h"
#include "CheckBox.h"
#include "View_Game.h"
#include "TextField.h"
#include "DropDown.h"

#include "mouse.h"
#include "ObserverPattern.h"
#include "MouseListener.h"
#include "Selectable.h"

#include "Coords.h"
#include "Collision.h"
#include "Sprite.h"
#include "Pane.h"

#include "Viewport.h"


class SelectablePane : public Pane{
public:
	// Sprite Interface
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	//Focusable interface
	virtual void on_focus() = 0;
	virtual void off_focus() = 0;
	//Selectable interface	
	virtual void on_selected() = 0;
	virtual void off_selected() = 0;
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev)=0;
	virtual bool keyboard_keyup(SDL_Event& ev)=0;
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr)=0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr)=0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr)=0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr)=0;
	
};
class SelectableViewport : public SelectablePane, public Mouseable{
public:	
	SelectableViewport(Viewport* port);
	virtual ~SelectableViewport();
	virtual bool keyboard_keydown(SDL_Event& ev){ return port->keyboard_keydown(ev); };
	virtual bool keyboard_keyup(SDL_Event& ev) { return port->keyboard_keyup(ev); }
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) {		
		port->mouse_buttondown(ev, ref);
		if(has_focus()){
			set_selected(true);
		}
		notify_mouse_buttondown(0, (void*)this);
		return true;
	}
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) {
		port->mouse_buttonup(ev, ref);
		set_selected(false);
		notify_mouse_buttonup(0, (void*)this);
		return true;
	}
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) { return port->mouse_motion(ev, ref); }
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) { return port->mouse_drag(ev, ref); }
	// Sprite Interface
	virtual void tick() { port->tick(); }
	virtual void update(SDL_Event& ev) { port->update(ev); }
	virtual void render(SDL_Renderer& ren) { port->render(ren); }
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) { port->render(ren, x, y, extent); }
	//IPane interface	
	virtual bool add_pane(IPane* pane){ return port->add_pane(pane); }
	virtual bool remove_pane(IPane* pane) { return port->remove_pane(pane); }
	virtual bool isvisible(){ return  port->isvisible(); }
	virtual void setvisible(bool value){ port->setvisible(value); }
	virtual void on_focus(){ port->on_focus(); }
	virtual void off_focus(){ port->off_focus(); }
	virtual bool has_focus(){ return port->has_focus(); }
	virtual void set_focus(bool value){ port->set_focus(value); }
	virtual void defocus_all_children(){ port->defocus_all_children(); }	
	//Selectable interface	
	virtual void on_selected(){}
	virtual void off_selected(){}
private:
	bool _selected;
	Viewport* port;
};

class Page : public SelectablePane, public Mouseable{
public:
	Page();
	virtual ~Page();
		
	virtual bool keyboard_keydown(SDL_Event& ev);
	virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	//IPane interface
	virtual void on_focus();
	virtual void off_focus();
	//Selectable interface	
	virtual void on_selected();
	virtual void off_selected();	
protected:
	SDL_Color selected_colour;
	SDL_Color focused_colour;
};

class Book : public SelectablePane, public Mouseable, public IMouseListener {
public:	
	Book(int flag=0);
	virtual ~Book();

	virtual bool keyboard_keydown(SDL_Event& ev);
	virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev) ;
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	// Pane interface
	virtual void on_focus();
	virtual void off_focus();
	// Selectable interface
	virtual void on_selected();
	virtual void off_selected();
	// IMouseListener interface
	virtual void update_on_mouse_motion(Mouseable* origin,int code, void* data);
	virtual void update_on_mouse_drag(Mouseable* origin,int code, void* data);
	virtual void update_on_mouse_buttondown(Mouseable* origin,int code, void* data);
	virtual void update_on_mouse_buttonup(Mouseable* origin,int code, void* data);	
	//variables
	int num_panes;
	Page pages[5];	
	SDL_Color selected_colour;
	SDL_Color focused_colour;
	Book* sub_book;	
	SelectablePane* selected_shit;
	Viewport _viewport;
	SelectableViewport viewport;
protected:	
};





// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------


class View_Play : public IView
{
public:
	// variables
	int disp_w;
	int disp_h;
	Timer* fps_timer;
	SDL_Surface* surface;	
	SDL_Texture* texture;
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;

	Collision mouse_intersect;
	Uint32 button;
	int mouse_x, mouse_y;

	// M Y  S T U F F   S T A R T	
	Mouse mouse;
	Book book;
	
	// M Y   S T U F F  E N D 

	// constructor and destructor
	View_Play(SDL_Window& win, SDL_Renderer& ren);
	~View_Play();
	// methods
	void on_start(SDL_Event& ev);
	void on_switch(SDL_Event& ev);
	void on_close(SDL_Event& ev);
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_events(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
};

