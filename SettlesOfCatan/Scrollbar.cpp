#include <SDL.h>
#include "Util.h"
#include "Pane.h"
#include "Coords.h"
#include "Viewport.h"
#include "mouse.h"
#include "Scrollbar.h"

// -------------------------------------------------------------------
// _ S C R O L L B A R
// -------------------------------------------------------------------
_Scrollbar::_Scrollbar(Scrollbar* viewport,type_e type)
{
	this->viewport = viewport;
	this->type = type;	
	this->min_bar_px_len = 10;
	this->default_bar_area_px_width = 15;	
	this->active = false;
	this->bar.setactive(false);
	this->bar_area.setactive(false);
	this->bar.setvisible(false);
	this->bar_area.setvisible(false);
	
	if(type == _Scrollbar::HORIZONTAL){
		bar.coord().init(0, 0, 1, min_bar_px_len, default_bar_area_px_width);
		bar_area.coord().init(0, 0, 0, 1, default_bar_area_px_width);
		bar_area.coord().set_relative_w(1.0f); // the full length of the viewport?			
		bar_area.type = _Scrollbar::HORIZONTAL;
		bar_position = _Scrollbar::BOTTOM;
	} else if(type == _Scrollbar::VERTICAL){
		bar.coord().init(0, 0, 1, default_bar_area_px_width, min_bar_px_len);
		bar_area.coord().init(0, 0, 0, default_bar_area_px_width, 1);
		bar_area.coord().set_relative_h(1.0f); // the full length of the viewport?	
		bar_area.type = _Scrollbar::VERTICAL;
		bar_position = _Scrollbar::RIGHT;
	}	
	this->viewport->add_viewport_pane(&bar);
	this->viewport->add_viewport_pane(&bar_area);
	//this->bar.coord().set_parent(&this->viewport->viewport_coord());
	//this->bar_area.coord().set_parent(&this->viewport->viewport_coord());

	bar.subscribe(this->viewport);
	bar_area.subscribe(this->viewport);
}
_Scrollbar::~_Scrollbar(){}
int _Scrollbar::cam_px_from_bar_pos(){
	// camx/target_len = barpos/bar_are_len
	if(bar_area_px_len() != 0){
		return (bar_px_pos()*target_px_len()) / bar_area_px_len();
	}
	return 0;
}
int _Scrollbar::bar_pos_from_cam_px(){
	if(target_px_len() != 0){
		return (cam_px_pos()*bar_area_px_len()) / target_px_len();
	}
	return 0;
}
int _Scrollbar::bar_px_pos(){
	return  (type == _Scrollbar::VERTICAL) ?
		bar.coord().y() : bar.coord().x();
}
int _Scrollbar::bar_px_len(){
	return  (type == _Scrollbar::VERTICAL) ?
		bar.coord().h() : bar.coord().w();
}
int _Scrollbar::bar_area_px_len(){
	return  (type == _Scrollbar::VERTICAL) ?
		bar_area.coord().h() : bar_area.coord().w();
}
int _Scrollbar::target_px_len(){ 
	return  (type == _Scrollbar::VERTICAL) ?
		viewport->target_unit_h()*viewport->pixels_per_yunit() :
		viewport->target_unit_w()*viewport->pixels_per_xunit();
}
int _Scrollbar::cam_px_len(){ 
	return  (type == _Scrollbar::VERTICAL) ?
		viewport->camera_px_h() : viewport->camera_px_w(); 
}
int _Scrollbar::cam_px_pos(){
	return  (type == _Scrollbar::VERTICAL) ?
		viewport->camera_px_y() : viewport->camera_px_x();
}

