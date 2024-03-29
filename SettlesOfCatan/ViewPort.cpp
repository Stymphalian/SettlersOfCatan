#include <cmath>
#include "Util.h"
#include "Pane.h"
#include "Coords.h"
#include "ViewPort.h"


//Viewport iterator
Viewport::iterator::iterator(){
	this->x = 0;
	this->width=0;
	this->wrapping = false;
}
Viewport::iterator::~iterator(){
	this->x = 0;
	this->width = 0;
	this->wrapping = false;
}
Viewport::iterator::iterator(int x, int width, bool wrapping){
	this->x = x;
	this->width = width;
	this->wrapping = wrapping;
}
Viewport::iterator& Viewport::iterator::operator++(){
	this->x++;
	make_within_bounds();
	return *this;
}
Viewport::iterator Viewport::iterator::operator++(int stuff){
	iterator prev_self = *this;
	this->x++;
	make_within_bounds();
	return prev_self;
}
int& Viewport::iterator::operator*(){ return x; }
//int* Viewport::iterator::operator->() { return &x; }
bool Viewport::iterator::operator==(const iterator& rhs){ return x == rhs.x; }
bool Viewport::iterator::operator!=(const iterator& rhs){ return x != rhs.x; }
void Viewport::iterator::make_within_bounds(){
	if(wrapping){
		if(x >= width){ x = x%width; }
	}
}

// Viewport
Viewport::Viewport() : IWrapPane(nullptr),IViewport(){
	this->coord().init(0, 0, 0, 0, 0);
	this->camera_coords.init(0, 0, 0, 0, 0);
	this->viewport_coord().init(0, 0, 0, 0, 0);

	this->target = nullptr;
	this->draw_rect = { 0, 0, 0, 0 };
	this->draw_rect_unmod = { 0, 0, 0, 0 };
	this->_pixels_per_xunit = 0;
	this->_pixels_per_yunit = 0;
	this->_xunits_per_pixel = 0;
	this->_yunits_per_pixel = 0;	
	this->_horiz_wrap = false;
	this->_vert_wrap = false;
	this->_visible = true;
	this->_focus = false;	


	this->_viewport_children.clear();
	this->_focused_viewport_child_pane = &NULL_Pane::get();
	//this->_passing_ref_coords;
}
Viewport::~Viewport(){
	this->target = nullptr;
	this->_pixels_per_xunit = 0;
	this->_pixels_per_yunit = 0;
	this->_xunits_per_pixel = 0;
	this->_yunits_per_pixel = 0;	
}

bool Viewport::keyboard_keydown(SDL_Event& ev){
	if(has_focus()){
		const Uint8* keyboard = SDL_GetKeyboardState(NULL);		
		if(keyboard[SDL_SCANCODE_W]){ move_camera_px_y(-1); }
		if(keyboard[SDL_SCANCODE_D]){ move_camera_px_x(1); }
		if(keyboard[SDL_SCANCODE_S]){ move_camera_px_y(1); }
		if(keyboard[SDL_SCANCODE_A]){ move_camera_px_x(-1); }		
	}
	this->wrappee->keyboard_keydown(ev);
	return true;
}
//bool Viewport::keyboard_keyup(SDL_Event& ev);
	  
Coords& Viewport::determine_passing_reference_coords(Coords* reference){	
	int relx, rely;
	if(reference != nullptr){
		relx = reference->x();
		rely = reference->y();
	} else{
		IMouse& rel_mouse = GMouse::get(&viewport_coord());
		rel_mouse.update();
		relx = rel_mouse.x();
		rely = rel_mouse.y();		
	}
			
	_passing_ref_coords.x(camera_coords.x() + relx);
	_passing_ref_coords.y(camera_coords.y() + rely);
	return _passing_ref_coords;
}
bool Viewport::mouse_buttondown(SDL_Event& ev, Coords* ref){
	determine_passing_reference_coords(ref);
	this->wrappee->mouse_buttondown(ev, &_passing_ref_coords);
	return true;
}
bool Viewport::mouse_buttonup(SDL_Event& ev, Coords* ref){
	determine_passing_reference_coords(ref);
	this->wrappee->mouse_buttonup(ev, &_passing_ref_coords);
	return true;
}
bool Viewport::mouse_motion(SDL_Event& ev, Coords* ref){
	determine_passing_reference_coords(ref);
	this->wrappee->mouse_motion(ev, &_passing_ref_coords);
	return true;
}
bool Viewport::mouse_drag(SDL_Event& ev, Coords* ref){
	determine_passing_reference_coords(ref);
	this->wrappee->mouse_drag(ev, &_passing_ref_coords);
	return true;
}
	  
