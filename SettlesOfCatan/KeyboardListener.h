#pragma once
#include "ObserverPattern.h"

class Keyboardable;
class IKeyboardListener : IObserver<Keyboardable>{
public:
	IKeyboardListener(){}
	virtual ~IKeyboardListener(){}

	virtual void update(Keyboardable* origin, int code, void* data){}
	virtual void update_on_keydown(Keyboardable* origin, int code, void* data)=0;
	virtual void update_on_keyup(Keyboardable* origin, int code, void* data)=0;
};

class Keyboardable : IObservable<IKeyboardListener> {
public:
	Keyboardable();
	virtual ~Keyboardable();
	virtual void notify(int code, void* data){}
	virtual void notify_on_keydown(int code, void* data);
	virtual void notify_on_keyup(int code, void* data);
};