void _Scrollbar::set_bar_side_position(bar_position_e pos){	
	bar_position = pos;
	if(pos == _Scrollbar::RIGHT){		
		bar.coord().set_relative_x((int)bar_area.coord().w());
		bar_area.coord().set_relative_x((int)bar_area.coord().w());		
	} else if(pos == _Scrollbar::LEFT){		
		bar.coord().set_relative_x(0.0f);
		bar_area.coord().set_relative_x(0.0f);	
	} else if(pos == _Scrollbar::TOP){		
		bar.coord().set_relative_y(0.0f);
		bar_area.coord().set_relative_y(0.0f);		
	} else if(pos == _Scrollbar::BOTTOM){		
		bar.coord().set_relative_y((int)bar_area.coord().h());
		bar_area.coord().set_relative_y((int)bar_area.coord().h());		
	}
}
void _Scrollbar::set_type(type_e type){
	this->type = type;
	if(this->type == _Scrollbar::VERTICAL){
		bar_area.type = _Scrollbar::VERTICAL;
		bar.coord().init(0, 0, 1, default_bar_area_px_width, min_bar_px_len);
		bar_area.coord().init(0, 0, 0, default_bar_area_px_width, 1);
		bar_area.coord().set_relative_h(1.0f); // the full length of the viewport?	
	} else if(this->type == _Scrollbar::HORIZONTAL){
		bar_area.type = _Scrollbar::HORIZONTAL;
		bar.coord().init(0, 0, 1, min_bar_px_len, default_bar_area_px_width);
		bar_area.coord().init(0, 0, 0, 1, default_bar_area_px_width);
		bar_area.coord().set_relative_w(1.0f); // the full length of the viewport?					
	}
}
void _Scrollbar::adjust_bar_dimensions(){
	//bar_len / bar_area_total = cam_len / target_len;
	int len = min_bar_px_len;
	if(target_px_len() != 0){
		len = (cam_px_len()* bar_area_px_len()) / target_px_len();
	}
	// FUCK!
	if(len < min_bar_px_len){
		len = min_bar_px_len;
	}
	(type == _Scrollbar::VERTICAL) ? bar.coord().h(len) : bar.coord().w(len);
}
void _Scrollbar::adjust_bar_position(){
	int bar_pos = bar_pos_from_cam_px();
	(type == _Scrollbar::VERTICAL) ?
		bar.coord().y(bar_pos) : bar.coord().x(bar_pos);
}
void _Scrollbar::set_bar_px_pos(int value){
	if(type == _Scrollbar::VERTICAL) {
		bar.coord().y(value);
		if(bar_px_pos() < 0){
			bar.coord().y(0);
		} else if(bar_px_pos() + bar_px_len() >= bar_area_px_len()){
			bar.coord().y(bar_area_px_len() - bar_px_len());
		}
	} else if( type == _Scrollbar::HORIZONTAL){
		bar.coord().x(value);
		if(bar_px_pos() < 0){
			bar.coord().x(0);
		} else if(bar_px_pos() + bar_px_len() >= bar_area_px_len()){
			bar.coord().x(bar_area_px_len() - bar_px_len());
		}
	}
}
void _Scrollbar::set_bar_px_len(int value){
	if(type == _Scrollbar::VERTICAL) {
		if(value < min_bar_px_len){
			bar.coord().h(min_bar_px_len);
		} else{
			bar.coord().h(value);
		}
	} else if (type == _Scrollbar::HORIZONTAL) {
		if(value < min_bar_px_len){
			bar.coord().w(min_bar_px_len);
		} else{
			bar.coord().w(value);
		}
	}
}
void _Scrollbar::set_cam_px_len(int value){
	if(type == _Scrollbar::VERTICAL) {
		viewport->set_camera_px_h(value);
	} else{
		viewport->set_camera_px_w(value);
	}
	
}
void _Scrollbar::set_cam_px_pos(int value){
	if(type == _Scrollbar::VERTICAL) {
		viewport->set_camera_px_y(value);
	} else{
		viewport->set_camera_px_x(value);
	}	
}
void _Scrollbar::show(type_e type, bar_position_e pos){
	active = true;
	bar.setactive(true);
	bar_area.setactive(true);
	bar.setvisible(true);
	bar_area.setvisible(true);
	set_type(type);
	set_bar_side_position(pos);
	//adjust_bar_position();
	//adjust_bar_dimensions();
}
void _Scrollbar::hide(){
	active = false;
	bar.setactive(false);
	bar_area.setactive(false);
	bar.setvisible(false);
	bar_area.setvisible(false);	
	//adjust_bar_position();
	//adjust_bar_dimensions();
}

