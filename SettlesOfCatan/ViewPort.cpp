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
		IMouse& rel_mouse = GMouse::get(&coord());
		rel_mouse.update();
		relx = rel_mouse.x();
		rely = rel_mouse.y();		
	}
		
	/*IMouse& rel_mouse = GMouse::get(&coord());		
	rel_mouse.update();
	int relx = rel_mouse.x();
	int rely = rel_mouse.y();
	relx += (reference != nullptr) ? reference->x() : 0;
	rely += (reference != nullptr) ? reference->y() : 0;	
	*/

	/*
	IMouse& target_mouse = GMouse::get(target);
	target_mouse.update();

	int ref_x = camera_coords.x() + relx - target_mouse.x();
	int ref_y = camera_coords.y() + rely - target_mouse.y();
	*/

	//_passing_ref_coords.x(ref_x);
	//_passing_ref_coords.y(ref_y);
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
		0,0,coord().w(),coord().h()
	};
	render(ren, coord().disp_x(),coord().disp_y(),&rect);
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

	SDL_Rect rect = {
		x - extent->x,
		y - extent->y,
		coord().w(),
		coord().h()
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
//void Viewport::on_focus(){}
//void Viewport::off_focus(){}
//bool Viewport::has_focus(){
//	return _focus && wrappee->has_focus();
//}
//void Viewport::set_focus(bool value){	
//	if(value == true){
//		if(_focus == false){ on_focus(); }
//	} else{
//		if(_focus == true){ off_focus(); }
//	}
//	_focus = value;
//	wrappee->set_focus(value);
//}

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

int Viewport::viewport_px_x(){ return coord().x(); }
int Viewport::viewport_px_y(){ return coord().y(); }
int Viewport::viewport_px_w(){ return coord().w(); }
int Viewport::viewport_px_h(){ return coord().h(); }

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
	draw_rect_unmod.x += coord().x();

	if(_vert_wrap && row < camera_unit_y()){
		draw_rect_unmod.y = (row + target_unit_h() - camera_unit_y())*(int)_pixels_per_yunit;
	} else{
		draw_rect_unmod.y = (row - camera_unit_y())*(int)_pixels_per_yunit;				
	}	
	draw_rect_unmod.y -= camera_coords.y() % (int)_pixels_per_yunit;
	draw_rect_unmod.y += coord().y();	

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
	if(draw_rect.x < coord().x()){
		draw_rect.x = coord().x();
	}
	// only happens to the last columns
	if(draw_rect.x + draw_rect.w - coord().x() >= camera_coords.w()){
		draw_rect.w -= draw_rect.x + draw_rect.w - coord().x() - camera_coords.w();
	}

	if(draw_rect.y < coord().y()){
		draw_rect.y = coord().y();
	}
	if(draw_rect.y + draw_rect.h - coord().y() >= camera_coords.h()){
		draw_rect.h -= draw_rect.y + draw_rect.h - coord().y() - camera_coords.h();
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
