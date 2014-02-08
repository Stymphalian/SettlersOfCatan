#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <SDL.h>
#include "Logger.h"
#include "Collision.h"

Collision::Collision(){
	active = true;
	x = y = z = nullptr;
}
Collision::~Collision(){
	//Logger::getLog().log(Logger::DEBUG, "Collision destructor");
	rects.clear();
	x = y = z =nullptr;;
	active = false;
}

void Collision::hook(int* x, int* y,int* z){
	this->x = x;
	this->y = y;
	this->z = z;
}
void Collision::unhook(){
	this->x = nullptr;
	this->y = nullptr; 
	this->z = nullptr;
}
int Collision::getx(){
	if(x != nullptr){ return *x; }
	return 0;
}
int Collision::gety(){
	if(y != nullptr){ return *y; }
	return 0;
}
int Collision::getz(){
	if(z != nullptr){ return *z; }
	return 0;
}

int Collision::get_index_of_hit(){
	return index_of_hit;
	//index_of_hit = -1;
}
void Collision::clear(){
	rects.clear();
}
/*
Checks if Colllsions 'A' intersects with one of OUR collision hitboxes.
For now, this only supports rectangle collision
For now, this ignores z values of the Rectangles.
*/
bool Collision::collides(Collision& A){
	std::vector<SDL_Rect>::iterator it; // iterator for 'this'
	std::vector<SDL_Rect>::iterator it2;// iterator for 'A'

	int _x = getx();
	int _y = gety();
	SDL_Rect r = { _x,_y, 0, 0 };
	SDL_Rect ra = { A.getx(), A.gety(), 0, 0 };

	A.index_of_hit = -1;
	this->index_of_hit = -1;

	int internal_count = 0;
	for(it = rects.begin(); it != rects.end(); ++it){
		r.x =  _x + it->x;
		r.y =  _y + it->y;
		r.w = it->w;
		r.h = it->h;

		// test all the rectangles in A.
		int external_count = 0;
		for(it2 = A.rects.begin(); it2 != A.rects.end(); ++it2){
			ra.x = A.getx() + it2->x;
			ra.y = A.gety() + it2->y;
			ra.w = it2->w;
			ra.h = it2->h;			
			if(Collision::intersect_rect_rect(&r,&ra)){ 
				A.index_of_hit = external_count;
				index_of_hit = internal_count;
				return true;
			}
			external_count++;
		}
		internal_count++;
	}

	return false;
}

/*
for now, the z value is ignored.
*/
void Collision::add_rect(int x, int y,int z, int w, int h){
	SDL_Rect r = { x, y, w, h };
	rects.push_back(r);
}
/*
for now, the z values is ignored.
*/
bool Collision::remove_rect(int x, int y,int z, int w, int h){
	std::vector<SDL_Rect>::iterator  it;
	for(it = rects.begin(); it != rects.end(); ++it){
		if(it->x == x  && it->y == y && it->w == w  && it->h == h){
			rects.erase(it);
			return true;
		}
	}
	return false;
}

bool Collision::intersect_rect_rect(SDL_Rect* A, SDL_Rect* B, SDL_Rect* result){	
	// Invalid params
	if(A == nullptr || B == nullptr){ return false; }
	bool x_flag = true, y_flag = true;

	// Check for horizontal intersection
	if(A->x + A->w <= B->x || A->x >= B->x + B->w){
		x_flag = false;
	}
	// Check for vertical interseiction
	if(A->y + A->h <=  B->y || A->y >= B->y + B->h){
		y_flag = false;
	}

	// compute the intersecting rectangle
	if(x_flag && y_flag  && result != nullptr){
		result->x = (B->x > A->x) ? B->x : A->x;
		result->y = (B->y > A->y) ? B->y : A->y;
		result->w = (B->x + B->w < A->x + A->w) ? B->x + B->w : A->x + A->w;
		result->w -= result->x;
		result->h = (B->y + B->h < A->y + A->h) ? B->y + B->h : A->y + A->h;
		result->h -= result->y;
	}
	// There is only an intersection if both horizontal and vertical intersect
	return (x_flag && y_flag);
}
bool Collision::intersect_circle_circle(circle_t* A, circle_t* B){
	if(A == nullptr || B == nullptr){ return false; }
	// compare the distance between the two circles center and the 
	// distance of the sum of the radii
	int dist = (int)(sqrt(A->x*B->x + A->y + B->y));
	return (dist < A->radius + B->radius);
}
bool Collision::intersect_circle_rect(circle_t* A, SDL_Rect* B){
	if(A == nullptr || B == nullptr){ return false; }
	// make an oval out of the rectangles
	int x = (B->x + B->w / 2) / B->w;
	int y = (B->y + B->h / 2) / B->h;
	int rad_len = (int)(sqrt(x*x + y*y));
	rad_len += A->radius;
	int dist = (int)(sqrt(A->x*x + A->y*y));	
	// compare the distances between the circle and the oval-rectangle
	return (dist < rad_len);
	// this is inadequate... really
}


