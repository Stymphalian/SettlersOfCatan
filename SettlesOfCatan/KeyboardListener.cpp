#include <vector>
#include "KeyboardListener.h"

Keyboardable::Keyboardable(){}
Keyboardable::~Keyboardable(){}
void Keyboardable::notify_on_keydown(int code, void* data){
	std::vector<IKeyboardListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_keydown(this, code, data);
	}
}
void Keyboardable::notify_on_keyup(int code, void* data){
	std::vector<IKeyboardListener*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		(*it)->update_on_keyup(this, code, data);
	}
}
