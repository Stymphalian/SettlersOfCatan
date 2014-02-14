#pragma once
#include <list>
#include <SDL.h>

#include "Sprite.h"
#include "Coords.h"
#include "Collision.h"
#include "mouse.h"

class IPane : public Sprite{
public:
	IPane() : Sprite() {}
	virtual ~IPane(){}
	virtual bool handle_keyboard_events(SDL_Event& ev) = 0;
	virtual bool handle_mouse_events(SDL_Event& ev) = 0;
	virtual void pass_mouse_child(SDL_Event& ev)= 0;
	virtual void pass_keyboard_child(SDL_Event& ev) = 0;
	virtual void render_children(SDL_Renderer& ren) = 0;

	// Sprite Interface
	virtual void tick()=0;
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;
protected:
	virtual bool add_pane(IPane* pane) = 0;
	virtual bool remove_pane(IPane* pane) = 0;
};

class Pane : public IPane{
public:
	IMouse* mouse;

	Pane();
	virtual ~Pane();
	// IPane interface
	virtual bool handle_keyboard_events(SDL_Event& ev) = 0;
	virtual bool handle_mouse_events(SDL_Event& ev) = 0;
	void pass_mouse_child(SDL_Event& ev);
	void pass_keyboard_child(SDL_Event& ev);
	void render_children(SDL_Renderer& ren);
	

	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev) = 0;
	virtual void render(SDL_Renderer& ren) = 0;

	// Pnae methods	
	void init(int x, int y, int z, int w, int h);	
	void set_dim(int w, int h);	
	void set_background(SDL_Color c);
protected:
	// variables	
	std::list<IPane*> children;

	//  IPane interface
	bool add_pane(IPane* pane);
	bool remove_pane(IPane* pane);
	// Pane methods
	void pack();
	bool operator< (IPane* pane2);
private:	
	SDL_Color background;
};