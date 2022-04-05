#include <rgl/rgl.h>

#include <iostream>
#include <lua5.3/lua.hpp>
#include <vector>

#include "chrono"
#include "unistd.h"

using namespace std;
using namespace std::chrono;

#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define NC "\033[0m"

// https://csl.name/post/lua-and-cpp/

int64_t now() {
	int64_t ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch())
		.count();

	return ms;
}

int lua_sleep(lua_State* state) {
	float time = lua_tonumber(state, 1);

	printf("sleep for %f\n", time);
	sleep(time);

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
		cout << BLUE;

		result = lua_pcall(state, 0, 0, 0);

		if (result != LUA_OK) {
			cout << RED << "Error \"";
			print_error(state);
			cout << "\"\n";
		}
		cout << NC;
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

	cout << "Running with " << LUA_VERSION << "\n";

	// c_runscript(state, "scripts/first.lua");
	// c_runscript(state, "scripts/external.lua");
	// c_runscript(state, "scripts/vector.lua");
	// c_runscript(state, "scripts/sleep.lua");
	c_runscript(state, "scripts/rgl.lua");

	lua_close(state);

	return 0;
}
