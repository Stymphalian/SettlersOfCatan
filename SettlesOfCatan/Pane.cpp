#include <list>
#include <SDL.h>
#include "Logger.h"
#include "Util.h"

#include "Coords.h"
#include "Collision.h"
#include "mouse.h"
#include "Sprite.h"
#include "Pane.h"
// ------------------------------------------------------------------
// I P A N E
// ------------------------------------------------------------------
IPane::IPane() 
: Sprite(),Focusable(),IMouseHandler(),IKeyboardHandler(){}
IPane::~IPane(){}

Coords* IPane::new_coords_from_ref(Coords* ref, Coords* target){
	if(ref != nullptr){
		_new_reference_coords.x(ref->x() - target->x());
		_new_reference_coords.y(ref->y() - target->y());
		return &_new_reference_coords;
	} else{
		return nullptr;
	}
}
// larger number means 'ontop'
// Order the panes first by z-order (largets to smallest)
// and then by relative distance to the top-left x,y corner
// sort the children panes by largest z index to smallest z-index
// answers the question does pane1 go before pane2?
bool IPane::compare_IPane(IPane* pane1, IPane* pane2){
	IPane* p1 = (IPane*)pane1;
	IPane* p2 = (IPane*)pane2;
	if(p1->coord().z() > p2->coord().z()){ return true; }
	else if(p1->coord().z() < p2->coord().z()){ return false; }

	int x = p1->coord().x();
	int y = p1->coord().y();
	int x2 = p2->coord().x();
	int y2 = p2->coord().y();
	if(sqrt(x*x + y*y) < sqrt(x2*x2 + y2*y2)) { return true; }

	return false;
}


// ------------------------------------------------------------------
// I W R A P P A N E
// ------------------------------------------------------------------
IWrapPane::IWrapPane(IPane* wrappee)
:IPane()
{
	this->wrappee = wrappee;
}
IWrapPane::~IWrapPane(){
	this->wrappee = nullptr;
}
bool IWrapPane::keyboard_keydown(SDL_Event& ev) { return this->wrappee->keyboard_keydown(ev); }
bool IWrapPane::keyboard_keyup(SDL_Event& ev) { return this->wrappee->keyboard_keyup(ev); }
bool IWrapPane::mouse_buttondown(SDL_Event& ev, Coords* ref){ return this->wrappee->mouse_buttondown(ev, ref); }
bool IWrapPane::mouse_buttonup(SDL_Event& ev, Coords* ref){ return  this->wrappee->mouse_buttonup(ev, ref); }
bool IWrapPane::mouse_motion(SDL_Event& ev, Coords* ref){ return this->wrappee->mouse_motion(ev, ref); }
bool IWrapPane::mouse_drag(SDL_Event& ev, Coords* ref){ return this->wrappee->mouse_drag(ev, ref); }
void IWrapPane::tick() { this->wrappee->tick(); }
void IWrapPane::update(SDL_Event& ev) { this->wrappee->update(ev); }
void IWrapPane::render(SDL_Renderer& ren){ this->render(ren); }
void IWrapPane::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){ this->wrappee->render(ren, x, y, extent); }
bool IWrapPane::add_pane(IPane* pane){ return this->add_pane(pane); }
bool IWrapPane::remove_pane(IPane* pane) { return this->remove_pane(pane); }
bool IWrapPane::isvisible() { return this->wrappee->isvisible(); }
void IWrapPane::setvisible(bool value) { this->wrappee->setvisible(value); }
void IWrapPane::on_focus() { this->wrappee->on_focus(); }
void IWrapPane::off_focus(){ this->wrappee->off_focus(); }
bool IWrapPane::has_focus() { return this->wrappee->has_focus(); }
void IWrapPane::set_focus(bool value) { this->wrappee->set_focus(value); }
bool IWrapPane::is_selected() { return this->wrappee->is_selected(); }
void IWrapPane::set_selected(bool value){ this->wrappee->set_selected(value); }
void IWrapPane::on_selected() { this->wrappee->on_selected(); }
void IWrapPane::off_selected(){ this->wrappee->off_selected(); }
bool IWrapPane::isactive(){ return this->wrappee->isactive(); }
void IWrapPane::setactive(bool value){ this->wrappee->setactive(value); }
void IWrapPane::set_background(SDL_Color colour){ this->wrappee->set_background(colour); }
SDL_Color IWrapPane::get_background(){ return this->wrappee->get_background(); }
void IWrapPane::render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent) { this->wrappee->render_children(ren, x, y, extent); }
void IWrapPane::determine_focused_child_pane(IMouse* mouse) { this->wrappee->determine_focused_child_pane(mouse); }
IPane* IWrapPane::get_focused_child_pane(){ return this->wrappee->get_focused_child_pane(); }
void IWrapPane::defocus_all_children(){ this->wrappee->defocus_all_children(); }


