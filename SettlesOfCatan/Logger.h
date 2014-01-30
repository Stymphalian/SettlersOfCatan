#pragma once

/*
Logger class
Can be used to log information to a file/console window

Supports various debug levels.
	NONE
	DEBUG
	ERROR
	WARN
	INFO
log( level, 
*/

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <iostream>
#include <ctime>
#include <SDL.h>

class Logger{
public:
	// static enumerations and getLog()
	enum levels{ NONE=0, DEBUG, ERROR, WARN, INFO };
	static Logger& getLog(const char* file);

	// public methods
	void set_level(Logger::levels level);
	void set_deepest_level_allowed(Logger::levels level);
	void log(const char* format, ...);	
	void log(Logger::levels level, const char* format, ...);
	void log_nnl(Logger::levels level, const char* format, ...);
	void log_append(Logger::levels level, const char* format, ...);
	void SDL_log(Logger::levels level, const char* format, ...);
	void Mix_log(Logger::levels level, const char* format, ...);
	void TTF_log(Logger::levels level, const char* format, ...);
	void IMG_log(Logger::levels level, const char* format, ...);
private:
	// variables
	bool good_flag;
	bool output_to_stderr_flag;
	FILE* file_stream;
	std::string filename;
	enum levels level;
	enum levels depth_level;

	// helper methods
	std::string get_current_time();
	std::string get_level_from_enum(Logger::levels level);
	void _log(Logger::levels level, const char* format, va_list args);
	void _log_nnl(Logger::levels level, const char* format, va_list args);
	void _log_append(Logger::levels level, const char* format, va_list args);

	// private constructors and destructors
	Logger(){}
	Logger(const char* filename,bool output_to_stderr_aswell);
	Logger(const Logger& orig); // copy constructor
	virtual ~Logger();
	void operator= (Logger const&); // assignment
};


