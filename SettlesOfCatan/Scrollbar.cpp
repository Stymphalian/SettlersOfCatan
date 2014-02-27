#include <SDL.h>
#include "Util.h"
#include "Pane.h"
#include "Coords.h"
#include "Viewport.h"
#include "mouse.h"
#include "Scrollbar.h"
// -------------------------------------------------------------------
// V E R T I C A L S C R O L L B A R
// -------------------------------------------------------------------
void VerticalScrollbar::set_bar_side_position(bar_position_e pos){
	_bar_side_position = pos;
	float amount = 0;
	if(pos == VerticalScrollbar::RIGHT){
		amount = 0.05f;
	} 
	_bar.coord().set_relative_x(amount);
	_bar_area.coord().set_relative_x(amount);
}

VerticalScrollbar::VerticalScrollbar(){
	_bar.coord().set_parent(&this->coord());
	_bar.coord().init(0, 0, 0, _default_bar_area_px_width, _min_bar_px_len);	

	_bar_area.coord().set_parent(&this->coord());		
	_bar_area.coord().init(0, 0, 0, _default_bar_area_px_width, 0);
	_bar_area.coord().set_relative_h(1.0f); // the full lenght of the viewport?

	_min_bar_px_len = 10;
	_default_bar_area_px_width = 15;
	_bar_object_is_selected = false;

	set_bar_side_position(VerticalScrollbar::RIGHT);	
}
VerticalScrollbar::~VerticalScrollbar(){}
//void VerticalScrollbar::render(SDL_Renderer& ren);
void VerticalScrollbar::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
	Viewport::render(ren, x, y, extent);
	// draw overtop the viewport
	// draw 
}
bool VerticalScrollbar::keyboard_keydown(SDL_Event& ev){
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	if(keyboard[SDL_SCANCODE_UP]){
		move_camera_px_y(pixels_per_yunit());
	} else if(keyboard[SDL_SCANCODE_DOWN]){
		move_camera_px_y(-(int)(pixels_per_yunit()));
	}
	return true;	
}
bool VerticalScrollbar::mouse_buttondown(SDL_Event& ev, Coords* ref){
	return false;
}
bool VerticalScrollbar::mouse_buttonup(SDL_Event& ev, Coords* ref){
	return false;
}
bool VerticalScrollbar::mouse_motion(SDL_Event& ev, Coords* ref){	
	// get a relative mouse to use for intersections.
	if(ref != nullptr){
		_rel_mouse.x(ref->x());
		_rel_mouse.y(ref->y());
	} else{
		IMouse& disp_rel_mouse = GMouse::get(&this->coord());
		_rel_mouse.x(disp_rel_mouse.x());
		_rel_mouse.y(disp_rel_mouse.y());
	}

	// check to to see if the scroll bar has focus
	bool we_have_focus = false;
	if(_bar.hitbox().collides(_rel_mouse.hitbox())){
		_bar.set_focus(true);		
		we_have_focus = true;
	} else if(_bar_area.hitbox().collides(_rel_mouse.hitbox())){
		_bar_area.set_focus(true);		
		we_have_focus = true;
	} 

	if(we_have_focus == false){
		// normal viewport operations
		Viewport::mouse_motion(ev,ref);
	}
	return true;
}
bool VerticalScrollbar::mouse_drag(SDL_Event& ev, Coords* ref ){
	if(_bar.is_selected()){		
		// make sure that the mouse is within the 'acceptable' region
		if(ref != nullptr){
			Coords new_ref;
			new_ref.x(ref->x() - _bar.coord().x());
			new_ref.y(ref->y() - _bar.coord().y());
			_bar.mouse_drag(ev, &new_ref);
		} else{
			_bar.mouse_drag(ev);
		}		
	} else{
		Viewport::mouse_drag(ev, ref);
	}
	return true;
}
void VerticalScrollbar::set_camera_coords(int x, int y, int w, int h){
	Viewport::set_camera_coords(camera_px_x(), y, w, h);
	adjust_bar_dimensions();
	adjust_bar_position();
}
//void VerticalScrollbar::set_viewport_coords(int x, int y, int w, int h);
//void VerticalScrollbar::set_pixels_per_xunit(unsigned ppu){}
void VerticalScrollbar::set_pixels_per_yunit(unsigned ppu){
	Viewport::set_pixels_per_yunit(ppu);	
}
int VerticalScrollbar::viewport_px_x(){return 0;}
int VerticalScrollbar::viewport_px_y(){return 0;}
int VerticalScrollbar::viewport_px_w(){return 0;}
int VerticalScrollbar::viewport_px_h(){return 0;}
int VerticalScrollbar::camera_px_x() {return 0;}
int VerticalScrollbar::camera_px_y() {return 0;}
int VerticalScrollbar::camera_px_w() {return 0;}
int VerticalScrollbar::camera_px_h() {return 0;}
void VerticalScrollbar::set_camera_px_x(int amount){}
void VerticalScrollbar::set_camera_px_y(int amount){}
void VerticalScrollbar::set_camera_px_w(int value){}
void VerticalScrollbar::set_camera_px_h(int value){}
void VerticalScrollbar::move_camera_px_x(int amount){}
void VerticalScrollbar::move_camera_px_y(int amount){}
int VerticalScrollbar::camera_unit_x(){return 0;}
int VerticalScrollbar::camera_unit_y(){return 0;}
int VerticalScrollbar::camera_unit_w(){return 0;}
int VerticalScrollbar::camera_unit_h(){return 0;}

