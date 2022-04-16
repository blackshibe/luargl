#include "sprite.h"

int luargl_sprite_index(lua_State* L) {
	lua_getfield(L, 1, "__sprite");
	rglSprite* sprite = (rglSprite*)lua_touserdata(L, -1);

	const char* index = lua_tostring(L, 2);
	if (strcmp(index, "position") == 0) {
		luargl_create_vector2(L, sprite->position.x, sprite->position.y);
	}
	else if (strcmp(index, "size") == 0) {
		luargl_create_vector2(L, sprite->size.x, sprite->size.y);
	}
	else if (strcmp(index, "size") == 0) {
		lua_pushnumber(L, sprite->rotation);
	}

	return 1;
}

// args: {__sprite = userdata}, index, value
int luargl_sprite_newindex(lua_State* L) {
	lua_getfield(L, 1, "__sprite");
	rglSprite* sprite = (rglSprite*)lua_touserdata(L, -1);

	const char* index = lua_tostring(L, 2);
	if (strcmp(index, "position") == 0) {
		lua_rawgeti(L, 3, 1);
		sprite->position.x = lua_tonumber(L, -1);

		lua_rawgeti(L, 3, 2);
		sprite->position.y = lua_tonumber(L, -1);
	}
	else if (strcmp(index, "size") == 0) {
		lua_rawgeti(L, 3, 1);
		sprite->size.x = lua_tonumber(L, -1);

		lua_rawgeti(L, 3, 2);
		sprite->size.y = lua_tonumber(L, -1);
	}
	else if (strcmp(index, "rotation") == 0) {
		sprite->rotation = lua_tonumber(L, 3);
	}

	return 0;
}

const luaL_Reg luargl_sprite_methods[] = {
	{ "__index", luargl_sprite_index},
	{ "__newindex", luargl_sprite_newindex},

	{ NULL, NULL }
};