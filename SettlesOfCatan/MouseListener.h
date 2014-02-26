#pragma once
#include "ObserverPattern.h"

class Mouseable;
class IMouseListener : public IObserver<Mouseable>{
public:
	IMouseListener(){}
	virtual ~IMouseListener(){}
	void update(Mouseable* origin, int code, void* data){}
	virtual void update_on_mouse_motion(Mouseable* origin, int code, void* data) = 0;
	virtual void update_on_mouse_drag(Mouseable* origin, int code, void* data) = 0;
	virtual void update_on_mouse_buttondown(Mouseable* origin, int code, void* data) = 0;
	virtual void update_on_mouse_buttonup(Mouseable* origin, int code, void* data) = 0;
};

class Mouseable : public IObservable<IMouseListener>{
public:
	Mouseable(){}
	virtual ~Mouseable(){}
	void notify_mouse_motion(int code, void* data);		
	void notify_mouse_drag(int code, void* data);
	void notify_mouse_buttondown(int code, void* data);		
	void notify_mouse_buttonup(int code, void* data);
	void notify(int code, void* data){}
protected:
};

