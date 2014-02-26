#pragma once
#include <SDL.h>
#include "Pane.h"
#include "Coords.h"

//----------------------------------------------
// I V I E W P O R T
//----------------------------------------------
// There are three extents which are use in a viewport.
// Viewport coordinates (px) - The pixel x,y,w,h position in which the viewport
//											is positioned on the screen/display
// camera coordinates (px) - the pixel x,y position, and the extent w,h 
//										of the camera in the targ et
// target coordinates (units) - the position and extent of the target 
//										in which the viewport is "viewing"
class IViewport {
public:		
	// IViewport interface	
	virtual void set_camera_coords(int x, int y, int w, int h)=0;
	virtual void set_viewport_coords(int x, int y,int w,int h)=0;	
	virtual void set_pixels_per_xunit(unsigned ppu) = 0;
	virtual void set_pixels_per_yunit(unsigned ppu) = 0;

	virtual int viewport_px_x()=0;
	virtual int viewport_px_y()=0;
	virtual int viewport_px_w()=0;
	virtual int viewport_px_h()=0;

	virtual int camera_px_x()=0;
	virtual int camera_px_y()=0;
	virtual int camera_px_w()=0;
	virtual int camera_px_h()=0;

	virtual int target_unit_x()=0;
	virtual int target_unit_y()=0;
	virtual int target_unit_w()=0;
	virtual int target_unit_h()=0;

	virtual void set_camera_px_x(int amount)=0;
	virtual void set_camera_px_y(int amount)=0;	
	virtual void set_camera_px_w(int value)=0;
	virtual void set_camera_px_h(int value)=0;	

	virtual unsigned pixels_per_xunit()=0;
	virtual unsigned xunits_per_pixel()=0;
	virtual unsigned pixels_per_yunit() = 0;
	virtual unsigned yunits_per_pixel() = 0;

	virtual bool camera_horiz_wrap() = 0;
	virtual bool camera_vert_wrap() = 0;
	virtual void set_camera_horiz_wrap(bool value) = 0;
	virtual void set_camera_vert_wrap(bool value) = 0;	
};

//----------------------------------------------
// V I E W P O R T
//----------------------------------------------s
class Viewport : public IWrapPane, public IViewport{
public:		
	// variables	
	Coords camera_coords;
	Viewport();
	virtual ~Viewport();

	// IWrapPane interface
	 // KeyboardHandler interface
	virtual bool keyboard_keydown(SDL_Event& ev);
	//virtual bool keyboard_keyup(SDL_Event& ev);
	 // MouseHandler interface
	virtual bool mouse_buttondown(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_buttonup(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_motion(SDL_Event& ev, Coords* ref = nullptr);
	virtual bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
	 // Sprite Interface
	//virtual void tick();
	//virtual void update(SDL_Event& ev);
	virtual void render(SDL_Renderer& ren);
	virtual void render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent);
	 //IPane interface
	//virtual bool add_pane(IPane* pane);
	//virtual bool remove_pane(IPane* pane);
	virtual bool isvisible();
	virtual void setvisible(bool value);
	//virtual void on_focus();
	//virtual void off_focus();
	virtual bool has_focus();
	virtual void set_focus(bool value);
	virtual void defocus_all_children();
	//virtual void set_background(SDL_Color colour);
	//virtual SDL_Color get_background();

	// IViewport Interface
	void set_target(IPane* target,unsigned ppxu,unsigned ppyu);
	void set_camera_coords(int x, int y,int w, int h);
	void set_viewport_coords(int x, int y,int w, int h);	
	void set_pixels_per_xunit(unsigned ppu);
	void set_pixels_per_yunit(unsigned ppu);

	int viewport_px_x();
	int viewport_px_y();
	int viewport_px_w();
	int viewport_px_h();

	int camera_px_x();
	int camera_px_y();
	int camera_px_w();
	int camera_px_h();

	int target_unit_x();
	int target_unit_y();
	int target_unit_w();
	int target_unit_h();

	void set_camera_px_x(int value);
	void set_camera_px_y(int value);
	void set_camera_px_w(int value);
	void set_camera_px_h(int value);

	unsigned pixels_per_xunit();
	unsigned xunits_per_pixel();
	unsigned pixels_per_yunit();
	unsigned yunits_per_pixel();

	bool camera_horiz_wrap(); // <-- stupid shit.
	bool camera_vert_wrap(); // <-- stupid shit.
	void set_camera_horiz_wrap(bool value); // <-- stupid shit.
	void set_camera_vert_wrap(bool value); // <-- stupid shit.	

	class iterator{
	public:
		iterator();
		virtual ~iterator();
		iterator(int x, int width, bool wrapping);
		iterator& operator++();
		iterator operator++(int stuff);
		int& operator*();
		int* operator->();
		bool operator==(const iterator& rhs);
		bool operator!=(const iterator& rhs);
	private:
		void make_within_bounds();
		int x;
		int width;
		bool wrapping;
	};
	iterator begin_camera_x();
	iterator end_camera_x();
	iterator begin_camera_y();
	iterator end_camera_y();
	
	// convenience methods
	void move_camera_px_x(int amount);
	void move_camera_px_y(int amount);
	int camera_unit_x();
	int camera_unit_y();
	int camera_unit_w();
	int camera_unit_h();	
	SDL_Rect* camera_unit_draw_rect(int col, int row);
	SDL_Rect* camera_unit_draw_rect_unmod(int col, int row);	
protected:	
	Coords* target;
	SDL_Rect draw_rect;
	SDL_Rect draw_rect_unmod;	
	Coords _passing_ref_coords;
	int _pixels_per_xunit;
	int _pixels_per_yunit;
	int _xunits_per_pixel;
	int _yunits_per_pixel;	
	bool _horiz_wrap;
	bool _vert_wrap;
	// IWrapPane necessary variables
	bool _visible;
	bool _focus;

	//methods
	inline void make_cam_x_within_bounds();
	inline void make_cam_y_within_bounds();
	inline int camera_row_unit_offset();
	inline int camera_col_unit_offset();
	Coords& determine_passing_reference_coords(Coords* ref);
};