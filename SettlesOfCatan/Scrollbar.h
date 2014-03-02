#pragma once
#include "Pane.h"
#include "MouseListener.h"
#include "ViewPort.h"

class Scrollbar;
class _Scrollbar{			
public:
	enum bar_position_e { TOP, BOTTOM, LEFT, RIGHT };
	enum type_e { VERTICAL, HORIZONTAL };
private:	
	// PRIVATE CLASSESS
	class Scrollbar_bar : public Pane, public Mouseable{
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

	class Scrollbar_bar_area : public Pane, public Mouseable{
	public:
		_Scrollbar::type_e type;
		Scrollbar_bar_area();
		virtual ~Scrollbar_bar_area();

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
		int clicked_pos;
	protected:
		SDL_Color selected_colour;
		SDL_Color focused_colour;
	private:
	};

public:	
	Scrollbar* viewport;
	type_e type;		// VERTICAL, HORIZONTAL
	bar_position_e bar_position; //TOP,LEFT, RIGHT, BOTTOM;
	Scrollbar_bar bar;	
	Scrollbar_bar_area bar_area;
	bool active;
	int min_bar_px_len;
	int default_bar_area_px_width;
	
	_Scrollbar(Scrollbar* viewport,type_e type);
	virtual ~_Scrollbar();
		
	int cam_px_from_bar_pos();
	int bar_pos_from_cam_px();
	int bar_px_pos();
	int bar_px_len();
	int bar_area_px_len();
	int target_px_len();
	int cam_px_len();
	int cam_px_pos();
	void set_type(type_e type);
	void set_bar_side_position(bar_position_e pos);
	void adjust_bar_dimensions();
	void adjust_bar_position();
	void set_bar_px_pos(int value);
	void set_bar_px_len(int value);
	void set_cam_px_len(int value);
	void set_cam_px_pos(int value);

	// conveniecne method for showing/hiding scrollbars.
	void show(_Scrollbar::type_e type,_Scrollbar::bar_position_e pos);
	void hide();
};


class Scrollbar : public Viewport, public IMouseListener{
public:		
	Scrollbar();
	virtual ~Scrollbar();		
	// Sprite Interface
	//virtual void tick();
	//virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
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
	virtual void set_target(IPane* target, unsigned ppux, unsigned ppuy);
	virtual void set_camera_coords(int x, int y, int w, int h);
	virtual void set_viewport_coords(int x, int y, int w, int h);
	//virtual void set_pixels_per_xunit(unsigned ppu);
	//virtual void set_pixels_per_yunit(unsigned ppu);
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
	//void move_camera_px_x(int amount);
	//void move_camera_px_y(int amount);
	//int camera_unit_x();
	//int camera_unit_y();
	//int camera_unit_w();
	//int camera_unit_h();	
	virtual void update_on_mouse_motion(Mouseable* origin, int code, void* data);
	virtual void update_on_mouse_drag(Mouseable* origin, int code, void* data);
	virtual void update_on_mouse_buttondown(Mouseable* origin, int code, void* data);
	virtual void update_on_mouse_buttonup(Mouseable* origin, int code, void* data);	
	
	void show_horizontal_scrollbar();
	void show_vertical_scrollbar();
	void hide_horizontal_scrollbar();
	void hide_vertical_scrollbar();
protected:
	friend class _Scrollbar;
	Mouse _rel_mouse;	
	_Scrollbar _horiz_bar;
	_Scrollbar _vert_bar;	
	bool add_viewport_pane(IPane* pane){ return Viewport::add_viewport_pane(pane); }		
				
	//IPane* wrappee;
	// IPane Interface	
	//virtual void render_children(SDL_Renderer& ren, int x, int y, SDL_Rect* extent = nullptr);
	//virtual void determine_focused_child_pane(IMouse* mouse);
	//virtual IPane* get_focused_child_pane();
};

