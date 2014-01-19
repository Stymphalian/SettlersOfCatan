#pragma once

#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include "Logger.h"

class Timer{
private:
	int timer_id;
	bool running;
	Uint32 event_type;
	Uint32 interval;
public:
	// Variables
	int get_timerid();
	int isrunning();
	Uint32 get_event_type();
	Uint32 get_interval();

	// Constructor  & Destructor
	Timer(Uint32 interval,Uint32 user_event_type);
	virtual ~Timer();

	// Member methods
	bool start();
	bool stop();

	// Call back function which generates the events
	static Uint32 timer_callback(Uint32 interval, void* param);
};

class TimerFactory{
public:
	// Static get()  for Singleton pattern
	static TimerFactory& get();
	Timer* make(Uint32 interval);
	Uint32 event_type();
private:
	// Variables
	Uint32 m_user_event_type_timer;

	// Constructor, and Destructors for Singleton Pattern
	TimerFactory(){}
	TimerFactory(int param);
	virtual ~TimerFactory();
	TimerFactory(const TimerFactory& orig);
	void operator=(TimerFactory const&);
};