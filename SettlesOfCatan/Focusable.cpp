#include "Focusable.h"


Focusable::Focusable(){
	_focus_flag = false;
}
Focusable::~Focusable(){}
bool Focusable::has_focus(){return _focus_flag;}
void Focusable::set_focus(bool value){
	if(value == false){
		if(_focus_flag == true){ off_focus(); }
	} else{
		if(_focus_flag == false){ on_focus(); }
	}
	_focus_flag = value;
}