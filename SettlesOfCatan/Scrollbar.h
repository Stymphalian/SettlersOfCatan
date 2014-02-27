#pragma once
#include "Pane.h"
#include "MouseListener.h"
#include "ViewPort.h"
class Scrollbar_bar : public Pane{
public:
	Scrollbar_bar();
	virtual ~Scrollbar_bar();

	virtual bool keyboard_keydown(SDL_Event& ev);
	virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	// Sprite Interface
	virtual void tick();
	virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	//IPane interface
	virtual void on_focus();
	virtual void off_focus();
	//Selectable interface	
	virtual void on_selected();
	virtual void off_selected();
protected:
	SDL_Color selected_colour;
	SDL_Color focused_colour;
};


class VerticalScrollbar : public Viewport{
public:	
	enum bar_position_e { LEFT,RIGHT };
	VerticalScrollbar();
	virtual ~VerticalScrollbar();		
	// Sprite Interface
	//virtual void tick();
	//virtual void update(SDL_Event& ev);
	//virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	//virtual bool isvisible();
	//virtual void setvisible(bool value);
	// Focusable Interface	
	//virtual bool has_focus();
	//virtual void set_focus(bool value);
	//virtual void on_focus();
	//virtual void off_focus();
	// Selected Interface
	//virtual void on_selected();
	//virtual void off_selected();
	//virtual bool is_selected();
	//virtual void set_selected(bool value);
	// KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev);
	//virtual bool keyboard_keyup(SDL_Event& ev);
	// MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	//IPane interface
	//virtual bool isactive();
	//virtual void setactive(bool value);
	//virtual bool add_pane(IPane* pane);
	//virtual bool remove_pane(IPane* pane);
	//virtual void set_background(SDL_Color colour);
	//virtual SDL_Color get_background();
	//virtual void defocus_all_children();
	//IViewport Interface
	// IViewport interface	
	virtual void set_camera_coords(int x, int y, int w, int h);
	//virtual void set_viewport_coords(int x, int y, int w, int h);
	//virtual void set_pixels_per_xunit(unsigned ppu);
	virtual void set_pixels_per_yunit(unsigned ppu);
	virtual int viewport_px_x();
	virtual int viewport_px_y();
	virtual int viewport_px_w();
	virtual int viewport_px_h();
	virtual int camera_px_x();
	virtual int camera_px_y();
	virtual int camera_px_w();
	virtual int camera_px_h();
	//virtual int target_unit_x() = 0;
	//virtual int target_unit_y() = 0;
	//virtual int target_unit_w() = 0;
	//virtual int target_unit_h() = 0;
	virtual void set_camera_px_x(int amount);
	virtual void set_camera_px_y(int amount);
	virtual void set_camera_px_w(int value);
	virtual void set_camera_px_h(int value);
	//virtual unsigned pixels_per_xunit() = 0;
	//virtual unsigned xunits_per_pixel() = 0;
	//virtual unsigned pixels_per_yunit() = 0;
	//virtual unsigned yunits_per_pixel() = 0;
	//virtual bool camera_horiz_wrap() = 0;
	//virtual bool camera_vert_wrap() = 0;
	//virtual void set_camera_horiz_wrap(bool value) = 0;
	//virtual void set_camera_vert_wrap(bool value) = 0;
	void move_camera_px_x(int amount);
	void move_camera_px_y(int amount);
	int camera_unit_x();
	int camera_unit_y();
	int camera_unit_w();
	int camera_unit_h();
	//VerticalScrollbar Methods
	void set_bar_side_position(bar_position_e pos);
protected:
	//int ratio();
	//int determine_bar_len_from_camera();
	//void calculate_bar_position_from_camera();//??	
	Mouse _rel_mouse;
	Scrollbar_bar _bar;
	Scrollbar_bar _bar_area;	
	bar_position_e _bar_side_position;
	bool _bar_object_is_selected;
	int _min_bar_px_len;
	int _default_bar_area_px_width;
	
	int make_within_bounds();	
	void adjust_bar_dimensions();
	void adjust_bar_position();
	int cam_px_from_bar_pos();
	int bar_pos_from_cam_px();
	int bar_px_pos();
	int bar_px_len();
	int bar_area_px_len();
	int target_px_len();
	int cam_px_len();
	int cam_px_pos();
		

	//IPane* wrappee;
	// IPane Interface	
	//virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent = nullptr);
	//virtual void determine_focused_child_pane(IMouse* mouse);
	//virtual IPane* get_focused_child_pane();
};