// -------------------------------------------------------------------
// S C R O L L B A R _ B A R
// -------------------------------------------------------------------
_Scrollbar::Scrollbar_bar::Scrollbar_bar()
: Pane()
{
	focused_colour = { 20, 60, 40, 90 };
	selected_colour = { 0, 0, 255, 140 };
}

_Scrollbar::Scrollbar_bar::~Scrollbar_bar(){ }
bool _Scrollbar::Scrollbar_bar::keyboard_keydown(SDL_Event& ev){ return false; }
bool _Scrollbar::Scrollbar_bar::keyboard_keyup(SDL_Event& ev){ return false; }
bool _Scrollbar::Scrollbar_bar::mouse_buttondown(SDL_Event& ev, Coords* ref){
	if(has_focus()){
		set_selected(true);
	}
	//notify_mouse_buttondown(0, (void*)this);
	return true;
}
bool _Scrollbar::Scrollbar_bar::mouse_buttonup(SDL_Event& ev, Coords* ref){
	set_selected(false);
	//notify_mouse_buttonup(0, (void*)this);
	return true;
}
bool _Scrollbar::Scrollbar_bar::mouse_motion(SDL_Event& ev, Coords* ref){
	return true;
}
bool _Scrollbar::Scrollbar_bar::mouse_drag(SDL_Event& ev, Coords* ref){
	if(is_selected()){
		coord().x(coord().x() + ev.motion.xrel);
		coord().y(coord().y() + ev.motion.yrel);
		coord().make_within_bounds(coord().get_parent());
	}
	notify_mouse_drag(0,this);
	return true;
}
void _Scrollbar::Scrollbar_bar::tick(){}
void _Scrollbar::Scrollbar_bar::update(SDL_Event& ev){}
void _Scrollbar::Scrollbar_bar::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0, 0,
		coord().w(),
		coord().h()
	};
	render(ren, coord().disp_x(), coord().disp_y(), &rect);
}
void _Scrollbar::Scrollbar_bar::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	SDL_Rect old_clip;
	SDL_Rect new_clip = {
		x, y, extent->w, extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);

	if(has_focus()){
		// stuff??
	}
	SDL_Rect rect = *extent;
	rect.x = x;
	rect.y = y;
	Util::render_fill_rectangle(&ren, &rect, focused_colour);
	rect = {
		x - extent->x,
		y - extent->y,
		coord().w(),
		coord().h()
	};
	Util::render_rectangle(&ren, &rect, selected_colour);

	SDL_RenderSetClipRect(&ren, &old_clip);
}

void _Scrollbar::Scrollbar_bar::on_focus(){
	focused_colour = { 20, 60, 120, 90 };
}
void _Scrollbar::Scrollbar_bar::off_focus(){
	focused_colour = { 20, 60, 40, 90 };
	Pane::defocus_all_children();
}
void _Scrollbar::Scrollbar_bar::on_selected(){
	selected_colour = { 255, 0, 0, 255 };
	// do nothing ...
}
void _Scrollbar::Scrollbar_bar::off_selected(){
	selected_colour = { 0, 0, 255, 255 };
	// do nothing ...
}

