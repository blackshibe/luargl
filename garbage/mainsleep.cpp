// keeps dying from segfaults or some shit idk running 2 threads in one lua state is a bad idea
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

struct timeoutInfo {
    int func;
    long time;
    bool disposable;
    bool active;
};

vector<timeoutInfo> shit;

int64_t now() {
    int64_t ms = duration_cast<milliseconds>(
                     system_clock::now().time_since_epoch())
                     .count();

    return ms;
}

int luanow(lua_State *state) {
    lua_pushnumber(state, now());

    return 1;
}

// todo: verify this isnt a completely shit implementation that breaks every 5 seconds
// https://stackoverflow.com/questions/12823934/storing-reference-to-lua-function-in-c
int tab_idx;
int set_timeout(lua_State *state) {
    lua_rawgeti(state, LUA_REGISTRYINDEX, tab_idx);  // retrieve table for functions
    lua_pushvalue(state, 1);
    int t = luaL_ref(state, 3);  // store a function in the function table

    lua_pop(state, 1);

    int time = lua_tointeger(state, 2);
    shit.push_back({t, now() + time, false, true});

    return 0;
}

// The error message is on top of the stack.
// Fetch it, print it and then pop it off the stack.
void print_error(lua_State *state) {
    const char *message = lua_tostring(state, -1);
    cout << message;
    lua_pop(state, 1);
}

void c_runscript(lua_State *state, const char *filename) {
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
    } else {
        cout << result << "\n";
        print_error(state);
    }
}

int main() {
    lua_State *state = luaL_newstate();
    luaL_openlibs(state);
    lua_register(state, "set_timeout", set_timeout);
    lua_register(state, "now", luanow);

    lua_newtable(state);                           // create table for functions
    tab_idx = luaL_ref(state, LUA_REGISTRYINDEX);  // store said table in pseudo-registry

    cout << "Running with " << LUA_VERSION << "\n";

    // c_runscript(state, "scripts/first.lua");
    // c_runscript(state, "scripts/external.lua");
    // c_runscript(state, "scripts/vector.lua");
    // c_runscript(state, "scripts/sleep.lua");
    c_runscript(state, "scripts/rgl.lua");

    cout << "\n";

    while (true)

    {
        // int64_t ms = now();
        // for (int i = 0; i < shit.size(); i++) {
        //     timeoutInfo element = shit[i];
        //     if (element.active) {
        //         if (element.time < ms && !element.disposable) {
        //             lua_rawgeti(state, LUA_REGISTRYINDEX, tab_idx);  // retrieve function table
        //             lua_rawgeti(state, -1, element.func);            // retreive function
        //             // Lua calls the function at the top of the stack
        //             int result = lua_pcall(state, 0, 0, 0);
        //             if (result != LUA_OK) {
        //                 printf("%sRuntime error: \"%s\"\n%s", RED, lua_tostring(state, -1), NC);
        //             }

        //             // lua_pop(state, 1);

        //             element.disposable = true;
        //             element.active = false;

        //             shit.erase(shit.begin() + i);
        //         }
        //     }
        // }

        sleep(1 / 16);
    }

    lua_close(state);

    return 0;
}
