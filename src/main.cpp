#include <rgl/rgl.h>
#include <iostream>
#include <lua5.3/lua.hpp>
#include <vector>

#include "chrono"
#include "unistd.h"
#include "luargl/other/includes.h"

using namespace std;
using namespace std::chrono;

// https://csl.name/post/lua-and-cpp/
// https://www.lua.org/pil/28.1.html

int64_t now() {
	int64_t ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch())
		.count();
	return ms;
}

int lua_sleep(lua_State* L) {
	float time = lua_tonumber(L, 1);
	sleep(time);
	return 0;
}

// adds a prefix to Lua print
// lbaselib.c
int lua_base_print(lua_State* L) {
	int arguments = lua_gettop(L);
	int i;

	for (i = 1; i <= arguments; i++) {  /* for each argument */
		size_t l;
		const char* s = luaL_tolstring(L, i, &l);  /* convert it to string */
		if (i > 1)  /* not the first element? */
			lua_writestring("\t", 1);  /* add a tab before it */
		lua_writestring(s, l);  /* print it */
		lua_pop(L, 1);  /* pop result */
	}
	lua_writeline();
	return 0;
}

int lua_warn(lua_State* L) {
	lua_writestring(ANSI_BOLD_YELLOW, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_print(lua_State* L) {
	lua_writestring(ANSI_BLUE, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_now(lua_State* L) {
	lua_pushnumber(L, now());
	return 1;
}

// The error message is on top of the stack.
// Fetch it, print it and then pop it off the stack.
void print_error(lua_State* L) {
	const char* message = lua_tostring(L, -1);
	cout << message;
	lua_pop(L, 1);
}

bool c_runscript(lua_State* L, const char* filename) {
	cout << "[main.cpp] compiling " << filename << "\n";

	int result = luaL_loadfile(L, filename);

	if (result == LUA_OK) {
		result = lua_pcall(L, 0, 0, 0);
		if (result != LUA_OK) {
			cout << ANSI_RED << "Error \"";
			print_error(L);
			cout << "\"\n";
		}
	}
	else {
		cout << ANSI_RED << "[main.cpp] ";
		print_error(L);
		cout << ANSI_NC << "\n";
		return false;
	}

	return true;
}

int main(int argc, char* argv[]) {
	if (!argv[1]) {
		cout << ANSI_RED << "Specify an example to run as the first argument.\nex. playground, manyfox\n" << ANSI_NC;
		return 0;
	}

	cout << "[main.cpp] running " << LUA_VERSION << "\n";

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua_register(L, "sleep", lua_sleep);
	lua_register(L, "now", lua_now);
	lua_register(L, "print", lua_print);
	lua_register(L, "warn", lua_warn);
	luaL_dostring(L, "rgl = require('luargl')");

	if (c_runscript(L, strcat(argv[1], "/init.lua"))) luaL_dostring(L, "rgl.make_window()");

	lua_close(L);

	return 0;
}