//void Viewport::tick();
//void Viewport::update(SDL_Event& ev);
void Viewport::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0,0,viewport_coord().w(),viewport_coord().h()
	};
	render(ren, viewport_coord().disp_x(),viewport_coord().disp_y(),&rect);
}
void Viewport::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	SDL_Rect old_clip;
	SDL_Rect new_clip = {
		x ,//+ extent->x,
		y ,//+ extent->y,
		extent->w,
		extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);

	SDL_Rect target_extent = {
		camera_coords.x() + extent->x,
		camera_coords.y() + extent->y,
		extent->w,
		extent->h
	};
	this->wrappee->render(ren, x, y, &target_extent);
	this->render_viewport_children(ren, x, y, extent);

	SDL_Rect rect = {
		x - extent->x,
		y - extent->y,
		viewport_coord().w(),
		viewport_coord().h()
	};
	if(has_focus()){
		Util::render_rectangle(&ren, &rect, Util::colour_orange());
	} else{
		Util::render_rectangle(&ren, &rect, Util::colour_black());
	}
	SDL_RenderSetClipRect(&ren, &old_clip);
}
	  
//bool Viewport::add_pane(IPane* pane);
//bool Viewport::remove_pane(IPane* pane);
//bool Viewport::isvisible(){return _visible && wrappee->isvisible();}
//void Viewport::setvisible(bool value){
//	_visible = value;
//	wrappee->setvisible(value);
//}
void Viewport::on_focus(){
	this->wrappee->on_focus();
}
void Viewport::off_focus(){
	this->wrappee->off_focus();
	defocus_all_viewport_children();
}
bool Viewport::has_focus(){
	return _focus && wrappee->has_focus();
}
void Viewport::set_focus(bool value){	
	if(value == true){
		if(_focus == false){ on_focus(); }
	} else{
		if(_focus == true){ off_focus(); }
	}
	_focus = value;
	wrappee->set_focus(value);
}

void Viewport::defocus_all_children(){
	wrappee->defocus_all_children();
	wrappee->set_focus(false);
}
//void Viewport::set_background(SDL_Color colour);
//SDL_Color Viewport::get_background();




//---------------------------------------------
// IViewport Interface
//---------------------------------------------
void Viewport::set_target(IPane* target,unsigned ppxu,unsigned ppyu){
	this->wrappee = target;
	this->target = &wrappee->coord();
	set_pixels_per_xunit(ppxu);
	set_pixels_per_yunit(ppyu);
}
void Viewport::set_camera_coords(int x, int y, int w, int h){
	camera_coords.init(x, y, 0, w, h);
}
void Viewport::set_viewport_coords(int x, int y, int w, int h){
	coord().init(x, y,0, w, h);
	viewport_coord().init(x, y, 0, w, h);
}
void Viewport::set_pixels_per_yunit(unsigned pixels_per_unit){
	if(pixels_per_unit > 0){
		this->_pixels_per_yunit =  pixels_per_unit;
		this->_yunits_per_pixel = (unsigned) (1 / pixels_per_unit);
	} else{
		this->_pixels_per_yunit =1;
		this->_yunits_per_pixel = 1;
	}
}

void Viewport::set_pixels_per_xunit(unsigned pixels_per_unit){
	if(pixels_per_unit > 0){
		this->_pixels_per_xunit = pixels_per_unit;
		this->_xunits_per_pixel = (unsigned) (1 /pixels_per_unit);
	} else{
		this->_pixels_per_xunit = 1;
		this->_xunits_per_pixel = 1;
	}
}

int Viewport::viewport_px_x(){ return viewport_coord().x(); }
int Viewport::viewport_px_y(){ return viewport_coord().y(); }
int Viewport::viewport_px_w(){ return viewport_coord().w(); }
int Viewport::viewport_px_h(){ return viewport_coord().h(); }

int Viewport::camera_px_x(){return camera_coords.x();}
int Viewport::camera_px_y(){return camera_coords.y();}
int Viewport::camera_px_w(){return camera_coords.w();}
int Viewport::camera_px_h(){return camera_coords.h();}
 
int Viewport::target_unit_x(){return target->x();}
int Viewport::target_unit_y(){return target->y();}
int Viewport::target_unit_w(){return target->w();}
int Viewport::target_unit_h(){return target->h();}

void Viewport::set_camera_px_x(int amount){
	camera_coords.x(amount);
	make_cam_x_within_bounds();
}
void Viewport::set_camera_px_y(int amount){
	camera_coords.y(amount);
	make_cam_y_within_bounds();	
}
void Viewport::set_camera_px_w(int value){
	camera_coords.w(value);
	if(camera_coords.w() < 0){ camera_coords.w(0); }
}
void Viewport::set_camera_px_h(int value){
	camera_coords.h(value);
	if(camera_coords.h() < 0){ camera_coords.h(0); }
}
unsigned Viewport::pixels_per_xunit(){ return this->_pixels_per_xunit; }
unsigned Viewport::pixels_per_yunit(){ return this->_pixels_per_yunit; }
unsigned Viewport::xunits_per_pixel(){ return this->_xunits_per_pixel; }
unsigned Viewport::yunits_per_pixel(){ return this->_yunits_per_pixel; }

