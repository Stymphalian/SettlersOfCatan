#include <string>
#include "CheckBox.h"
#include "Collision.h"


CheckBox::CheckBox(){
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->w = 0;
	this->h = 0;
	_ticked = false;
	_group = nullptr;
}
CheckBox::~CheckBox(){
	if(_group != nullptr){
		_group->remove_checkbox(this);
		_group = nullptr;
	}	
}

/*
initilize the checkboxes.
This sets the x,y,z position as well as the width and height of the box.
Default tick value is off
Default is not apart of a group. ( sets to nullptr)
*/
void CheckBox::init(int x, int y, int z, int w, int h, bool ticked){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
	this->h = h;
	_ticked = ticked;
	_group = nullptr;

	this->hitbox.hook(&this->x, &this->y,&this->z);
	this->hitbox.add_rect(0, 0, 0, this->w, this->h);
}

void CheckBox::click(){
	// If the checkbox is a part of a group,then handle that logic here.
	if(_group != nullptr){
		_group->handle_clicked_checkbox(this);		
	}

	_ticked = (_ticked) ? false : true;
	if(_ticked){
		on_tick_action();
	} else{ off_tick_action(); }
}

bool CheckBox::is_ticked(){
	return _ticked;
}

void CheckBox::set_label(const char* label){
	_label = (label == nullptr) ? "" : label;
}
std::string CheckBox::get_label(){
	return _label;
}




// --------------------------------------------------
// C H E C K B O X _ G R O U P
// --------------------------------------------------
CheckBox_Group::CheckBox_Group(){
	init(0, 0, 0);
}

CheckBox_Group::~CheckBox_Group(){
	std::vector<CheckBox*>::iterator it;
	for(it = _group.begin(); it != _group.end(); ++it){
		(*it)->_group = nullptr;
	}
	_group.clear();
}

void CheckBox_Group::init(int x, int y, int z){
	this->hitbox.rects.clear();
	this->x = x;
	this->y = y;
	this->z = z;
	//this->w = 0;
	//this->h = 0;
	this->can_only_select_one = false;
	this->_group.clear();
	//this->_direction = CheckBox_Group::HORIZONTAL;

	this->hitbox.hook(&this->x, &this->y, &this->z);
	this->hitbox.add_rect(0, 0, 0, 0, 0);
}

std::list<CheckBox*> CheckBox_Group::get_selected(){
	std::list<CheckBox*> selected;
	for(int i = 0; i < (int)_group.size(); ++i){
		if(_group[i]->_ticked == true){
			selected.push_back(_group[i]);
			if(can_only_select_one){ return selected; }
		}
	}
	return selected;
}

bool CheckBox_Group::add_checkbox(CheckBox* box){
	if(box == nullptr){ return false; }
	// make sure that the checkbox is not already attached to a gorup.
	if(box->_group != nullptr){ return false; }

	// We can't have duplicate entries into the list.
	std::vector<CheckBox*>::iterator it;
	for(it = _group.begin(); it != _group.end(); ++it){
		if((*it) == box){ return false; }
	}
	// add the check box to the list.
	box->_group = this;
	_group.push_back(box);

	return true;
}

bool CheckBox_Group::remove_checkbox(CheckBox* box){
	if(box == nullptr){ return false; }
	// Check for the box in the group list.
	std::vector<CheckBox*>::iterator it;
	it = _group.begin();
	while(it != _group.end()){	
		if((*it) == box){
			(*it)->_group = nullptr;

			_group.erase(it++);
			return true;
		}
		++it;
	}
	return false;
}
void CheckBox_Group::clear(){
	std::vector<CheckBox*>::iterator it;
	for(it = _group.begin(); it != _group.end(); ++it){
		(*it)->_group = nullptr;
	}
	_group.clear();
}

void CheckBox_Group::handle_clicked_checkbox(CheckBox* box){
	if(can_only_select_one){
		for(int i = 0; i < (int)_group.size(); ++i){
			if(_group[i] == box){ continue; }
		
			_group[i]->_ticked = false;
			_group[i]->off_tick_action();
		}	
	} 
}

//bool CheckBox_Group::is_horizontal(){ return (_direction == CheckBox_Group::HORIZONTAL); }
//bool CheckBox_Group::is_vertical(){ return (_direction == CheckBox_Group::VERTICAL); }
//void CheckBox_Group::set_horizontal(){ _direction = CheckBox_Group::HORIZONTAL; }
//void CheckBox_Group::set_vertical(){ _direction = CheckBox_Group::VERTICAL; }
