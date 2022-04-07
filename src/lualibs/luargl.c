#include <pthread.h>
#include <rgl/rgl.h>
#include <sys/time.h>
#include <lua.h>

#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"
#include "lua5.3/lua.h"

#include "math.h"
#include "stdio.h"

lua_State* state = NULL;
int library_reference = -1;
bool started = false;

// defined across 2 files lmao kill me!!!!!
#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define NC "\033[0m"

typedef int(access_function)(lua_State* L, int idx);

int64_t now() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec;
}

// luax defs

void luax_push_error(const char* error) {
	lua_pushstring(state, error);
	lua_error(state);
}

void luax_call_global_if_exists(const char* global, int args) {
	lua_getglobal(state, global);
	if (lua_isfunction(state, -1)) {
		int run_result = lua_pcall(state, args, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%sRuntime error: \"%s\"\n%s", RED, lua_tostring(state, -1), NC);
		}
	}
}

int luax_access_field(int tbl_stack_pos, char* strindex, access_function func) {
	int stack_pos = lua_getfield(state, tbl_stack_pos, strindex);
	if (func(state, stack_pos)) {
		return stack_pos;
	}

	return -1;
}

// rgl defs

void rgl_app_init(void) {
	luax_call_global_if_exists("rgl_app_init", 0);
}

void rgl_app_update(f32 dt) {
	// if (rglIsKeyPressed(RGL_KEY_R)) {
	// _rgl_data->camera->position.x += 10 * dt;
	// };
	lua_getglobal(state, "rgl_app_update");
	if (lua_isfunction(state, -1)) {
		lua_pushnumber(state, dt);
		int run_result = lua_pcall(state, 1, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%sRuntime error: \"%s\"\n%s", RED, lua_tostring(state, -1), NC);
		}
	}
}

void rgl_app_draw(void) {
	luax_call_global_if_exists("rgl_app_draw", 0);
}

void rgl_app_quit(void) {}

// lua lib defs

int luargl_make_window(lua_State* state) {
	if (started) {
		luax_push_error("cannot call make_window when one was already created!");
		return 0;
	}

	int width = 1280;
	int height = 720;
	// todo: fix the name
	char* name = "Window";

	// read window properties or apply defaults
	// printf("%b", lua_isstring(state, library_reference));
	lua_rawgeti(state, LUA_REGISTRYINDEX, library_reference);
	lua_getfield(state, -1, "window_properties");

	if (lua_istable(state, -1)) {
		int width_pos = luax_access_field(-1, "width", lua_isinteger);
		if (width_pos != -1) {
			width = lua_tointeger(state, width_pos);
			lua_pop(state, 1);
		}

		int height_pos = luax_access_field(-1, "height", lua_isinteger);
		if (height_pos != -1) {
			height = lua_tointeger(state, height_pos);
			lua_pop(state, 1);
		}

		int name_pos = luax_access_field(-1, "title", lua_isstring);
		if (name_pos != -1) {
			name = lua_tostring(state, name_pos);
			lua_pop(state, 1);
		}
	}

	printf("new window with size %i, %i\n", width, height);

	rglAppDesc desc = (rglAppDesc){
		.title = name,
		.width = width,
		.height = height,
		.background_color = RGL_RGB(40, 40, 40),
		.init_f = rgl_app_init,
		.update_f = rgl_app_update,
		.draw_f = rgl_app_draw,
		.quit_f = rgl_app_quit,
	};

	rglStart(&desc);

	return 0;
}

int luargl_draw_circle(lua_State* state) {
	// printf("draw circle\n");

	// color: float, position: float[], size: int
	int size = lua_tointeger(state, 3);

	lua_rawgeti(state, 1, 1);
	lua_rawgeti(state, 1, 2);
	lua_rawgeti(state, 1, 3);
	float r = lua_tonumber(state, -3);
	float g = lua_tonumber(state, -2);
	float b = lua_tonumber(state, -1);

	// printf("R: %f G: %f B: %f\n", r, g, b);

	lua_rawgeti(state, 2, 1);
	lua_rawgeti(state, 2, 2);
	float x = lua_tonumber(state, -2);
	float y = lua_tonumber(state, -1);

	// printf("X: %f Y: %f\n", x, y);
	// printf("Radius: %i\n", size);

	rglV2 position = { x, y };

	rglDrawCircle(RGL_RGB(r, g, b), position, size);
	lua_settop(state, 0);
	return 0;
}

int luargl_is_key_just_pressed(lua_State* state) {
	int key = (rglKey)lua_tointeger(state, -1);
	lua_pushboolean(state, rglIsKeyJustPressed(key));

	return 1;
}

int luargl_is_key_pressed(lua_State* state) {
	int key = (rglKey)lua_tointeger(state, -1);
	lua_pushboolean(state, rglIsKeyPressed(key));

	return 1;
}

int luargl_load_image_from_file(lua_State* state) {

	char* path = lua_tostring(state, 1);
	printf("loadingi image %s", path);
	rglTexture* texture = lua_newuserdata(state, sizeof(rglTexture));
	rglTextureLoadFromFile(texture, path, RGL_TEXTURE_FILTER_NONE);

	return 1;
}

int luargl_create_sprite(lua_State* state) {

	if (!_rgl_running) {
		luax_push_error("cannot load assets when rgl hasn't started");
		return 1;
	}

	rglTexture* texture = lua_touserdata(state, 1);
	rglSprite* sprite = lua_newuserdata(state, sizeof(rglSprite));
	rglSpriteCreate(sprite, texture);

	return 1;
}

int luargl_draw_sprite(lua_State* state) {
	rglSprite* sprite = lua_touserdata(state, 1);

	rglSpriteRender(sprite);

	return 0;
}

static const luaL_Reg lib[] = {
	{"make_window", luargl_make_window},
	{"draw_circle", luargl_draw_circle},
	{"is_key_just_pressed", luargl_is_key_just_pressed},
	{"is_key_pressed", luargl_is_key_pressed},

	{"load_image_from_file", luargl_load_image_from_file},
	{"create_sprite", luargl_create_sprite},
	{"draw_sprite", luargl_draw_sprite},

	{NULL, NULL},
};

int luargl_set_camera_position(lua_State* state) {
	lua_rawgeti(state, 1, 1);
	lua_rawgeti(state, 1, 2);

	float x = lua_tonumber(state, -2);
	float y = lua_tonumber(state, -1);

	_rgl_camera->position.x = x;
	_rgl_camera->position.y = y;

	return 0;
}

int luargl_get_camera_position(lua_State* state) {
	lua_newtable(state);

	lua_pushnumber(state, _rgl_camera->position.x);
	lua_rawseti(state, 1, 1);
	lua_pushnumber(state, _rgl_camera->position.y);
	lua_rawseti(state, 1, 2);

	return 1;
}

static const luaL_Reg data_lib[] = {
	{"set_camera_position", luargl_set_camera_position},
	{"get_camera_position", luargl_get_camera_position},

	{NULL, NULL},
};

LUALIB_API int luaopen_luargl(lua_State* L) {
	state = L;

	// create a table, put it inside the registry, register library functions inside it
	// inserts the new table inside the lua_registryindex and returns the index
	lua_newtable(L);
	library_reference = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	luaL_setfuncs(L, lib, 0);

	// todo: do this better
	lua_newtable(L);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	luaL_setfuncs(L, data_lib, 0);

	// luargl.data = {}
	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	lua_pushstring(L, "data");
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	lua_settable(L, -3);

	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);

	return 1;
}