// -------------------------------------------------------------------
// S C R O L L B A R _ B A R _ A R E A
// -------------------------------------------------------------------
_Scrollbar::Scrollbar_bar_area::Scrollbar_bar_area()
: Pane()
{
	focused_colour = { 20, 60, 40, 90 };
	selected_colour = { 0, 0, 255, 140 };
	clicked_pos = 0;
//	this->type = _Scrollbar::VERTICAL;
}
_Scrollbar::Scrollbar_bar_area::~Scrollbar_bar_area(){ }
bool _Scrollbar::Scrollbar_bar_area::keyboard_keydown(SDL_Event& ev){ return false; }
bool _Scrollbar::Scrollbar_bar_area::keyboard_keyup(SDL_Event& ev){ return false; }
bool _Scrollbar::Scrollbar_bar_area::mouse_buttondown(SDL_Event& ev, Coords* ref){
	if(has_focus()){
		set_selected(true);
	}

	if(ref != nullptr){

	} else{
		// FUCK THIS SHITS
		IMouse* mouse = &GMouse::get(&this->coord());
		if(type == _Scrollbar::VERTICAL){
			clicked_pos = mouse->y();
		} else if(type == _Scrollbar::HORIZONTAL){
			clicked_pos = mouse->x();
		}
	}
	notify_mouse_buttondown(0, this);
	return true;
}
bool _Scrollbar::Scrollbar_bar_area::mouse_buttonup(SDL_Event& ev, Coords* ref){
	set_selected(false);
	return true;
}
bool _Scrollbar::Scrollbar_bar_area::mouse_motion(SDL_Event& ev, Coords* ref){
	return true;
}
bool _Scrollbar::Scrollbar_bar_area::mouse_drag(SDL_Event& ev, Coords* ref){
	return true;
}
void _Scrollbar::Scrollbar_bar_area::tick(){}
void _Scrollbar::Scrollbar_bar_area::update(SDL_Event& ev){}
void _Scrollbar::Scrollbar_bar_area::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0, 0,
		coord().w(),
		coord().h()
	};
	render(ren, coord().disp_x(), coord().disp_y(), &rect);
}
void _Scrollbar::Scrollbar_bar_area::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	SDL_Rect old_clip;
	SDL_Rect new_clip = {
		x, y, extent->w, extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);

	if(has_focus()){
		// stuff??
	}
	SDL_Rect rect = *extent;
	rect.x = x;
	rect.y = y;
	Util::render_fill_rectangle(&ren, &rect, focused_colour);

	SDL_RenderSetClipRect(&ren, &old_clip);
}

void _Scrollbar::Scrollbar_bar_area::on_focus(){
	focused_colour = { 20, 60, 120, 90 };
}
void _Scrollbar::Scrollbar_bar_area::off_focus(){
	focused_colour = { 20, 60, 40, 90 };
	Pane::defocus_all_children();
}
void _Scrollbar::Scrollbar_bar_area::on_selected(){
	selected_colour = { 255, 0, 0, 255 };
	// do nothing ...
}
void _Scrollbar::Scrollbar_bar_area::off_selected(){
	selected_colour = { 0, 0, 255, 255 };
	// do nothing ...
}



// -------------------------------------------------------------------
// S C R O L L B A R
// -------------------------------------------------------------------
Scrollbar::Scrollbar()
: _horiz_bar(this, _Scrollbar::HORIZONTAL),
_vert_bar(this, _Scrollbar::VERTICAL)
{
	Sprite::coord().init(0, 0, 0, 1, 1);
}
Scrollbar::~Scrollbar(){}

void Scrollbar::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0, 0, coord().w(), coord().h()
	};
	render(ren,
		coord().disp_x(),
		coord().disp_y(),
		&rect);
}
void Scrollbar::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	SDL_Rect old_clip;
	SDL_Rect new_clip = {
		x, y, extent->w, extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);

	int dispx = x;
	int dispy = y;
	int dispw = extent->w;
	int disph = extent->h;
	if(_horiz_bar.active){
		if(_horiz_bar.bar_position == _Scrollbar::TOP){
			dispy = y + _horiz_bar.default_bar_area_px_width;
			disph = extent->h - _horiz_bar.default_bar_area_px_width;
		} else if(_horiz_bar.bar_position == _Scrollbar::BOTTOM) {
			disph = extent->h - _horiz_bar.default_bar_area_px_width;
		}
	}
	if(_vert_bar.active){
		if(_vert_bar.bar_position == _Scrollbar::LEFT){
			dispx = x + _vert_bar.default_bar_area_px_width;
			dispw = extent->w - _vert_bar.default_bar_area_px_width;
		} else if(_vert_bar.bar_position == _Scrollbar::RIGHT) {
			dispw = extent->w - _vert_bar.default_bar_area_px_width;
		}
	}

	SDL_Rect target_extent = {
		camera_coords.x() + extent->x,
		camera_coords.y() + extent->y,
		dispw, disph
	};
	this->wrappee->render(ren, dispx, dispy, &target_extent);
	this->render_viewport_children(ren, x, y, extent);

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

