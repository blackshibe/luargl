#include <rgl/rgl.h>
#include <iostream>
#include <lua5.3/lua.hpp>
#include <vector>

#include "chrono"
#include "unistd.h"
#include "colors.h"

using namespace std;
using namespace std::chrono;

// https://csl.name/post/lua-and-cpp/

int64_t now() {
	int64_t ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch())
		.count();

	return ms;
}

int lua_sleep(lua_State* state) {
	float time = lua_tonumber(state, 1);

	sleep(time);

	return 0;
}

// adds a prefix to Lua print
// lbaselib.c
int lua_base_print (lua_State *L) {
	int arguments = lua_gettop(L);
	int i;

	for (i = 1; i <= arguments; i++) {  /* for each argument */
		size_t l;
		const char *s = luaL_tolstring(L, i, &l);  /* convert it to string */
		if (i > 1)  /* not the first element? */
		lua_writestring("\t", 1);  /* add a tab before it */
		lua_writestring(s, l);  /* print it */
		lua_pop(L, 1);  /* pop result */
	}
	lua_writeline();
	return 0;
}

int lua_warn (lua_State *L) {
	lua_writestring(ANSI_BOLD_YELLOW, 7);  
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);  

	return 0;
}

int lua_print (lua_State *L) {
	lua_writestring(ANSI_BLUE, 7);  
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);  

	return 0;
}

int lua_now(lua_State* state) {
	lua_pushnumber(state, now());
	return 1;
}

// The error message is on top of the stack.
// Fetch it, print it and then pop it off the stack.
void print_error(lua_State* state) {
	const char* message = lua_tostring(state, -1);
	cout << message;
	lua_pop(state, 1);
}

void c_runscript(lua_State* state, const char* filename) {
	cout << " ... Running " << filename << "\n";

	int result = luaL_loadfile(state, filename);

	// you have to initialize the libraries in Lua.
	// after you call lua_open, call
	if (result == LUA_OK) {
		result = lua_pcall(state, 0, 0, 0);
		if (result != LUA_OK) {
			cout << ANSI_RED << "Error \"";
			print_error(state);
			cout << "\"\n";
		}
	}
	else {
		cout << result << "\n";
		print_error(state);
	}
}

int main() {
	lua_State* state = luaL_newstate();
	luaL_openlibs(state);
	lua_register(state, "sleep", lua_sleep);
	lua_register(state, "now", lua_now);
	lua_register(state, "print", lua_print);
	lua_register(state, "warn", lua_warn);

	cout << "Running with " << LUA_VERSION << "\n";

	// c_runscript(state, "scripts/first.lua");
	// c_runscript(state, "scripts/external.lua");
	// c_runscript(state, "scripts/vector.lua");
	// c_runscript(state, "scripts/sleep.lua");
	// c_runscript(state, "scripts/output.lua");
	c_runscript(state, "scripts/rgl.lua");


	lua_close(state);

	return 0;
}
