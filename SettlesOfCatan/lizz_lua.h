#pragma once

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class lizz_lua{
public:
	// variables	
	bool active;
	//methods
	static lizz_lua& get();
	bool run_file(const char* file);
private:
	// variables	
	lua_State* lua_state;
	// methods
	void load();
	// constructor and destructors
	lizz_lua();
	lizz_lua(const lizz_lua& orig){};
	virtual ~lizz_lua();
	void operator= (const lizz_lua&);
};

