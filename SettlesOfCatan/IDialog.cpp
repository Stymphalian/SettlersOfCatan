#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "IDialog.h"
#include "Collision.h"
#include "IView.h"
#include "View_Game.h"
#include "Button.h"
#include "checkBox.h"


// ----------------------------------------------------
//		I D I A L O G
// ----------------------------------------------------
IDialog::IDialog(IView& view,int x, int y, int z, int w, int h) 
: view(view)
{
	Logger::getLog().log(Logger::DEBUG, "IDialog constructor");
	this->modal = true;
	this->x = x; 
	this->y = y;
	this->z = z;
	this->w = w;
	this->h = h;
	
	hitbox.hook(&this->x, &this->y, &this->z);
	hitbox.add_rect(0, 0, 0, this->w, this->h);

	// position the mouse
	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_buttons = 0;
	_mouse_hitbox.hook(&_mouse_x, &_mouse_y);
	_mouse_hitbox.add_rect(0, 0, 0, 1, 1 );
	
	// set the clip rects.
	SDL_RenderGetClipRect(&view.ren, &old_clip);
	new_clip = { this->x, this->y, this->w, this->h };
	_visible = false;
}
IDialog::~IDialog(){
	Logger::getLog().log(Logger::DEBUG, "IDialog destructor");
	_mouse_hitbox.clear();
	hitbox.clear();
}
bool IDialog::isvisible(){return _visible;}
bool IDialog::open(void* data){ _visible = true; return true; }
void* IDialog::close(){
	_visible = false;
	return nullptr;	
}