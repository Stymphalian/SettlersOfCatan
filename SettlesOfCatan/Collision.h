#pragma once
#include <SDL.h>
#include <vector>
#include "Coords.h"

class circle_t{
public:
	int x, y, radius;
};

class Collision{
	class collision_rect{
	public:
		collision_rect(){ this->target = nullptr; }
		virtual ~collision_rect(){ this->target = nullptr; }
		// ICoordinates interface
		int x(){ return x_; }
		int y(){ return y_; }
		int z(){ return z_; }
		int w(){ return (this->target != nullptr)? target->w() : w_; }
		int h(){ return (this->target != nullptr) ? target->h() : h_; }
		void set_dim(ICoords* c){ target = c; }
		
		void init(int x, int y, int z, int w, int h){
			this->x_ = x;
			this->y_ = y;
			this->z_ = z;
			this->w_ = w;
			this->h_ = h;
		}		
	protected:
		int x_, y_, z_, w_, h_;
		ICoords* target;
	};

public:
	//variables		
	bool active;
	std::vector<collision_rect> rects;
	int index_of_hit;

	// constructor and destructor
	Collision();
	virtual ~Collision();

	// methods
	void clear();
	int get_index_of_hit();
	bool collides(Collision& A);
	void add_rect(int x, int y, int z, int w, int h ); // these rects are relative to the origin point
	void add_mutable_rect(ICoords* coords);

	bool remove_rect(int x, int y,int z, int w, int h);	
	void hook(int* x, int* y, int* z=nullptr);
	void hook(ICoords* coords);

	void unhook();
	int getx();
	int gety();
	int getz();

	// static methods
	static bool intersect_rect_rect(SDL_Rect* A, SDL_Rect*B,SDL_Rect* result= nullptr);
	static bool intersect_circle_circle(circle_t* A, circle_t* B);
	static bool intersect_circle_rect(circle_t* A, SDL_Rect* B);
private:
	const int *x;
	const int *y;
	const int *z;
	ICoords *coord;
};