bool Scrollbar::keyboard_keydown(SDL_Event& ev){
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	if(keyboard[SDL_SCANCODE_1]){
		_horiz_bar.adjust_bar_dimensions();
		_vert_bar.adjust_bar_dimensions();
	}
	if(keyboard[SDL_SCANCODE_2]){		
		if(keyboard[SDL_SCANCODE_LSHIFT]){			
			hide_vertical_scrollbar();
		} else{
			show_vertical_scrollbar();			
		}
	}
	if(keyboard[SDL_SCANCODE_3]){
		if(keyboard[SDL_SCANCODE_LSHIFT]){
			hide_horizontal_scrollbar();
		} else{
			show_horizontal_scrollbar();			
		}
	}

	if(_vert_bar.active){
		if(keyboard[SDL_SCANCODE_UP]){
			move_camera_px_y(-(int)(pixels_per_yunit()));
		} else if(keyboard[SDL_SCANCODE_DOWN]){
			move_camera_px_y(pixels_per_yunit());
		}
	}
	if(_horiz_bar.active){
		if(keyboard[SDL_SCANCODE_LEFT]){
			move_camera_px_x(-(int)(pixels_per_xunit()));
		} else if(keyboard[SDL_SCANCODE_RIGHT]){
			move_camera_px_x(pixels_per_xunit());
		}
	}

	Viewport::keyboard_keydown(ev);
	return true;	
}
bool Scrollbar::mouse_buttondown(SDL_Event& ev, Coords* ref){	
	IPane* focused_pane = get_focused_viewport_child_pane();
	if(NULL_Pane::isDummy(focused_pane) ){
		Viewport::mouse_buttondown(ev, ref);
	} else{
		if(focused_pane->has_focus()){
			Coords* new_ref = new_coords_from_ref(ref, &focused_pane->coord());
			focused_pane->mouse_buttondown(ev);
		}
	}	
	return true;
}
bool Scrollbar::mouse_buttonup(SDL_Event& ev, Coords* ref){
	if(_horiz_bar.active){
		_horiz_bar.bar.set_selected(false);
		_horiz_bar.bar_area.set_selected(false);
	}
	if(_vert_bar.active){
		_vert_bar.bar.set_selected(false);
		_vert_bar.bar_area.set_selected(false);
	}
	Viewport::mouse_buttonup(ev, ref);
	return true;
}
bool Scrollbar::mouse_motion(SDL_Event& ev, Coords* reference){	
	// get a relative mouse to use for intersections.
	IMouse& rel_mouse = GMouse::get(&coord());
	rel_mouse.update();
	if(reference != nullptr){
		rel_mouse.x(reference->x());
		rel_mouse.y(reference->y());
	}
	if(has_focus()){
		determine_focused_viewport_child_pane(&rel_mouse);
	}

	IPane* focused_pane = get_focused_viewport_child_pane();
	if(NULL_Pane::isDummy(focused_pane)){
		Viewport::mouse_motion(ev, reference);
	} else{
		// nothing to do.
	}
	
	return true;
}
bool Scrollbar::mouse_drag(SDL_Event& ev, Coords* ref ){
	IMouse& rel_mouse = GMouse::get(&coord());
	rel_mouse.update();
	if(ref != nullptr){
		rel_mouse.x(ref->x());
		rel_mouse.y(ref->y());
	}
	if(has_focus()){
		determine_focused_viewport_child_pane(&rel_mouse);
	}

	bool only_us = true;
	if(_vert_bar.active){
		if(_vert_bar.bar.is_selected()){
			// make sure that the mouse is within the 'acceptable' region
			// drag the mouse bar?
			Coords* new_ref = new_coords_from_ref(ref, &_vert_bar.bar.coord());
			_vert_bar.bar.mouse_drag(ev, new_ref);
			only_us = false;
		}
	} 
	if(_horiz_bar.active){
		if(_horiz_bar.bar.is_selected()){
			// make sure that the mouse is within the 'acceptable' region
			// drag the mouse bar?
			Coords* new_ref = new_coords_from_ref(ref, &_horiz_bar.bar.coord());
			_horiz_bar.bar.mouse_drag(ev, new_ref);
			only_us = false;
		}
	}

	if(only_us){
		// normal viewport operations
		Viewport::mouse_drag(ev, ref);
	}	
	return true;
}
void Scrollbar::set_target(IPane* target, unsigned ppux, unsigned ppuy){
	Viewport::set_target(target, ppux, ppuy);	
}

