#include <list>
#include <SDL.h>
#include "Logger.h"
#include "Util.h"

#include "Coords.h"
#include "Collision.h"
#include "mouse.h"
#include "Sprite.h"
#include "Pane.h"


Pane::Pane() : IPane()
{
	this->mouse = new relMouse(new Mouse(),&this->coord);
	this->children.clear();
	background = { 140, 140, 140, 80 };
}
Pane::~Pane()
{
	delete this->mouse;
	this->mouse = nullptr;

	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		(*it)->coord.set_parent(this->coord.get_parent());
	}
	this->children.clear();
}

void Pane::pass_mouse_child(SDL_Event& ev){ 
	this->mouse->update(); // this might be slow...
	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		if(this->mouse->hitbox().collides((*it)->hitbox))
		{
			(*it)->handle_mouse_events(ev);
			break;
		}
	}	
}
void Pane::pass_keyboard_child(SDL_Event& ev){	
	this->mouse->update(); // this might be slow..
	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		if(this->mouse->hitbox().collides((*it)->hitbox))
		{
			(*it)->handle_mouse_events(ev);
			break;
		}
	}
}
void Pane::render_children(SDL_Renderer& ren){
	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		(*it)->render(ren);
	}
}


// Sprite interface
void Pane::tick(){}



// Pane methods
void Pane::init(int x, int y, int z, int w, int h){
	coord.init(x, y, z, w, h);
}
void Pane::set_dim(int w, int h){
	coord.w(w);
	coord.h(h);
}
void Pane::set_background(SDL_Color c){
	background = c;
}


// IPane interface
bool Pane::add_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it = children.begin();
	for(it = children.begin(); it != children.end(); ++it){
		if((*it) == pane){ return false; }
	}
	children.push_back(pane);
	pane->coord.set_parent(&this->coord);
	pack();
	return true;
}
bool Pane::remove_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		if((*it) == pane) {
			(*it)->coord.set_parent(nullptr);
			children.erase(it);
			pack();
			return true;
		}
	}
	return false;
}

// Pane methods
void Pane::pack(){
	children.sort();
}
// larger number means 'ontop'
// Order the panes first by z-order (largets to smallest)
// and then by relative distance to the top-left x,y corner
// sort the children panes by largest z index to smallest z-index
bool Pane::operator< (IPane* pane2){
	IPane* p1 = (IPane*)this;
	IPane* p2 = (IPane*)pane2;
	if(p1->coord.z() > p2->coord.z()){ return true; }

	int x = p1->coord.x();
	int y = p1->coord.y();
	int x2 = p2->coord.x();
	int y2 = p2->coord.y();
	if(sqrt(x*x + y*y) < sqrt(x2*x2 + y2*y2)) { return true; }

	return false;
}