
#include <SDL.h>
#include "Logger.h"
#include "Collision.h"
#include "Coords.h"
#include "mouse.h"

//-------------------------------------------------------
// I W R A P M O U S E
//-------------------------------------------------------
IWrapMouse::IWrapMouse(IMouse* w)
:IMouse(), wrapee(w)
{}

bool IWrapMouse::left() {
	return (wrapee != nullptr) ? wrapee->left() : false;
}
bool IWrapMouse::right() {
	return (wrapee != nullptr) ? wrapee->right() : false;
}
bool IWrapMouse::middle() {
	return (wrapee != nullptr) ? wrapee->middle() : false;
}
int IWrapMouse::x() {
	return (wrapee != nullptr) ? wrapee->x() : 0;
}
int IWrapMouse::y() {
	return (wrapee != nullptr) ? wrapee->y() : 0;
}
int IWrapMouse::x(int value) {
	return (wrapee != nullptr) ? wrapee->x(value) : 0;
}
int IWrapMouse::y(int value) {
	return (wrapee != nullptr) ? wrapee->y(value) : 0;
}
Collision& IWrapMouse::hitbox(){
	static Collision h;
	return (wrapee != nullptr) ? wrapee->hitbox() : h;
}

//-------------------------------------------------------
// G M O U S E
//-------------------------------------------------------
IMouse& GMouse::get(){
	static Mouse instance;
	return instance;
}
IMouse& GMouse::get(ICoords* offset){
	static Mouse mouse_instance;
	static relativeMouse instance(&mouse_instance);
	if(offset == nullptr){
		return GMouse::get();
	}
	instance.set_offset(offset);
	instance.update();
	return instance;
}
IMouse& GMouse::get(int xoff, int yoff){
	static Mouse mouse_instance;
	static relativeMouse instance(&mouse_instance);
	instance.set_offset(xoff, yoff);
	instance.update();
	return instance;
}

//-------------------------------------------------------
// M O U S E
//-------------------------------------------------------
Mouse::Mouse()
: IMouse()
{
	Logger::getLog().log(Logger::DEBUG, "Mouse construcotr");
	_hitbox.hook(&mouse_x, &mouse_y);
	_hitbox.add_rect(0, 0, 0, 1, 1);	
	this->mouse_x = 0;
	this->mouse_y = 0;
	this->buttons = 0;
}
Mouse::~Mouse(){ 
	Logger::getLog().log(Logger::DEBUG, "Mouse destructor");
	_hitbox.clear();
}
void Mouse::update(){ buttons = SDL_GetMouseState(&mouse_x, &mouse_y); }
bool Mouse::left(){return ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0); }
bool Mouse::right(){return ((buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0); }
bool Mouse::middle(){return ((buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) !=0); }
int Mouse::x(){ return mouse_x; }
int Mouse::y(){ return mouse_y; }
int Mouse::x(int value){ return mouse_x=value; }
int Mouse::y(int value){ return mouse_y=value; }
Collision& Mouse::hitbox(){ return this->_hitbox; }




// -----------------------------------------------
// R E L A T I V E M O U S E
// -----------------------------------------------
relativeMouse::relativeMouse(IMouse* wrappee)
: IWrapMouse(wrappee)
{
	type = relativeMouse::NONE;
	coord = nullptr;
}
relativeMouse::~relativeMouse(){
	type = relativeMouse::NONE;
	coord = nullptr;
}
void relativeMouse::update(){
	if(wrapee == nullptr){ return; }
	wrapee->update();
	if(type == relativeMouse::COORD){
		if(coord != nullptr){
			wrapee->x(abs(wrapee->x() - coord->disp_x()));
			wrapee->y(abs(wrapee->y() - coord->disp_y()));
		}
	} else if( type == relativeMouse::X_OFF) {
		wrapee->x(wrapee->x() - xoff);
		wrapee->y(wrapee->y() - yoff);
	}
}
void relativeMouse::set_offset(ICoords* coord){
	type = relativeMouse::COORD;
	this->coord = coord;
}
void relativeMouse::set_offset(int xoff, int yoff){
	type = relativeMouse::X_OFF;
	this->xoff = xoff;
	this->yoff = yoff;
}


//-------------------------------------------------------
// R E L M O U S E
//-------------------------------------------------------
relMouse::relMouse(IMouse* m, Coords* offset)
: IWrapMouse(m)
{		
	this->coord = offset;
	type = relMouse::COORD;	
}
relMouse::relMouse(IMouse* m, int xoffset, int yoffset)
: IWrapMouse(m)
{
	this->xoff = xoffset;
	this->yoff = yoffset;
	type = relMouse::X_OFF;
}
relMouse::~relMouse(){
	Logger::getLog().log(Logger::DEBUG, "relMouse destructor");
	delete wrapee;	
}

void relMouse::update(){	
	if(wrapee == nullptr){ return; }
	wrapee->update();
	if(type == relMouse::COORD){				
		if(coord != nullptr){
			wrapee->x(abs(wrapee->x() - coord->disp_x()));
			wrapee->y(abs(wrapee->y() - coord->disp_y()));			
		}
	} else{
		wrapee->x(wrapee->x() - xoff);
		wrapee->y(wrapee->y() - yoff);		
	}
}

