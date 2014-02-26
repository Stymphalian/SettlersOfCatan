#pragma once
#include <list>
#include <SDL.h>

#include "Sprite.h"
#include "Coords.h"
#include "Collision.h"
#include "mouse.h"
#include "MouseHandler.h"
#include "KeyboardHandler.h"
class IWrapPane;


class IPane : public Sprite,IMouseHandler,IKeyboardHandler{
public:
	IPane();
	IPane(int x, int y, int z, int w, int h);
	virtual ~IPane();

	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev) = 0;
	virtual bool keyboard_keyup(SDL_Event& ev) = 0;
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) = 0;
	// Sprite Interface
	virtual void tick() = 0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;	
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;	

	//IPane interface
	virtual bool add_pane(IPane* pane) = 0;
	virtual bool remove_pane(IPane* pane) = 0;		
	virtual bool isvisible()= 0;
	virtual void setvisible(bool value)=0;
	virtual void on_focus()=0;
	virtual void off_focus()=0;	
	virtual bool has_focus()=0;	
	virtual void set_focus(bool value) = 0;
	virtual void set_background(SDL_Color colour)=0;		
	virtual SDL_Color get_background()=0;	
	virtual void defocus_all_children() = 0;
protected:
	friend class IWrapPane;	
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent=nullptr) = 0;
	virtual void determine_focused_child_pane(IMouse* mouse) = 0;		
	virtual IPane* get_focused_child_pane() = 0;
	virtual bool isDummy(IPane* pane) = 0;
};

class IWrapPane : public IPane{
public:
	IWrapPane(IPane* wrappee);
	virtual ~IWrapPane();

	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev);
	virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) = 0;
	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	//IPane interface
	virtual bool add_pane(IPane* pane);
	virtual bool remove_pane(IPane* pane);
	virtual bool isvisible();
	virtual void setvisible(bool value);
	virtual void on_focus();
	virtual void off_focus();
	virtual bool has_focus();
	virtual void set_focus(bool value);	
	virtual void defocus_all_children();
	virtual void set_background(SDL_Color colour);
	virtual SDL_Color get_background();		
protected:
	IPane* wrappee;
	friend class IWrapPane;
	//IPane Interface	
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent=nullptr);
	virtual void determine_focused_child_pane(IMouse* mouse);		
	virtual IPane* get_focused_child_pane() ;	
	virtual bool isDummy(IPane* pane);
};


class Pane : public IPane{
public:
	Pane();
	virtual ~Pane();

	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev)=0;
	virtual bool keyboard_keyup(SDL_Event& ev)=0;
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr) = 0;
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) = 0;
	// Sprite Interface
	virtual void tick()=0;	
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren)=0;
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) = 0;
	//IPane interface	
	virtual bool add_pane(IPane* pane);
	virtual bool remove_pane(IPane* pane);
	virtual bool isvisible();
	virtual void setvisible(bool value);	
	virtual void on_focus();
	virtual void off_focus();
	virtual void set_focus(bool value);
	virtual bool has_focus();
	virtual void set_background(SDL_Color colour);
	virtual SDL_Color get_background();	
	virtual IPane* get_focused_child_pane();
protected:	
	//IPane Interface	
	virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent=nullptr);
	virtual void determine_focused_child_pane(IMouse* mouse);		
	virtual void defocus_all_children();
	virtual bool isDummy(IPane* pane);
	
	// Pane variables		
	std::list<IPane*> children;	
	// Pane methods
	void pack();
	bool operator< (IPane* pane2);	
private:		
	// Private class implementing a NULL object
	class NullPane : public IPane{
	public:
		NullPane() : IPane(){};
		virtual ~NullPane(){}
		bool keyboard_keydown(SDL_Event& ev){ return false; }
		bool keyboard_keyup(SDL_Event& ev){ return false; }
		// MouseHandler interface
		bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr){ return false; }
		bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr){ return false; }
		bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr){ return false; }
		bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr) { return false; }
		// Sprite Interface
		void tick(){}
		void update(SDL_Event& ev){}
		void render(SDL_Renderer& ren){ }
		void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){}
		//IPane interface
		bool add_pane(IPane* pane){ return false; }
		bool remove_pane(IPane* pane){ return false; }
		bool isvisible(){ return false; }
		void setvisible(bool value){}
		void on_focus(){}
		void off_focus(){}		
		bool has_focus(){ return false; }
		void set_focus(bool value){}		
		void set_background(SDL_Color colour){}
		SDL_Color get_background(){
			static SDL_Color bcolour = { 0, 0, 0, 0 };
			return bcolour;
		}
		virtual void defocus_all_children(){}
	protected:
		//IPane Interface		
		virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent=nullptr){};
		virtual void determine_focused_child_pane(IMouse* mouse){}		
		virtual IPane* get_focused_child_pane(){ return nullptr; }
		virtual bool isDummy(IPane* pane) { return false; }		
	};

	// Pane variables	
	SDL_Color _background;
	bool _visible;
	bool _focus;	
	IPane* _focused_child_pane;	
	NullPane nullPane;	
};