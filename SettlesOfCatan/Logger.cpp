#include "Logger.h"
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

// Constructor
	Logger::Logger(const char* file, bool output_to_stderr_aswell = true){
		// set some of the flags
		good_flag = true;
		output_to_stderr_flag = output_to_stderr_aswell;
		level = Logger::levels::NONE;
		depth_level = Logger::levels::DEBUG;

		// Open up the file stream
		file_stream = 0;
		fopen_s(&file_stream, file, "w");
		if(file_stream == nullptr){
			good_flag = false;
			fprintf(stderr, "Logger failed to open log file '%s'\n", file);
			return;
		}
		filename = file;
	}
	// Destructor
	Logger::~Logger() {
		if(good_flag == true){
			Logger::getLog("jordan.log").log(Logger::DEBUG, "Logger destructor");
			fclose(file_stream);
		}
	}

	// Helper methods
	std::string Logger::get_current_time(){
		time_t now;
		tm t;
		char buffer[128];
		memset(buffer, 0, sizeof(buffer));
		time(&now);
		localtime_s(&t, &now);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d,%H:%M:%S", &t);
		return std::string(buffer);
	}

	std::string Logger::get_level_from_enum(Logger::levels level){
		switch(level){
		case(Logger::levels::NONE) : {return ""; }
		case(Logger::levels::DEBUG) : {return"DEBUG"; }
		case(Logger::levels::ERROR) : {return "ERROR"; }
		case(Logger::levels::WARN) : {return "WARN"; }
		case(Logger::levels::INFO) : {return "INFO"; }
		}
		return "";
	}

	// public methods
	// Get the log. Return an function-static instantion of the logger
	Logger& Logger::getLog(const char* file){
		static Logger instance(file);
		return instance;
	}
	void Logger::_log(Logger::levels level, const char* format, va_list args){
		// Output to the log
		fprintf(file_stream, "%s | %-10s |\t",
			get_current_time().c_str(), get_level_from_enum(level).c_str()
			);
		vfprintf(file_stream, format, args);
		fprintf(file_stream, "\n");
		// Flush out the data from the file stream
		fflush(file_stream);

		// Ouput to stderr as well
		if(output_to_stderr_flag){
			fprintf(stderr, "%s | %-10s |\t",
				get_current_time().c_str(), get_level_from_enum(level).c_str()
				);
			vfprintf(stderr, format, args);
			fprintf(stderr, "\n");
			// Flush out the data from the file stream
			fflush(stderr);
		}
	}
	void Logger::_log_nnl(Logger::levels level, const char* format, va_list args){
		// Output to the log
		fprintf(file_stream, "%s | %-10s |\t",
			get_current_time().c_str(), get_level_from_enum(level).c_str()
			);
		vfprintf(file_stream, format, args);
		//fprintf(file_stream, "\n");
		// Flush out the data from the file stream
		fflush(file_stream);

		// Ouput to stderr as well
		if(output_to_stderr_flag){
			fprintf(stderr, "%s | %-10s |\t",
				get_current_time().c_str(), get_level_from_enum(level).c_str()
				);
			vfprintf(stderr, format, args);
			//fprintf(stderr, "\n");
			// Flush out the data from the file stream
			fflush(stderr);
		}
	}
	void Logger::_log_append(Logger::levels level, const char* format, va_list args){
		// Output to the log
		/*fprintf(file_stream, "%s | %-10s |\t",
			get_current_time().c_str(), get_level_from_enum(level).c_str()
			);*/
		vfprintf(file_stream, format, args);
		//fprintf(file_stream, "\n");
		// Flush out the data from the file stream
		fflush(file_stream);

		// Ouput to stderr as well
		if(output_to_stderr_flag){
			/*fprintf(stderr, "%s | %-10s |\t",
				get_current_time().c_str(), get_level_from_enum(level).c_str()
				);*/
			vfprintf(stderr, format, args);
			//fprintf(stderr, "\n");
			// Flush out the data from the file stream
			fflush(stderr);
		}
	}


	void Logger::set_level(Logger::levels level){
		this->level = level;
	}
	void Logger::set_deepest_level_allowed(Logger::levels level){
		if(level == Logger::levels::NONE){
			this->depth_level = Logger::levels::DEBUG;
			return;
		}
		this->depth_level = level;
	}

	void Logger::log(const char* format, ...){
		if(!format){ return; }
		if(good_flag == false){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);
		// Do the logging
		_log(level, format, args);
		va_end(args);
	}

	void Logger::log(Logger::levels level, const char* format, ...){
		if(!format){ return; }
		if(good_flag == false){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Do the logging
		_log(level, format, args);
		va_end(args);
	}
	void Logger::log_nnl(Logger::levels level, const char* format, ...){
		if(!format){ return; }
		if(good_flag == false){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Do the logging
		_log_nnl(level, format, args);
		va_end(args);
	}
	void Logger::log_append(Logger::levels level, const char* format, ...){
		if(!format){ return; }
		if(good_flag == false){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Do the logging
		_log_append(level, format, args);
		va_end(args);
	}

	// The only difference between a normal Logger::log() and SDL_log()
	// is the extra call to SDL_GetError()
	void Logger::SDL_log(Logger::levels level, const char* format, ...)
	{
		if(!format){ return; }
		if(good_flag == false){ return; }
		if(level < depth_level){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Log the information
		_log(level, format, args);
		fprintf(file_stream, "SDL_GetError() : %s\n", SDL_GetError());
		fflush(file_stream);
		if(output_to_stderr_flag){
			fprintf(stderr, "SDL_GetError() : %s\n", SDL_GetError());
			fflush(stderr);
		}
		va_end(args);
	}
	void Logger::Mix_log(Logger::levels level, const char* format, ...)
	{
		if(!format){ return; }
		if(good_flag == false){ return; }
		if(level < depth_level){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Log the information
		_log(level, format, args);
		fprintf(file_stream, "Mix_GetError() : %s\n", Mix_GetError());
		fflush(file_stream);
		if(output_to_stderr_flag){
			fprintf(stderr, "Mix_GetError() : %s\n", Mix_GetError());
			fflush(stderr);
		}
		va_end(args);
	}
	void Logger::TTF_log(Logger::levels level, const char* format, ...)
	{
		if(!format){ return; }
		if(good_flag == false){ return; }
		if(level < depth_level){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Log the information
		_log(level, format, args);
		fprintf(file_stream, "TTF_GetError() : %s\n", TTF_GetError());
		fflush(file_stream);
		if(output_to_stderr_flag){
			fprintf(stderr, "TTF_GetError() : %s\n", TTF_GetError());
			fflush(stderr);
		}
		va_end(args);
	}
	void Logger::IMG_log(Logger::levels level, const char* format, ...)
	{
		if(!format){ return; }
		if(good_flag == false){ return; }
		if(level < depth_level){ return; }

		// Open up all the variable arguments
		va_list args;
		va_start(args, format);

		// Log the information
		_log(level, format, args);
		fprintf(file_stream, "IMG_GetError() : %s\n", IMG_GetError());
		fflush(file_stream);
		if(output_to_stderr_flag){
			fprintf(stderr, "IMG_GetError() : %s\n", IMG_GetError());
			fflush(stderr);
		}
		va_end(args);
	}