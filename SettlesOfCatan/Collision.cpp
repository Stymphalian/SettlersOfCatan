#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <SDL.h>
#include "Logger.h"
#include "Coords.h"
#include "Collision.h"


Collision::Collision(){
	active = true;
	x = y = z = nullptr;
	rects.clear();
	index_of_hit = -1;
	coord = nullptr;
}
Collision::~Collision(){
	//Logger::getLog().log(Logger::DEBUG, "Collision destructor");
	rects.clear();
	x = y = z =nullptr;
	active = false;
	index_of_hit = -1;
	coord = nullptr;
}

// Can only either specific x, y, z 
// or a Coords object
void Collision::hook(ICoords* coord){
	this->coord = coord;
	this->x = nullptr;
	this->y = nullptr;
	this->z = nullptr;
}
// Can only either specific x, y, z 
// or a Coords object
void Collision::hook(int* x, int* y,int* z){
	this->x = x;
	this->y = y;
	this->z = z;
	this->coord = nullptr;
}
void Collision::unhook(){
	this->x = nullptr;
	this->y = nullptr; 
	this->z = nullptr;
	this->coord = nullptr;
}

int Collision::getx(){
	if(x != nullptr){ return *x; }
	if(this->coord != nullptr){ return coord->x(); }
	return 0;
}
int Collision::gety(){
	if(y != nullptr){ return *y; }
	if(this->coord != nullptr){ return coord->y(); }
	return 0;
}
int Collision::getz(){
	if(z != nullptr){ return *z; }
	if(this->coord != nullptr){ return coord->z(); }
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
	std::vector<collision_rect>::iterator us; // iterator for 'this'
	std::vector<collision_rect>::iterator them;// iterator for 'A'

	int us_x = getx();
	int us_y = gety();
	int them_x = A.getx();
	int them_y = A.gety();
	SDL_Rect us_rect, them_rect;

	A.index_of_hit = -1;
	this->index_of_hit = -1;

	int internal_count = 0;
	for(us = rects.begin(); us != rects.end(); ++us){
		us_rect.x =  us_x + us->x();
		us_rect.y =  us_y + us->y();
		us_rect.w = us->w();
		us_rect.h = us->h();

		// test all the rectangles in A.
		int external_count = 0;
		for(them = A.rects.begin(); them != A.rects.end(); ++them){
			them_rect.x = A.getx() + them->x();
			them_rect.y = A.gety() + them->y();
			them_rect.w = them->w();
			them_rect.h = them->h();			
			if(Collision::intersect_rect_rect(&us_rect,&them_rect)){ 
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
	static collision_rect rect;
	rect.init(x, y, z, w, h);	
	rects.push_back(rect);
}
/*
for now, the z value is ignored.
*/
void Collision::add_mutable_rect(ICoords* coords){
	if(coords == nullptr){ return; }	
	static collision_rect rect;
	rect.init(coords->x(), coords->y(), coords->z(),-1,-1);
	rect.set_dim(coords);
	rects.push_back(rect);
}
/*
for now, the z values is ignored.
*/
bool Collision::remove_rect(int x, int y,int z, int w, int h){
	std::vector<collision_rect>::iterator  it;
	for(it = rects.begin(); it != rects.end(); ++it){
		if(it->x() == x  && it->y() == y && it->w() == w  && it->h() == h){
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
