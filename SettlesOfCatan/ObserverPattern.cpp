#include <vector>
#include "ObserverPattern.h"

Observable::Observable(){ _changed = false; }
Observable::~Observable(){ observers.clear(); }

bool Observable::subscribe(IObserver* observer){
	// check for duplicates
	for(unsigned i = 0; i < observers.size(); ++i){
		if(observer == observers[i]){ return false; }
	}
	observers.push_back(observer);
	return true;
}
bool Observable::unsubscribe(IObserver* observer){
	std::vector<IObserver*>::iterator it;
	for(it = observers.begin(); it != observers.end(); ++it){
		if((*it) == observer){
			observers.erase(it);
			return true;
		}
	}
	// none were found
	return false;
}
bool Observable::clear_subscribers(){
	observers.clear();
	return true;
}
unsigned Observable::num_subscribers(){
	return observers.size();
}
void Observable::notify(void* data){
	if(_changed == false){ return; }
	for(unsigned i = 0; i < observers.size(); ++i){
		observers[i]->update(this,data);
	}
	_changed = false;	
}
bool Observable::has_changed(){return _changed;}
void Observable::mark_changed(){ _changed = true; }
void Observable::unmark_changed(){ _changed = false; }