bool Viewport::camera_horiz_wrap(){ return _horiz_wrap; }
bool Viewport::camera_vert_wrap(){return _vert_wrap;};
void Viewport::set_camera_horiz_wrap(bool value) { _horiz_wrap = value; }
void Viewport::set_camera_vert_wrap(bool value) {_vert_wrap = value ; }

Viewport::iterator Viewport::begin_camera_x(){
	return Viewport::iterator(camera_unit_x(), target_unit_w(), _horiz_wrap);
}
Viewport::iterator Viewport::end_camera_x(){
	int cam_x = camera_col_unit_offset();
	if(cam_x >= target_unit_w() && _horiz_wrap){
		cam_x = cam_x %target_unit_w();
	}
	return Viewport::iterator(cam_x, target_unit_w(), _horiz_wrap);
}

Viewport::iterator Viewport::begin_camera_y(){
	return Viewport::iterator(camera_unit_y(), target_unit_h(), _vert_wrap);
}
Viewport::iterator Viewport::end_camera_y(){
	int cam_y = camera_row_unit_offset();
	if(cam_y >= target_unit_h() && _vert_wrap){
		cam_y = cam_y %target_unit_h();
	}
	return Viewport::iterator(cam_y, target_unit_h(), _vert_wrap);
}



// convenience methods
void Viewport::move_camera_px_x(int amount){
	set_camera_px_x(camera_coords.x() + amount);
}
void Viewport::move_camera_px_y(int amount){
	set_camera_px_y(camera_coords.y() + amount);	
}

int Viewport::camera_unit_x(){ return camera_coords.x() / (int)_pixels_per_xunit; }
int Viewport::camera_unit_y(){ return camera_coords.y() / (int)_pixels_per_yunit; }
int Viewport::camera_unit_w(){ return (int)ceil(camera_coords.w() / _pixels_per_xunit); }
int Viewport::camera_unit_h(){ return (int)ceil(camera_coords.h() / _pixels_per_yunit); }

SDL_Rect* Viewport::camera_unit_draw_rect_unmod(int col, int row){	
	
	// determine the x position of the draw rect.
	if(_horiz_wrap  && col < camera_unit_x()){
		// the x positin is the offset from the camera unit x position
		// because hozrizonal wrapping is 'on', and our 'col' position
		// is less than the camera_unit_x() position, we know that we have possibly
		// wrapped around the target map, therfore we must add target_unit_w pixels
		// to the draw_rect in order to compensate.
		draw_rect_unmod.x = (col + target_unit_w() - camera_unit_x())*(int)_pixels_per_xunit;		
	}else{
		// the x position is the offset from the camera unit x position
		draw_rect_unmod.x = (col - camera_unit_x())*(int)_pixels_per_xunit;				
	}
	// handle px offset between 'units
	draw_rect_unmod.x -= camera_coords.x() % (int)_pixels_per_xunit;
	draw_rect_unmod.x += viewport_coord().x();

	if(_vert_wrap && row < camera_unit_y()){
		draw_rect_unmod.y = (row + target_unit_h() - camera_unit_y())*(int)_pixels_per_yunit;
	} else{
		draw_rect_unmod.y = (row - camera_unit_y())*(int)_pixels_per_yunit;				
	}	
	draw_rect_unmod.y -= camera_coords.y() % (int)_pixels_per_yunit;
	draw_rect_unmod.y += viewport_coord().y();	

	draw_rect_unmod.w = (int)_pixels_per_xunit;
	draw_rect_unmod.h = (int)_pixels_per_yunit;
	return &draw_rect_unmod;
}


// determine the draw_rect given a col and row position
// It will modify the x and y such that they are within
// bounds, and with a width/height that allow them to fit
// in the current camera extent.
SDL_Rect* Viewport::camera_unit_draw_rect(int col, int row){
	draw_rect = *camera_unit_draw_rect_unmod(col, row);

	// only happens to the first columns
	if(draw_rect.x < viewport_coord().x()){
		draw_rect.x = viewport_coord().x();
	}
	// only happens to the last columns
	if(draw_rect.x + draw_rect.w - viewport_coord().x() >= camera_coords.w()){
		draw_rect.w -= draw_rect.x + draw_rect.w - viewport_coord().x() - camera_coords.w();
	}

	if(draw_rect.y < viewport_coord().y()){
		draw_rect.y = viewport_coord().y();
	}
	if(draw_rect.y + draw_rect.h - viewport_coord().y() >= camera_coords.h()){
		draw_rect.h -= draw_rect.y + draw_rect.h - viewport_coord().y() - camera_coords.h();
	}

	return &this->draw_rect;
}


