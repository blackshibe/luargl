#include "vector2.h"

int luargl_vector2_test(lua_State* state) {
	printf("vector2 Test!\n");
	return 0;
}

int luargl_vector2_print(lua_State* state) {
	lua_rawgeti(state, -1, 2);
	lua_rawgeti(state, -2, 1);

	char str[110] = "";
	strcat(str, luaL_tolstring(state, -1, NULL));
	strcat(str, ", ");
	strcat(str, luaL_tolstring(state, -3, NULL));

	// strcat(str, );
	// strcat(str, ", y:");
	// strcat(str, lua_tolstring(state, -1, &l));

	lua_pushstring(state, str);

	return 1;
}

void luargl_create_vector2(lua_State* state, f32 x, f32 y) {
	// printf("%i\n", lua_gettop(state));
	int reltop = lua_gettop(state);
	lua_newtable(state);
	luaL_getmetatable(state, "Vector2");
	lua_setmetatable(state, reltop + 1);

	lua_pushnumber(state, x);
	lua_seti(state, reltop + 1, 1);

	lua_pushnumber(state, y);
	lua_seti(state, reltop + 1, 2);

	// printf("end: %i\n", lua_gettop(state));
}

const luaL_Reg luargl_vector2_methods[] = {
	{ "test", luargl_vector2_test },

	{ NULL, NULL }
};