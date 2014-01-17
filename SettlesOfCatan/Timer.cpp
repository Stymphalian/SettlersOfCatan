#include <SDL.h>
#include "Logger.h"
#include "Timer.h"

// Constructor
Timer::Timer(Uint32 interval,Uint32 user_event_type){
	this->interval = interval;
	this->running = false;
	event_type = user_event_type;
	start(); // Start the timer
}
//Destructor
Timer::~Timer(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Timer destructor");
	stop(); // Stop the timer
}

// Member methods
bool Timer::start(){
	timer_id = SDL_AddTimer(interval, Timer::timer_callback, &event_type);
	if(timer_id == 0){
		Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "SDL_AddTimer");
		return false;
	}
	running = true;
	return true;
}
bool Timer::stop(){
	if(running == false){ return true; }
	if(SDL_RemoveTimer(timer_id) == SDL_FALSE){
		Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "SDL_RemoveTimer %d",timer_id);
		return false;
	}
	running = false;
	return true;
}
// Call back function which generates the events
Uint32 Timer::timer_callback(Uint32 interval, void* event_type){
	if(event_type == nullptr){ return 0; }
	SDL_Event event;
	SDL_zero(event);
	event.type = *(Uint32*)event_type;
	event.user.type = *(Uint32*)event_type;
	event.user.code = 0;
	event.user.data1 = 0;
	event.user.data2 = 0;
	SDL_PushEvent(&event);
	return interval; // restart the timer
}

// Static get()  for Singleton pattern
TimerFactory& TimerFactory::get(){
	static TimerFactory instance(0);
	return instance;
}

// Factory 
Timer* TimerFactory::make(Uint32 interval){
	return new Timer(interval,m_user_event_type_timer);
}
Uint32 TimerFactory::event_type(){
	return m_user_event_type_timer;
}
// Constructor, and Destructors for Singleton Pattern
TimerFactory::TimerFactory(int param){
	m_user_event_type_timer = SDL_RegisterEvents(1);
	if(m_user_event_type_timer == ((Uint32)-1)){
		Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "SDL_RegisterEvents");
		m_user_event_type_timer = 0;
	}
}
TimerFactory::~TimerFactory(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Timer Factory destructor");
}


/*
Uint32 fps_timer_callback(Uint32 interval, void* param){
SDL_Event event;
event.type = SDL_USEREVENT;
event.user.type = *(Uint32*)param;
event.user.code = 0;
event.user.data1 = 0;
event.user.data2 = 0;
SDL_PushEvent(&event);
return interval; // Restart the timer
}
*/