#pragma once

#include <SDL.h>
#include <string>
#include "Sprite.h"
#include "Collision.h"

class IButton{
public:
	typedef void* (button_action)(IButton* origin, void* data);

	IButton(){}
	virtual ~IButton(){}
	virtual bool has_focus()=0;
	virtual bool is_selected()=0;
	virtual void set_focus(bool value)=0;
	virtual void set_selected(bool value)=0;

	virtual void on_focus()=0;
	virtual void off_focus()=0;
	virtual void on_click()=0;
	virtual void off_click()=0;
	virtual void* action(IButton* origin, void* data) = 0;
	virtual void set_action(button_action) = 0;
};

class Button : public IButton{
	enum frames {MOUSEOUT,PRESSED, NOTPRESSED, MOUSEOVER, num_frames };
	
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
	void init(const char* text, int x, int y,int z, int w, int h);
	void set_pad(int padx, int pady);

	// IButton interface
	virtual bool has_focus(){ return false; }
	virtual bool is_selected(){ return false; }
	virtual void set_focus(bool value){}
	virtual void set_selected(bool value){}
	virtual void on_focus(){}
	virtual void off_focus(){}
	virtual void on_click(){}
	virtual void off_click(){}
	virtual void* action(IButton* origin, void* data){ return nullptr; }
	virtual void set_action(button_action){}
private:
};