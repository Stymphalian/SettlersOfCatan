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
int Timer::get_timerid(){ return timer_id; }
int Timer::isrunning() { return running; }
Uint32 Timer::get_event_type(){ return event_type; };
Uint32 Timer::get_interval(){ return interval; };

// Member methods
bool Timer::start(){
	timer_id = SDL_AddTimer(interval, Timer::timer_callback,this);
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
Uint32 Timer::timer_callback(Uint32 interval, void* param){
	if(param == nullptr){ return 0; }
	Timer* self = (Timer*)param;
	SDL_Event event;
	SDL_zero(event);
	event.type = self->event_type;
	event.user.type = self->event_type;
	event.user.code = 0;
	event.user.data1 = param;
	event.user.data2 = 0;
	SDL_PushEvent(&event);
	return interval; // restart the timer
}

// -- T I M E R    F A c T O R Y  --
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
		Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "TimerFactory::SDL_RegisterEvents");
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