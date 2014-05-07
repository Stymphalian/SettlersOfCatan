#pragma once
#include <algorithm>
#include <map>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "Util.h"
#include "Timer.h"
#include "IView.h"
#include "IDialog.h"
#include "CheckBox.h"
#include "View_Game.h"
#include "TextField.h"
#include "DropDown.h"

#include "Vector3d.h"
#include "mouse.h"
#include "ObserverPattern.h"
#include "MouseListener.h"
#include "Selectable.h"

#include "Coords.h"
#include "Collision.h"
#include "Sprite.h"
#include "Pane.h"

#include "Viewport.h"
#include "Scrollbar.h"

class Polygon : public MouseHandler{
private:
	class min_max_t{
	public:
		bool good_flag;
		unsigned min, max;
		float min_len, max_len;
	};
	min_max_t min_max_along_axis(Polygon&  p, Vector3d& axis);
protected:
	void constructor(float x, float y, float z = 0.0f);
public:	
	Vector3d center;
	std::vector<Vector3d> vectors;
	std::vector<Vector3d> normals;

	Polygon();
	Polygon(float x, float y, float z= 0.0f);
	virtual ~Polygon();

	int num_points();
	void append_point(Vector3d vector);
	void append_point(float x,float y, float z = 0.0f);
	void add_point(unsigned index, Vector3d vector);	
	void add_point(unsigned index, float x, float y, float z = 0.0f);
	void remove_point(unsigned index);

	int min_along_axis(Vector3d& axis, float* rs = nullptr);
	int max_along_axis(Vector3d& axis, float* rs = nullptr);
	bool collides(Polygon& other);
	bool collides(Polygon& other, Vector3d& axis);
	void rotate(float angle); // +ve angle is clockwise
	void calculate_normals();
	void calculate_unit_normals();

	bool collided_flag;
	SDL_Color collided_colour;
	void set_collided(bool value);
	void on_collided();
	void off_collided();
	void render(SDL_Renderer& ren, int x, int y);
	bool mouse_drag(SDL_Event& ev, Coords* ref = nullptr);
};

class BB : MouseHandler{
public:
	Vector3d center;
	Vector3d vectors[4];
	const int num_points = 4;
	
	int min_along_axis(Vector3d& axis, float* rs=nullptr);
	int max_along_axis(Vector3d& axis, float* rs=nullptr);
	bool collides(BB& other);
	bool collides(BB& other,Vector3d& axis);
	void rotate(float angle); // +ve angle is clockwise

	// just for testing
	bool collided_flag;
	SDL_Color collided_colour;

	BB();
	virtual ~BB(){}
	void set_collided(bool value);
	void on_collided();
	void off_collided();
	void render(SDL_Renderer& ren, int x, int y);	
	bool mouse_drag(SDL_Event& ev,Coords* ref = nullptr);
};


class View_Play : public IView
{
public:
	// variables
	int disp_w;
	int disp_h;
	Timer* fps_timer;
	SDL_Surface* surface;	
	SDL_Texture* texture;
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;

	Collision mouse_intersect;
	Uint32 button;
	int mouse_x, mouse_y;

	// M Y  S T U F F   S T A R T	
	Mouse mouse;
	//Book2 book;
	const int num_boxes = 2;
	int selected_box_index;
	Polygon boxes[2];
	Polygon* selected_box;
	// M Y   S T U F F  E N D 

	// constructor and destructor
	View_Play(SDL_Window& win, SDL_Renderer& ren);
	~View_Play();
	// methods
	void on_start(SDL_Event& ev);
	void on_switch(SDL_Event& ev);
	void on_close(SDL_Event& ev);
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_events(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
};