//-----------------------------------
void VerticalScrollbar::set_bar_side_position(bar_position_e pos){
	_bar_side_position = pos;
}
void VerticalScrollbar::adjust_bar_dimensions(){
	//bar_len / bar_area_total = cam_len / target_len;
	int len = bar_px_len()*target_px_len() / bar_area_px_len();
	_bar.coord.h(len);
}
void VerticalScrollbar::adjust_bar_position(){
	int bar_pos = bar_pos_from_cam_px();
	_bar.coord().y(bar_pos);
}
int VerticalScrollbar::cam_px_from_bar_pos(){
	// camx/target_len = barpos/bar_are_len
	return (bar_px_pos()*target_px_len()) / bar_area_px_len();
}
int VerticalScrollbar::bar_pos_from_cam_px(){
	return (cam_px_pos()*bar_area_px_len()) / target_px_len();	
}
int VerticalScrollbar::bar_px_pos(){ return _bar.coord().y(); }
int VerticalScrollbar::bar_px_len(){ return _bar.coord().h(); }
int VerticalScrollbar::bar_area_px_len(){ return _bar_area.coord().h(); }
int VerticalScrollbar::target_px_len(){ return target_unit_h()*pixels_per_yunit(); }
int VerticalScrollbar::cam_px_len(){ return camera_px_h(); }
int VerticalScrollbar::cam_px_pos(){ return camera_px_y(); }



// -------------------------------------------------------------------
// S C R O L L B A R _ B A R
// -------------------------------------------------------------------
Scrollbar_bar::Scrollbar_bar()
: Pane()
{
	focused_colour = { 20, 60, 40, 90 };
	selected_colour = { 0, 0, 255, 140 };
}

Scrollbar_bar::~Scrollbar_bar(){ }

bool Scrollbar_bar::keyboard_keydown(SDL_Event& ev){ return false; }
bool Scrollbar_bar::keyboard_keyup(SDL_Event& ev){ return false; }

bool Scrollbar_bar::mouse_buttondown(SDL_Event& ev, Coords* ref){
	if(has_focus()){
		set_selected(true);
	}
	//notify_mouse_buttondown(0, (void*)this);
	return true;
}
bool Scrollbar_bar::mouse_buttonup(SDL_Event& ev, Coords* ref){
	set_selected(false);
	//notify_mouse_buttonup(0, (void*)this);
	return true;
}
bool Scrollbar_bar::mouse_motion(SDL_Event& ev, Coords* ref){
	return true;
}
bool Scrollbar_bar::mouse_drag(SDL_Event& ev, Coords* ref){
	if(is_selected()){
		coord().x(coord().x() + ev.motion.xrel);
		coord().y(coord().y() + ev.motion.yrel);
		coord().make_within_bounds(coord().get_parent());
	}
	return true;
}
void Scrollbar_bar::tick(){}
void Scrollbar_bar::update(SDL_Event& ev){}
void Scrollbar_bar::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0, 0,
		coord().w(),
		coord().h()
	};
	render(ren, coord().disp_x(), coord().disp_y(), &rect);
}
void Scrollbar_bar::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){
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

void Scrollbar_bar::on_focus(){
	focused_colour = { 20, 60, 120, 90 };
}
void Scrollbar_bar::off_focus(){
	focused_colour = { 20, 60, 40, 90 };
	Pane::defocus_all_children();
}
void Scrollbar_bar::on_selected(){
	selected_colour = { 255, 0, 0, 255 };
	// do nothing ...
}
void Scrollbar_bar::off_selected(){
	selected_colour = { 0, 0, 255, 255 };
	// do nothing ...
}