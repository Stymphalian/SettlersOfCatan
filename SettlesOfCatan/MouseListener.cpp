#include "MouseListener.h"

//-------------------------------------------
// M O U S E A B L E
//-------------------------------------------
void Mouseable::notify_mouse_motion(int code, void* data){
	std::vector<IMouseListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_mouse_motion(this, code, data);
	}
}
void Mouseable::notify_mouse_drag(int code, void* data){
	std::vector<IMouseListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_mouse_drag(this, code, data);
	}
}
void Mouseable::notify_mouse_buttondown(int code, void* data){
	std::vector<IMouseListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_mouse_buttondown(this, code, data);
	}
}
void Mouseable::notify_mouse_buttonup(int code, void* data){
	std::vector<IMouseListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_mouse_buttonup(this, code, data);
	}
}