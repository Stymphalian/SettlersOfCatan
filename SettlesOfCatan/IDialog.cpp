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
	this->modal = true;
	this->x = x; 
	this->y = y;
	this->z = z;
	this->w = w;
	this->h = h;
	
	hitbox.hook(&this->x, &this->y, &this->z);
	hitbox.add_rect(0, 0, 0, this->w, this->h);
	
	SDL_RenderGetClipRect(&view.ren, &old_clip);
	new_clip = { this->x, this->y, this->w, this->h };
	_visible = false;
}
IDialog::~IDialog(){}
bool IDialog::isvisible(){return _visible;}
bool IDialog::open(void* data){ _visible = true; return true; }
void* IDialog::close(){ _visible = false; return nullptr; }