void Scrollbar::set_camera_coords(int x, int y, int w, int h){
	Viewport::set_camera_coords(x, y, w, h);
	if(_horiz_bar.active){
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();
	}
	if(_vert_bar.active){
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();
	}	
}
void Scrollbar::set_viewport_coords(int x, int y, int w, int h){
	Viewport::set_viewport_coords(x, y, w, h);	
	if(_vert_bar.active){
		if(_vert_bar.bar_position == _Scrollbar::LEFT){
			viewport_coord().x(x + _vert_bar.default_bar_area_px_width);
			viewport_coord().w(viewport_coord().w() - _vert_bar.default_bar_area_px_width);
		} else if(_vert_bar.bar_position == _Scrollbar::RIGHT){
			viewport_coord().w(viewport_coord().w() - _vert_bar.default_bar_area_px_width);
		}
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();
	}
	if(_horiz_bar.active){
		if(_horiz_bar.bar_position == _Scrollbar::TOP){
			viewport_coord().y(y + _horiz_bar.default_bar_area_px_width);
			viewport_coord().h(viewport_coord().h() - _horiz_bar.default_bar_area_px_width);
		} else if(_horiz_bar.bar_position == _Scrollbar::BOTTOM){
			viewport_coord().h(viewport_coord().h() - _horiz_bar.default_bar_area_px_width);
		}
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();
	}	
}
//void Scrollbar::set_pixels_per_xunit(unsigned ppu);
//void Scrollbar::set_pixels_per_yunit(unsigned ppu);
int Scrollbar::viewport_px_x(){ return viewport_coord().x(); }
int Scrollbar::viewport_px_y(){ return viewport_coord().y(); }
int Scrollbar::viewport_px_w(){ return viewport_coord().w(); }
int Scrollbar::viewport_px_h(){ return viewport_coord().h(); }
int Scrollbar::camera_px_x() {return camera_coords.x();}
int Scrollbar::camera_px_y() {return camera_coords.y();}
int Scrollbar::camera_px_w() {return camera_coords.w();}
int Scrollbar::camera_px_h() {return camera_coords.h();}
void Scrollbar::set_camera_px_x(int amount){
	camera_coords.x(amount);
	make_cam_x_within_bounds();
	_horiz_bar.adjust_bar_position();
}
void Scrollbar::set_camera_px_y(int amount){
	camera_coords.y(amount);
	make_cam_y_within_bounds();
	_vert_bar.adjust_bar_position();
}
void Scrollbar::set_camera_px_w(int value){
	camera_coords.w(value);
	_horiz_bar.adjust_bar_dimensions();
}
void Scrollbar::set_camera_px_h(int value){
	camera_coords.h(value);	
	_vert_bar.adjust_bar_dimensions();
}
//void Scrollbar::move_camera_px_x(int amount){}
//void Scrollbar::move_camera_px_y(int amount){}
//int Scrollbar::camera_unit_x(){return 0;}
//int Scrollbar::camera_unit_y(){return 0;}
//int Scrollbar::camera_unit_w(){return 0;}
//int Scrollbar::camera_unit_h(){return 0;}
void Scrollbar::update_on_mouse_motion(Mouseable* origin, int code, void* data){
	//do nothing.
}
void Scrollbar::update_on_mouse_drag(Mouseable* origin, int code, void* data){
	if(data == &_vert_bar.bar && _vert_bar.active){		
		_vert_bar.set_cam_px_pos(_vert_bar.cam_px_from_bar_pos());
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();
	} else if(data == &_horiz_bar.bar && _horiz_bar.active){
		_horiz_bar.set_cam_px_pos(_horiz_bar.cam_px_from_bar_pos());
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();
	}
}
void Scrollbar::update_on_mouse_buttondown(Mouseable* origin, int code, void* data){
	if(data == &_vert_bar.bar_area && _vert_bar.active){
		int value = _vert_bar.bar_area.clicked_pos - (_vert_bar.bar_px_len()/2);
		_vert_bar.set_bar_px_pos(value);
		_vert_bar.set_cam_px_pos(_vert_bar.cam_px_from_bar_pos());
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();

		_vert_bar.bar.set_selected(true);
	} else if(data == &_horiz_bar.bar_area && _horiz_bar.active){
		int value = _horiz_bar.bar_area.clicked_pos - (_horiz_bar.bar_px_len() / 2);
		_horiz_bar.set_bar_px_pos(value);
		_horiz_bar.set_cam_px_pos(_horiz_bar.cam_px_from_bar_pos());
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();

		_horiz_bar.bar.set_selected(true);
	}
}
void Scrollbar::update_on_mouse_buttonup(Mouseable* origin, int code, void* data){	
	// do nothing
}


