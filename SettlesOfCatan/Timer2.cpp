#include "Timer2.h"
#include <SDL.h>


Timer2::Timer2()
{
	start_tick = 0;
	pause_tick = 0;
	timer_state = STOP;
}
Timer2::~Timer2() {}

int Timer2::get_ticks(){
	if(timer_state == STOP){
		// if the timer is stopped
		return 0;
	} else if(timer_state == PAUSE){
		// timer is currently paused
		pause_tick = SDL_GetTicks();
	} else if(timer_state == RUNNING){
		// the timer is running, return number of ticks since start
		return SDL_GetTicks() - start_tick;
	}
	return 0;
}

void Timer2::start(){
	if(timer_state == RUNNING){ return; }
	timer_state = RUNNING;
	start_tick = SDL_GetTicks();
}
void Timer2::stop(){
	timer_state = STOP;
	start_tick = 0;
}
void Timer2::pause(){
	if(timer_state == RUNNING){
		timer_state = PAUSE;
		pause_tick = SDL_GetTicks();
	}
}
void Timer2::unpause(){
	if(timer_state == PAUSE){
		timer_state = RUNNING;
		start_tick = SDL_GetTicks() - pause_tick;
		pause_tick = 0;
	}
}

bool Timer2::is_running(){ return (timer_state == RUNNING); }
bool Timer2::is_paused() { return (timer_state == PAUSE); }
bool Timer2::is_stopped(){ return (timer_state == STOP); }