// ------------------------------------------------------------------
// P A N E
// ------------------------------------------------------------------
Pane::Pane() : IPane()
{
	this->children.clear();
	this->_background = { 140, 140, 140, 80 };
	this->_focused_child_pane = &NULL_Pane::get();
	this->_visible_flag = false;
	this->_focus_flag = false;
	this->_active_flag = true;
}
Pane::~Pane()
{
	this->children.clear();
	this->_background = { 0, 0, 0, 0 };
	this->_focused_child_pane = nullptr;
}


// Sprite Interface 
// Focusable Interface
// KeyboardHandler Interface	
// Mousehandler Interface
// IPane Interface
bool Pane::isactive(){ return _active_flag; }
void Pane::setactive(bool value){ _active_flag = value; }
bool Pane::add_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it = children.begin();
	for(it = children.begin(); it != children.end(); ++it){
		if((*it) == pane){ return false; }
	}

	// add the pane in the appropriate place
	bool added = false;
	for(it = children.begin(); it != children.end(); ++it){
		if( IPane::compare_IPane(pane, *it) ){
			children.insert(it,pane);
			added = true;
			break;
		} 
	}
	if(added == false){
		children.push_back(pane);
	}
	
	pane->coord().set_parent(&this->_coord);
	return true;
}

bool Pane::remove_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it;
	for(it = children.begin(); it != children.end(); ++it){
		if((*it) == pane) {
			(*it)->coord().set_parent(nullptr);
			children.erase(it);
			return true;
		}
	}
	return false;
}

void Pane::set_background(SDL_Color colour){ _background = colour; }
SDL_Color Pane::get_background(){ return _background; }

// x,y are the display coordinates that 'this' pane was drawn at.
// extent is the region in which 'this' pane is allowed to draw.
void Pane::render_children(SDL_Renderer& ren,int x,int y, SDL_Rect* extent){
	if(extent == nullptr){return;}

	SDL_Rect intersection;		
	SDL_Rect B;
	std::list<IPane*>::reverse_iterator it;
	for(it = children.rbegin(); it != children.rend(); ++it){
		if((*it)->isvisible() == false){ continue; }
		// B is the full extent of the child
		// coordinates are relative to 'this'.
		B = {
			(*it)->coord().x(),
			(*it)->coord().y(),
			(*it)->coord().w(),
			(*it)->coord().h()
		};
		
		// if B collides with extent, we know that 
		// atleast a part of B is visible in extent.
		if(Collision::intersect_rect_rect(&B, extent, &intersection)){

			// the draw coordinates for the child object 
			// given that possibly only a partial amount 
			// of the object will be drawn.
			int draw_x = intersection.x + x - extent->x;
 			int draw_y = intersection.y + y - extent->y;			
			intersection.x -= (*it)->coord().x(),
			intersection.y -= (*it)->coord().y(),			
			(*it)->render(
				ren,
				draw_x,
				draw_y,
				&intersection);
		}	
	}	
}
void Pane::determine_focused_child_pane(IMouse* rel_mouse){
	if(rel_mouse == nullptr){ return; }
	//if(_focused_child_pane->hitbox().collides(rel_mouse->hitbox())){
	//	// the currently focused_child still has focus.
	//	return;
	//}

	std::list<IPane*>::iterator it;
	bool done = false;
	for(it = children.begin(); it != children.end(); ++it){
		// make sure that the pane is 'active'
		if((*it)->isactive() == false){ continue; }

		if((*it)->hitbox().collides(rel_mouse->hitbox())){
			//the pane is the currently in focus pane
			if(*it == _focused_child_pane){ 
				done = true;
				break; 
			}

			// a new pane has gained focus.
			_focused_child_pane->set_focus(false);
			_focused_child_pane = *it;
			_focused_child_pane->set_focus(true);			
			done = true;
			break;
		}
	}

	// no panes were selected.
	if(done == false){
		_focused_child_pane->set_focus(false);
		_focused_child_pane = &NULL_Pane::get();
	}
}

IPane* Pane::get_focused_child_pane(){		
	return _focused_child_pane;
}

void Pane::defocus_all_children() {
	_focused_child_pane->defocus_all_children();
	_focused_child_pane->set_focus(false);
	_focused_child_pane = &NULL_Pane::get();
}

