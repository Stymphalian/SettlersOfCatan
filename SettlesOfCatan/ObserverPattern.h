#pragma once
#include <vector>

template <class T> class Observable;
template <class T>
class IObserver{
public:	
	virtual void update(T* origin,int code, void* data) = 0;
private:
};

template <class T>
class IObservable{
private:
	bool _changed;
protected:
	std::vector<T*> observers;
	void mark_changed(){ _changed = true; }
	void unmark_changed(){ _changed = false; }
public:
	IObservable(){ 
		_changed = false;
	}

	virtual ~IObservable(){ 
		observers.clear();
	}

	virtual bool subscribe(T* observer) {
		// check for duplicates
		for(unsigned i = 0; i < observers.size(); ++i){
			if(observer == observers[i]){ return false; }
		}
		observers.push_back(observer);
		return true;
	}

	virtual bool unsubscribe(T* observer){
		std::vector<T*>::iterator it;
		for(it = observers.begin(); it != observers.end(); ++it){
			if((*it) == observer){
				observers.erase(it);
				return true;
			}
		}
		// none were found
		return false;
	}

	virtual bool clear_subscribers(){
		observers.clear();
		return true;
	}

	virtual unsigned num_subscribers(){
		return observers.size();
	}

	virtual bool has_changed(){ 
		return _changed; 
	}
	
	virtual void notify(int code, void* data)=0;
};
