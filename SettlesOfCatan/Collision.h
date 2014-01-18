#pragma once
#include <vector>

class circle_t{
public:
	int x, y, radius;
};

class Collision{
public:
	//variables		
	bool active;
	std::vector<SDL_Rect> rects;
	// constructor and destructor
	Collision();
	virtual ~Collision();
	// methods
	bool collides(Collision& A);
	void add_rect(int x, int y, int z, int w, int h ); // these rects are relative to the origin point
	bool remove_rect(int x, int y,int z, int w, int h);
	int getx(); int gety(); int getz();
	void hook(int* x, int* y, int* z=nullptr);
	void unhook();
	// static methods
	static bool intersect_rect_rect(SDL_Rect* A, SDL_Rect*B,SDL_Rect* result= nullptr);
	static bool intersect_circle_circle(circle_t* A, circle_t* B);
	static bool intersect_circle_rect(circle_t* A, SDL_Rect* B);
private:
	int *x, *y, *z;
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