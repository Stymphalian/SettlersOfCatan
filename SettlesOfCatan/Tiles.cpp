#include <cstdlib>
#include <string>
#include "Tiles.h"
#include "Logger.h"

// constructor and destructors
Tiles::Tiles(){
	active = 0;
	type = 0;
	roll = 0;
	ring_level = 0;
	reset_vertices_faces();
}
Tiles::~Tiles(){
	//Logger::getLog("jordan.log").log(Logger::DEBUG, "Tiles destructor");
}


// methods
void Tiles::init(int type, int roll){
	this->type = type;
	this->roll = roll;
}
void Tiles::reset_vertices_faces(){
	// set them to -1, such that we know the vertex hasnen't been used yet	
	memset(vertices, -1, sizeof(int)* 6);
	memset(faces, -1, sizeof(int)* 6);
}
bool Tiles::is_water_tile(int type){
	return (type >= Tiles::WATER_TILE);
}

int Tiles::get_direction_of_vertex(int vertex_num){
	for(int i = 0; i < 6; ++i){
		if(vertices[i] == vertex_num){return i;}
	}
	return -1;
}
int Tiles::get_direction_of_face(int vertex_num){
	for(int i = 0; i < 6; ++i){
		if(faces[i] == vertex_num){ return i; }
	}
	return -1;
}
/*
Adjacency table for hex tiles
even			odd
X		Y		X		Y
NE		1		-1		0		-1
E		1		0		1		0
SE		1		1		0		1
SW		0		1		-1		1
W		-1		0		-1		0
NW		0		-1		-1		-1
*/
bool Tiles::get_adjacent(int dir, int x, int y, int* dx, int* dy){
	if(!dx || !dy){ return false; }
	*dx = hex_dx(dir, (y % 2 == 1)) + x;
	*dy = hex_dy(dir) + y;
	return true;
}
int Tiles::hex_dy(int dir){
	switch(dir){
	case(HEXEAST) :
	case(HEXWEST) : return 0;
	case(HEXSOUTHEAST) :
	case(HEXSOUTHWEST) : return 1;
	case(HEXNORTHEAST) :
	case(HEXNORTHWEST) : return -1;
	default:{return 0; }
	}
}
int Tiles::hex_dx(int dir, bool odd_row){
	switch(dir){
	case(HEXNORTHEAST) :
	case(HEXSOUTHEAST) : return (odd_row) ? 1 : 0;
	case(HEXSOUTHWEST) :
	case(HEXNORTHWEST) : return (odd_row) ? 0 : -1;
	case(HEXEAST) : return 1;
	case(HEXWEST) : return -1;
	default:{return 0; }
	}
}