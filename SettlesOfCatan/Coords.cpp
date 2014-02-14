#include "Coords.h"

// --------------------------------------------------------
// C O O R D S 
// --------------------------------------------------------
Coords::Coords(ICoords* parent){
	this->_x = 0;
	this->_y = 0;
	this->_z = 0;
	this->_w = 0;
	this->_h = 0;	
	this->parent = parent;
	this->rel_data = nullptr;
}
Coords::~Coords(){
	this->parent = nullptr;
	delete rel_data;
}
void Coords::init(int x, int y, int z, int w, int h){
	this->_x = x;
	this->_y = y;
	this->_z = z;
	this->_w = w;
	this->_h = h;
}
void Coords::set_parent(ICoords* parent){this->parent = parent;}
ICoords* Coords::get_parent(){return this->parent;}

void Coords::x(int value){ _x = value; }
void Coords::y(int value){ _y = value; }
void Coords::z(int value){ _z = value; }
void Coords::w(int value){ _w = value; }
void Coords::h(int value){ _h = value; }

int Coords::x(){ 
	if(rel_data != nullptr  && parent != nullptr){
		if((rel_data->relative_mask & relative_coordinate_data::RELX) != 0){
			return  (int)(this->parent->w() * rel_data->rel_pos[0]);
		} else if((rel_data->relative_mask & relative_coordinate_data::RELX_px) != 0){
			return  (int)(this->parent->w() - rel_data->rel_pos[0]);
		}			
	}
	return _x;	
}
int Coords::y(){
	if(rel_data != nullptr  && parent != nullptr){
		if((rel_data->relative_mask & relative_coordinate_data::RELY) != 0){
			return  (int)(this->parent->h() * rel_data->rel_pos[1]);
		} else if((rel_data->relative_mask & relative_coordinate_data::RELY_px) != 0){
			return  (int)(this->parent->h() - rel_data->rel_pos[1]);
		}
	}
	return _y;
}
int Coords::z(){
	return _z;
}
int Coords::w(){
	if(rel_data != nullptr  && parent != nullptr){
		if((rel_data->relative_mask & relative_coordinate_data::RELW) != 0){
			return  (int)(this->parent->w() * rel_data->rel_pos[2]);
		} else if((rel_data->relative_mask & relative_coordinate_data::RELW_px) != 0){
			return  (int)(this->parent->w() - rel_data->rel_pos[2]);
		}
	}
	return _w;
}
int Coords::h(){
	if(rel_data != nullptr  && parent != nullptr){
		if((rel_data->relative_mask & relative_coordinate_data::RELH) != 0){
			return  (int)(this->parent->h() * rel_data->rel_pos[3]);
		} else if((rel_data->relative_mask & relative_coordinate_data::RELH_px) != 0){
			return  (int)(this->parent->h() - rel_data->rel_pos[3]);
		}
	}
	return _h;
}

int Coords::disp_x(){
	return this->x() + ((this->parent != nullptr) ? this->parent->disp_x() : 0);
}
int Coords::disp_y(){
	return this->y() + ((this->parent != nullptr) ? this->parent->disp_y() : 0);
}
int Coords::disp_z(){
	return this->z() + ((this->parent != nullptr) ? this->parent->disp_z() : 0);
}


void Coords::disable_relative_data(){
	delete this->rel_data;
	this->rel_data = nullptr;
}
void Coords::set_relative_x(float percent){
	set_relative(0, relative_coordinate_data::RELX, relative_coordinate_data::RELX_px, percent);
}
void Coords::set_relative_y(float percent){
	set_relative(1, relative_coordinate_data::RELY, relative_coordinate_data::RELY_px, percent);
}
void Coords::set_relative_w(float percent){
	set_relative(2, relative_coordinate_data::RELW, relative_coordinate_data::RELW_px, percent);
}
void Coords::set_relative_h(float percent){
	set_relative(3, relative_coordinate_data::RELH, relative_coordinate_data::RELH_px, percent);
}
void Coords::set_relative_x(int percent){
	set_relative(0, relative_coordinate_data::RELX_px, relative_coordinate_data::RELX, (float)percent);
}
void Coords::set_relative_y(int percent){
	set_relative(1, relative_coordinate_data::RELY_px, relative_coordinate_data::RELY, (float)percent);
}
void Coords::set_relative_w(int percent){
	set_relative(2, relative_coordinate_data::RELW_px, relative_coordinate_data::RELW, (float)percent);
}
void Coords::set_relative_h(int percent){
	set_relative(3, relative_coordinate_data::RELH_px, relative_coordinate_data::RELH, (float)percent);
}

void Coords::set_relative(int pos, unsigned set_mask, unsigned unset_mask,float value){
	if(this->rel_data == nullptr){
		this->rel_data = new relative_coordinate_data();
	}
	this->rel_data->rel_pos[pos] = value;
	this->rel_data->set_mask(set_mask);
	this->rel_data->unset_mask(unset_mask);
}

