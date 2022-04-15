#include "sprite.h"
#include "vector2.h"

int luargl_sprite_index(lua_State* state) {
	lua_getfield(state, 1, "__sprite");
	rglSprite* sprite = (rglSprite*)lua_touserdata(state, -1);

	const char* index = lua_tostring(state, 2);
	if (strcmp(index, "position") == 0) {
		luargl_create_vector2(state, sprite->position.x, sprite->position.y);
	}
	else if (strcmp(index, "size") == 0) {
		luargl_create_vector2(state, sprite->size.x, sprite->size.y);
	}

	return 1;
}

// args: {__sprite = userdata}, index, value
int luargl_sprite_newindex(lua_State* state) {
	lua_getfield(state, 1, "__sprite");
	rglSprite* sprite = (rglSprite*)lua_touserdata(state, -1);

	const char* index = lua_tostring(state, 2);
	if (strcmp(index, "position") == 0) {

		lua_rawgeti(state, 3, 1);
		sprite->position.x = lua_tonumber(state, -1);

		lua_rawgeti(state, 3, 2);
		sprite->position.y = lua_tonumber(state, -1);
	}
	else if (strcmp(index, "size") == 0) {

		lua_rawgeti(state, 3, 1);
		sprite->size.x = lua_tonumber(state, -1);

		lua_rawgeti(state, 3, 2);
		sprite->size.y = lua_tonumber(state, -1);
	}

	return 0;
}

const luaL_Reg luargl_sprite_methods[] = {
	{ "__index", luargl_sprite_index},
	{ "__newindex", luargl_sprite_newindex},

	{ NULL, NULL }
};