void Scrollbar::show_horizontal_scrollbar(){
	_horiz_bar.show(_Scrollbar::HORIZONTAL, _Scrollbar::BOTTOM); 
	if(_horiz_bar.active){
		if(_horiz_bar.bar_position == _Scrollbar::TOP){
			viewport_coord().y(viewport_coord().y() + _horiz_bar.default_bar_area_px_width);
			viewport_coord().h(viewport_coord().h() - _horiz_bar.default_bar_area_px_width);
			camera_coords.h(camera_coords.h() - _horiz_bar.default_bar_area_px_width);
		} else if(_horiz_bar.bar_position == _Scrollbar::BOTTOM){
			viewport_coord().h(viewport_coord().h() - _horiz_bar.default_bar_area_px_width);
			camera_coords.h(camera_coords.h() - _horiz_bar.default_bar_area_px_width);
		}
		
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();
	}
}
void Scrollbar::hide_horizontal_scrollbar(){
	if(_horiz_bar.active){
		if(_horiz_bar.bar_position == _Scrollbar::TOP){
			viewport_coord().y(viewport_coord().y() - _horiz_bar.default_bar_area_px_width);
			viewport_coord().h(viewport_coord().h() + _horiz_bar.default_bar_area_px_width);
			camera_coords.h(camera_coords.h() + _horiz_bar.default_bar_area_px_width);
		} else if(_horiz_bar.bar_position == _Scrollbar::BOTTOM){
			viewport_coord().h(viewport_coord().h() + _horiz_bar.default_bar_area_px_width);
			camera_coords.h(camera_coords.h() + _horiz_bar.default_bar_area_px_width);
		}
		_horiz_bar.adjust_bar_position();
		_horiz_bar.adjust_bar_dimensions();
	}
	_horiz_bar.hide();	
}
void Scrollbar::show_vertical_scrollbar(){
	_vert_bar.show(_Scrollbar::VERTICAL, _Scrollbar::RIGHT);
	if(_vert_bar.active){
		if(_vert_bar.bar_position == _Scrollbar::LEFT){
			viewport_coord().x(viewport_coord().x() + _vert_bar.default_bar_area_px_width);
			viewport_coord().w(viewport_coord().w() - _vert_bar.default_bar_area_px_width);
			camera_coords.w(camera_coords.w() - _vert_bar.default_bar_area_px_width);
		} else if(_vert_bar.bar_position == _Scrollbar::RIGHT){
			viewport_coord().w(viewport_coord().w() - _vert_bar.default_bar_area_px_width);
			camera_coords.w(camera_coords.w() - _vert_bar.default_bar_area_px_width);
		}
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();
	}
}
void Scrollbar::hide_vertical_scrollbar(){
	if(_vert_bar.active){
		if(_vert_bar.bar_position == _Scrollbar::LEFT){
			viewport_coord().x(viewport_coord().x() - _vert_bar.default_bar_area_px_width);
			viewport_coord().w(viewport_coord().w() + _vert_bar.default_bar_area_px_width);
			camera_coords.w(camera_coords.w() + _vert_bar.default_bar_area_px_width);
		} else if(_vert_bar.bar_position == _Scrollbar::RIGHT){
			viewport_coord().w(viewport_coord().w() + _vert_bar.default_bar_area_px_width);
			camera_coords.w(camera_coords.w() + _vert_bar.default_bar_area_px_width);
		}
		_vert_bar.adjust_bar_position();
		_vert_bar.adjust_bar_dimensions();
	}
	_vert_bar.hide();	
}