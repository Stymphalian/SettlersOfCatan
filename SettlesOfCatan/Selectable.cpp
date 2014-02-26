#include "Selectable.h"

Selectable::Selectable(){_selected_flag = false;}
Selectable::~Selectable(){}
bool Selectable::is_selected(){ return _selected_flag; }
void Selectable::set_selected(bool value){
	if(value == true){
		if(_selected_flag == false){ on_selected(); }
	} else{
		if(_selected_flag == true) { off_selected(); }
	}
	_selected_flag = value;
}
void Selectable::on_selected(){};
void Selectable::off_selected(){};