/*
// Collision Factory
Collision* Collision_Factory::make(const char* type){
	if(strncmp(type, "circle", strlen("circle")) == 0){
		return new Collision_Circle();
	} else if(strncmp(type, "rectangle", strlen("rectangle")) == 0){
		return new Collision_Rect();
	} else{
		return nullptr;
	}
}
// all the types of intersections
static bool intersect_circle_rect(Collision_Circle::circle_t* A, SDL_Rect* B){
	// this is inadequate... really
	int dist = (int)(sqrt(A->x*B->x + A->y*B->y));	
	int x = (B->x + B->w/2) / B->w;
	int y = (B->y  + B->h/2) / B->h;
	int rad_len = (int)(sqrt(x*x + y*y));
	rad_len += A->radius;
	return (dist < rad_len);
}
static bool intersect_rect_rect(SDL_Rect* A, SDL_Rect* B, SDL_Rect* result = nullptr){
	// Invalid params
	if(A == nullptr || B == nullptr){ return false; }
	bool x_flag = true, y_flag = true;

	// Check for horizontal intersection
	if(A->x + A->w < B->x || A->x > B->x + B->w){
		x_flag = false;
	}
	// Check for vertical interseiction
	if(A->y + A->h <  B->y || A->y > B->y + B->h){
		y_flag = false;
	}

	// There is only an intersection if both horizontal and vertical intersect
	if(x_flag && y_flag  && result != nullptr){
		result->x = (B->x > A->x) ? B->x : A->x;
		result->y = (B->y > A->y) ? B->y : A->y;
		result->w = (B->x + B->w < A->x + A->w) ? B->x + B->w : A->x + A->w;
		result->w -= result->x;
		result->h = (B->y + B->h < A->y + A->h) ? B->y + B->h : A->y + A->h;
		result->h -= result->y;
	}
	return (x_flag && y_flag);

	return false;
}
static bool intersect_circle_circle(Collision_Circle::circle_t* A, Collision_Circle::circle_t* B){
	int dist = (int) (sqrt(A->x*B->x + A->y + B->y));
	return (dist < A->radius + B->radius);
}

// Collision Circles
Collision_Circle::Collision_Circle() : Collision(){}

Collision_Circle::~Collision_Circle(){
	circles.clear();
}
void Collision_Circle::add(int x, int y, int r){
	circle_t c = { x, y, r };
	circles.push_back(c);
}
void Collision_Circle::remove(int x, int y, int r){
	std::vector<circle_t>::iterator it;
	for(it = circles.begin(); it != circles.end(); ++it){
		if((*it).x == x && (*it).y == y && (*it).radius == r){
			circles.erase(it);
			return;
		}
	}
}

bool Collision_Circle::collides(Collision_Circle& other){
	std::vector<circle_t>::iterator it;
	std::vector<circle_t>::iterator it2;
	for(it = circles.begin(); it != circles.end(); ++it){
		for(it2 = other.circles.begin(); it2 != other.circles.end(); ++it2){
			if(intersect_circle_circle( &(*it),&(*it2)) ){ return true; }
		}
	}
	return false;
}
bool Collision_Circle::collides(Collision_Rect& other){
	std::vector<circle_t>::iterator it;
	std::vector<SDL_Rect>::iterator it2;
	for(it = circles.begin(); it != circles.end(); ++it){
		for(it2 = other.rects.begin(); it2 != other.rects.end(); ++it2){
			if(intersect_circle_rect( &(*it), &(*it2) ) ){ return true; }
		}
	}
	return false;	
}

// Collision Rectangles
Collision_Rect::Collision_Rect(): Collision(){}
Collision_Rect::~Collision_Rect(){
	rects.clear();
}
// methods
bool Collision_Rect::collides(Collision_Circle& other){
	std::vector<SDL_Rect>::iterator it;
	std::vector<Collision_Circle::circle_t>::iterator it2;	
	for(it = rects.begin(); it != rects.end(); ++it){
		for(it2 = other.circles.begin(); it2 != other.circles.end(); ++it2){
			if(intersect_circle_rect(&(*it2), &(*it))){ return true; }
		}
	}
	return false;
}
bool Collision_Rect::collides(Collision_Rect& other){
	std::vector<SDL_Rect>::iterator it;
	std::vector<SDL_Rect>::iterator it2;
	for(it = rects.begin(); it != rects.end(); ++it){
		for(it2 = other.rects.begin(); it2 != other.rects.end(); ++it2){
			if(intersect_rect_rect(&(*it), &(*it2))){ return true; }
		}
	}
	return false;
}

void Collision_Rect::add(int x, int y, int w, int h){
	SDL_Rect rect = { x, y, w, h };
	rects.push_back(rect);
}
void Collision_Rect::remove(int x, int y, int w, int h){
	std::vector<SDL_Rect>::iterator it;
	for(it = rects.begin(); it != rects.end(); ++it){
		if(it->x == x && it->y == y && it->w == w && it->h == h){
			rects.erase(it);
			return;
		}
	}
}
*/