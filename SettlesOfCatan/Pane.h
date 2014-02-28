#pragma once
#include <list>
#include <SDL.h>

#include "Coords.h"
#include "Collision.h"
#include "mouse.h"
#include "Sprite.h"
#include "Focusable.h"
#include "Selectable.h"
#include "MouseHandler.h"
#include "KeyboardHandler.h"
class IWrapPane;

// -----------------------------------------
// I P A N E
// -----------------------------------------
class IPane:
	public Sprite,
	public Focusable,
	public Selectable,
	public IMouseHandler,
	public IKeyboardHandler
{
public:
	IPane();	
	virtual ~IPane();
	// Sprite Interface
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	// Focusable Interface
	virtual void on_focus() = 0;
	virtual void off_focus() = 0;
	// Selectable Interface
	virtual void on_selected() = 0;
	virtual void off_selected() = 0;
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev) = 0;
	virtual bool keyboard_keyup(SDL_Event& ev) = 0;
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) = 0;	
	//IPane interface
	virtual bool isactive()=0;
	virtual void setactive(bool value)=0;
	virtual bool add_pane(IPane* pane) = 0;
	virtual bool remove_pane(IPane* pane) = 0;		
	virtual void set_background(SDL_Color colour)=0;		
	virtual SDL_Color get_background()=0;	
	virtual void defocus_all_children() = 0;
protected:	
	// IPane Interface
	friend class IWrapPane;	
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent=nullptr) = 0;
	virtual void determine_focused_child_pane(IMouse* mouse) = 0;		
	virtual IPane* get_focused_child_pane() = 0;	

	//convenience method
	Coords _new_reference_coords;
	Coords* new_coords_from_ref(Coords* ref,Coords* target){
		if(ref != nullptr){
			_new_reference_coords.x(ref->x() - target->x());
			_new_reference_coords.y(ref->y() - target->y());			
			return &_new_reference_coords;
		}else{
			return nullptr;
		}				
	}
};

// -----------------------------------------
// I W R A P P A N E
// -----------------------------------------
class IWrapPane : public IPane	
{
public:
	IWrapPane(IPane* wrappee);	
	virtual ~IWrapPane();
	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	virtual bool isvisible();
	virtual void setvisible(bool value);
	//virtual Coords& coord();
	//virtual Collision& hitbox();
	// Focusable Interface	
	virtual bool has_focus();
	virtual void set_focus(bool value);	
	virtual void on_focus();
	virtual void off_focus();
	// Selected Interface
	virtual void on_selected();
	virtual void off_selected();	
	virtual bool is_selected();
	virtual void set_selected(bool value);
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev);
	virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	//IPane interface
	virtual bool isactive();
	virtual void setactive(bool value);
	virtual bool add_pane(IPane* pane);
	virtual bool remove_pane(IPane* pane);
	virtual void set_background(SDL_Color colour);
	virtual SDL_Color get_background();
	virtual void defocus_all_children();
protected:	
	IPane* wrappee;	
	// IPane Interface
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent = nullptr);
	virtual void determine_focused_child_pane(IMouse* mouse);
	virtual IPane* get_focused_child_pane();
};

// -----------------------------------------
// P A N E
// -----------------------------------------
class Pane : public IPane{
public:
	Pane();
	virtual ~Pane();
	// Sprite Interface
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	// Focusable Interface
	virtual void on_focus() = 0;
	virtual void off_focus() = 0;
	// Selectable Interface
	virtual void on_selected() = 0;
	virtual void off_selected() = 0;
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev) = 0;
	virtual bool keyboard_keyup(SDL_Event& ev) = 0;
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) = 0;
	//IPane interface
	virtual bool isactive();
	virtual void setactive(bool value);
	virtual bool add_pane(IPane* pane);
	virtual bool remove_pane(IPane* pane);
	virtual void set_background(SDL_Color colour);
	virtual SDL_Color get_background();
	virtual void defocus_all_children() ;
protected:		
	//IPane Interface	
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent = nullptr);
	virtual void determine_focused_child_pane(IMouse* mouse);
	virtual IPane* get_focused_child_pane();
	
	// Pane variables and methods
	std::list<IPane*> children;		
	void pack();
	bool operator< (IPane* pane2);	
private:		
	// Pane variables	
	SDL_Color _background;	
	IPane* _focused_child_pane;		
	bool _active_flag;
};



// -----------------------------------------
// N U L L _ P A N E
// -----------------------------------------
class NULL_Pane : public IPane{
public:
	static NULL_Pane& get(){
		static NULL_Pane instance;
		return instance;
	}
	static bool isDummy(void* object){
		return (object == &NULL_Pane::get());
	}
	// Sprite Interface
	virtual void tick() {}
	virtual void update(SDL_Event& ev){}
	virtual void render(SDL_Renderer& ren){}
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) {}
	// Focusable Interface
	virtual void on_focus(){}
	virtual void off_focus(){}
	// Selectable Interface
	virtual void on_selected(){}
	virtual void off_selected(){}
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev){ return false; }
	virtual bool keyboard_keyup(SDL_Event& ev) { return false; }
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) { return false; }
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) { return false; }
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr){ return false; }
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) { return false; }
	//IPane interface
	virtual bool isactive() { return false; }
	virtual void setactive(bool value){}
	virtual bool add_pane(IPane* pane) { return false; }
	virtual bool remove_pane(IPane* pane) { return false; }
	virtual void set_background(SDL_Color colour) {}
	virtual SDL_Color get_background() {
		static SDL_Color colour = { 0, 0, 0, 0 };
		return colour;
	}
	virtual void defocus_all_children(){}
protected:
	friend class IWrapPane;
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent = nullptr){}
	virtual void determine_focused_child_pane(IMouse* mouse) {}
	virtual IPane* get_focused_child_pane() {return nullptr;}
private:
	NULL_Pane(){}
	virtual ~NULL_Pane(){}
	NULL_Pane(const NULL_Pane&);
	void operator= (NULL_Pane const&);
};