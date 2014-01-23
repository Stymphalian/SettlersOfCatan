#include <cstdio>
extern "C"{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "sqlite3.h"
#include "Logger.h"
#include "lizz_lua.h"


int db_func(){
	sqlite3* dbh;
	sqlite3_open("db.db", &dbh);
	sqlite3_close(dbh);
	return 0;
}

int check_num_of_args(lua_State* state, int args){
	int n = lua_gettop(state);
	if(n < args){ return -1; }
	else if(n > args){ return 1; }
	return 0;
}

int first_function(lua_State* state){
	if(check_num_of_args(state,2) != 0){
		Logger::getLog("jordan.log").log(Logger::ERROR, "first_function error not enough arguments");
		return 0;
	}	
	printf("Hello Good there bob boy = %d for db!\n", db_func());
	return 1;
}


lizz_lua::lizz_lua(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "lizz_lua::lizz_lua() Constructor");
	this->lua_state = luaL_newstate();
	if(this->lua_state == nullptr){
		this->active = false;
		Logger::getLog("jordan.log").log(Logger::ERROR,"lizz_lua::lizz_lua() Unable to obtain lua_state" );
		return;
	}
	luaL_openlibs(this->lua_state);

	// load all the appropriate bindings
	this->load();
}
lizz_lua::~lizz_lua(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "lizz_lua::~lizz_lua Destructor");
	lua_close(this->lua_state);
	this->lua_state = nullptr;
}
lizz_lua& lizz_lua::get(){
	static lizz_lua instance;
	return instance;
}

bool lizz_lua::run_file(const char* file){
	if(file == nullptr){ return false; }
	if(luaL_dofile(this->lua_state, file)){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "lizz_lua::run_file(%s) Failed to run lua script",file);
		return false;
	} 
	return true;
}

void lizz_lua::load(){	
	Logger::getLog("jordan.log").log(Logger::DEBUG, "lizz_lua::load() Registering lua functions");

	// do nothing so far.
	lua_register(lua_state, "lizz", first_function);
}




