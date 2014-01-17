#pragma once
class Timer2
{
public:
	Timer2();
	virtual ~Timer2();

	int get_ticks();
	void start();
	void stop();
	void pause();
	void unpause();
	bool is_running();
	bool is_paused();
	bool is_stopped();
private:
	enum state {RUNNING,PAUSE,STOP};
	unsigned long start_tick;
	unsigned long pause_tick;
	unsigned timer_state;
};