// private methods
int Viewport::camera_col_unit_offset(){
	return (int)ceil((float)(camera_coords.w() + camera_coords.x()) / _pixels_per_xunit);
}
int Viewport::camera_row_unit_offset(){
	return (int)ceil((float)(camera_coords.h() + camera_coords.y()) / _pixels_per_yunit);
}


void Viewport::make_cam_x_within_bounds(){
	if(_horiz_wrap){
		if(camera_coords.x() < 0){
			camera_coords.x((int)(target->w()*_pixels_per_xunit) + camera_coords.x());
		} else if(camera_coords.x() >= target->w()*_pixels_per_xunit){
			camera_coords.x(camera_coords.x() % (int)(target->w()*_pixels_per_xunit));
		}
	} else{
		if(camera_coords.x() < 0){
			camera_coords.x(0);
		} else if(camera_coords.x() >= target->w()*_pixels_per_xunit - camera_coords.w()){
			camera_coords.x((int)(target->w()*_pixels_per_xunit) - camera_coords.w());
		}
	}

}

void Viewport::make_cam_y_within_bounds(){
	if(_vert_wrap){
		if(camera_coords.y() < 0){
			camera_coords.y((target->h()*_pixels_per_yunit) + camera_coords.y());
		} else if(camera_coords.y() >= target->h()*_pixels_per_yunit){
			camera_coords.y(camera_coords.y() % (int)(target->h()*_pixels_per_yunit));
		}

	} else {
		if(camera_coords.y() < 0){
			camera_coords.y(0);			
		} else if(camera_coords.y() >= target->h()*_pixels_per_yunit - camera_coords.h()){
			camera_coords.y((int)(target->h()*_pixels_per_yunit) - camera_coords.h());			
		}
	}
}

void Viewport::render_viewport_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	if(extent == nullptr){ return; }

	SDL_Rect intersection;
	SDL_Rect B;
	std::list<IPane*>::reverse_iterator it;
	for(it = _viewport_children.rbegin(); it != _viewport_children.rend(); ++it){
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
bool Viewport::add_viewport_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it = _viewport_children.begin();
	for(it = _viewport_children.begin(); it != _viewport_children.end(); ++it){
		if((*it) == pane){ return false; }
	}

	// add the pane in the appropriate place
	bool added = false;
	for(it = _viewport_children.begin(); it != _viewport_children.end(); ++it){
		if(IPane::compare_IPane(pane, *it)){
			_viewport_children.insert(it, pane);
			added = true;
			break;
		}
	}
	if(added == false){
		_viewport_children.push_back(pane);
	}

	pane->coord().set_parent(&this->_coord);	
	return true;
}

bool Viewport::remove_viewport_pane(IPane* pane){
	if(pane == nullptr){ return false; }
	std::list<IPane*>::iterator it;
	for(it = _viewport_children.begin(); it != _viewport_children.end(); ++it){
		if((*it) == pane) {
			(*it)->coord().set_parent(nullptr);
			_viewport_children.erase(it);			
			return true;
		}
	}
	return false;
}
void Viewport::determine_focused_viewport_child_pane(IMouse* rel_mouse){
	if(rel_mouse == nullptr){ return; }
	//if(_focused_viewport_child_pane->hitbox().collides(rel_mouse->hitbox())){
	//	// the currently focused_child still has focus.
	//	return;
	//}

	std::list<IPane*>::iterator it;
	bool done = false;
	for(it = _viewport_children.begin(); it != _viewport_children.end(); ++it){
		// make sure that the pane is 'active'
		if((*it)->isactive() == false){ continue; }

		if((*it)->hitbox().collides(rel_mouse->hitbox())){
			//the pane is the currently in focus pane
			if(*it == _focused_viewport_child_pane){
				done = true;
				break;
			}

			// a new pane has gained focus.
			_focused_viewport_child_pane->set_focus(false);
			_focused_viewport_child_pane = *it;
			_focused_viewport_child_pane->set_focus(true);
			done = true;
			break;
		}
	}

	// no panes were selected.
	if(done == false){
		_focused_viewport_child_pane->set_focus(false);
		_focused_viewport_child_pane = &NULL_Pane::get();
	} else{
		get_focused_child_pane()->set_focus(false);
	}
}
IPane* Viewport::get_focused_viewport_child_pane(){
	return _focused_viewport_child_pane;
}
void Viewport::defocus_all_viewport_children(){
	_focused_viewport_child_pane->defocus_all_children();
	_focused_viewport_child_pane->set_focus(false);
	_focused_viewport_child_pane = &NULL_Pane::get();
}