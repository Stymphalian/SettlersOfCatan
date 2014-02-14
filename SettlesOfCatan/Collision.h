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
/*
#include <vector>
class Collision;
class Collision_Circle;
class Collisson_Rect;

class Collision_Factory{
public:
	static Collision* make(const char* type);
private:
	Collision_Factory(){}
	virtual ~Collision_Factory();
	Collision_Factory(const Collision_Factory&);
	void operator= (Collision_Factory const&);

};

class Collision{
public:
	Collision(){}
	virtual ~Collision();
	virtual bool collides(Collision_Circle& other) = 0;
	virtual bool collides(Collision_Rect& other) = 0;
};

class Collision_Circle : public Collision{
public:
	// variables
	struct circle_t{int x, y, radius;};
	std::vector<struct circle_t> circles;

	// constructor and destructor
	Collision_Circle();
	virtual ~Collision_Circle();
	// methods
	virtual bool collides(Collision_Circle& other);
	virtual bool collides(Collision_Rect& other);
	void add(int x, int y, int r);
	void remove(int x, int y, int r);
};

class Collision_Rect : public Collision{
public:
	// variables
	std::vector<SDL_Rect> rects;
	// constructor and destructor
	Collision_Rect();	
	virtual ~Collision_Rect();
	// methods
	virtual bool collides(Collision_Circle& other);
	virtual bool collides(Collision_Rect& other);
	void add(int x, int y, int w, int h);
	void remove(int x, int y, int w, int h);
};

*/