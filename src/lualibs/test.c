#include "lua5.3/lauxlib.h"
#include "lua5.3/lua.h"
#include "math.h"
#include "stdio.h"

int ext(lua_State *state) {
    int args = lua_gettop(state);
    int sum = 0;

    for (int i = 1; i <= args; i++) {
        sum += lua_tointeger(state, i);
        printf("%lld, ", lua_tointeger(state, i));
    }

    printf("args: %d\n", args);
    lua_pushinteger(state, sum);

    return 1;
}

int test_vector3(lua_State *state) {
    printf("vector3\n");
    int args = lua_gettop(state);

    printf("args: %d\n", args);

    lua_Number x, y, z;

    lua_pushstring(state, "x");
    lua_gettable(state, args);
    x = lua_tonumber(state, 2);

    lua_pushstring(state, "y");
    lua_gettable(state, args);
    y = lua_tonumber(state, 3);

    lua_pushstring(state, "z");
    lua_gettable(state, args);
    z = lua_tonumber(state, 4);

    printf("x = %f, y = %f, z = %f\n", x, y, z);
    printf("x = %d, y = %d, z = %d\n", (int)(x), (int)(y), (int)(z));

    return 1;
}

static const luaL_Reg lib[] = {
    {"test", ext},
    {"test_vector3", test_vector3},
    {NULL, NULL}};

LUALIB_API int luaopen_test(lua_State *state) {
    lua_newtable(state);
    luaL_setfuncs(state, lib, 0);
    return 1;
}
