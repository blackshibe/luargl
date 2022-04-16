#include <pthread.h>
#include <rgl/rgl.h>
#include <rgl/rgl_shader.h>
#include <sys/time.h>
#include <math.h>

#include <lua.h>
#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"

#include "other/includes.h"
#include "sprite.h"
#include "vector2.h"
#include "camera.h"
#include "freetype.h"

lua_State* L = NULL;
int library_reference = -1;
typedef int(access_function)(lua_State* L, int idx);

int64_t now() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec;
}

void luax_push_error(const char* error) {
	lua_pushstring(L, error);
	lua_error(L);
}

void luax_call_global_if_exists(const char* global, int args) {
	lua_getglobal(L, global);
	if (lua_isfunction(L, -1)) {
		int run_result = lua_pcall(L, args, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%s[Lua] Runtime error: \"%s\"\n%s", ANSI_BOLD_RED, lua_tostring(L, -1), ANSI_NC);
		}
	}
}

int luax_access_field(int tbl_stack_pos, char* strindex, access_function func) {
	int stack_pos = lua_getfield(L, tbl_stack_pos, strindex);
	if (func(L, stack_pos)) {
		return stack_pos;
	}
	return -1;
}

// https://learnopengl.com/In-Practice/2D-Game/Postprocessing
// GLuint buffer;

// rgl defs
void rgl_app_init(void) {
	luax_call_global_if_exists("rgl_app_init", 0);
	freetype_init(L);

	// glGenFramebuffers(1, &buffer);
}

void rgl_app_update(f32 dt) {
	lua_getglobal(L, "rgl_app_update");
	if (lua_isfunction(L, -1)) {
		lua_pushnumber(L, dt);
		int run_result = lua_pcall(L, 1, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%s[Lua] Runtime error: \"%s\"\n%s", ANSI_BOLD_RED, lua_tostring(L, -1), ANSI_NC);
		}
	}

	// doesn't work inside init
	freetype_update_init();
}
void rgl_app_draw(void) {
	// 	glBindFramebuffer(GL_FRAMEBUFFER, &buffer);
	// 	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// 	glClear(GL_COLOR_BUFFER_BIT);

	luax_call_global_if_exists("rgl_app_draw", 0);
}

void rgl_app_quit(void) {
	luax_call_global_if_exists("rgl_app_quit", 0);
	freetype_quit();
}

int luargl_make_window(lua_State* L) {
	if (_rgl_running) {
		luax_push_error("cannot call make_window when one was already created!");
		return 0;
	}

	int width = 1280;
	int height = 720;
	const char* name = "Window";

	// read window properties or apply defaults
	// printf("%b", lua_isstring(L, library_reference));
	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	lua_getfield(L, -1, "window_properties");

	if (lua_istable(L, -1)) {
		int width_pos = luax_access_field(-1, "width", lua_isinteger);
		if (width_pos != -1) {
			width = lua_tointeger(L, width_pos);
			lua_pop(L, 1);
		}

		int height_pos = luax_access_field(-1, "height", lua_isinteger);
		if (height_pos != -1) {
			height = lua_tointeger(L, height_pos);
			lua_pop(L, 1);
		}

		// luax access field did not work lol
		lua_getfield(L, -1, "title");
		if (lua_isstring(L, -1)) {
			name = luaL_tolstring(L, -1, NULL);
		}
	}

	printf("[luargl.c] window created, size %i, %i\n", width, height);
	printf("[luargl.c] %s\n", name);

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

int luargl_draw_circle(lua_State* L) {
	// printf("draw circle\n");

	// color: float, position: float[], size: int
	int size = lua_tointeger(L, 3);

	lua_rawgeti(L, 1, 1);
	lua_rawgeti(L, 1, 2);
	lua_rawgeti(L, 1, 3);
	float r = lua_tonumber(L, -3);
	float g = lua_tonumber(L, -2);
	float b = lua_tonumber(L, -1);

	// printf("R: %f G: %f B: %f\n", r, g, b);

	lua_rawgeti(L, 2, 1);
	lua_rawgeti(L, 2, 2);
	float x = lua_tonumber(L, -2);
	float y = lua_tonumber(L, -1);

	// printf("X: %f Y: %f\n", x, y);
	// printf("Radius: %i\n", size);

	rglV2 position = { x, y };

	rglDrawCircle(RGL_RGB(r, g, b), position, size);
	lua_settop(L, 0);
	return 0;
}

int luargl_is_key_just_pressed(lua_State* L) {
	int key = (rglKey)lua_tointeger(L, -1);
	lua_pushboolean(L, rglIsKeyJustPressed(key));

	return 1;
}

int luargl_is_key_pressed(lua_State* L) {
	int key = (rglKey)lua_tointeger(L, -1);
	lua_pushboolean(L, rglIsKeyPressed(key));

	return 1;
}


int luargl_load_image_from_file(lua_State* L) {

	const char* path = lua_tostring(L, 1);
	rglTexture* texture = lua_newuserdata(L, sizeof(rglTexture));
	rglTextureLoadFromFile(texture, path, RGL_TEXTURE_FILTER_NONE);

	return 1;
}

int luargl_create_sprite(lua_State* L) {

	if (!_rgl_running) {
		luax_push_error("cannot load assets when rgl hasn't started");
		return 1;
	}

	rglTexture* texture = lua_touserdata(L, 1);

	// creates a table: { __sprite = userdata } with a Sprite metatable
	lua_newtable(L);
	rglSprite* sprite = lua_newuserdata(L, sizeof(rglSprite));
	lua_setfield(L, -2, "__sprite");
	luaL_getmetatable(L, "Sprite");
	lua_setmetatable(L, -2);

	rglSpriteCreate(sprite, texture);

	return 1;
}

int luargl_draw_sprite(lua_State* L) {
	lua_getfield(L, -1, "__sprite");
	rglSpriteRender(lua_touserdata(L, -1));

	return 0;
}

int luargl_destroy_sprite(lua_State* L) {
	lua_getfield(L, -1, "__sprite");
	rglSpriteDestroy(lua_touserdata(L, -1));

	return 0;
}

int luargl_destroy_image(lua_State* L) {
	rglTextureDestroy(lua_touserdata(L, 1));

	return 0;
}

int luargl_vector2(lua_State* L) {

	lua_Number x = lua_tonumber(L, 1);
	lua_Number y = lua_tonumber(L, 2);
	luargl_create_vector2(L, x, y);

	return 1;
}

int luargl_get_mouse_position(lua_State* L) {
	rglV2 pos;
	rglGetCursorPos(&pos);
	luargl_create_vector2(L, pos.x, pos.y);
	return 1;
}

int luargl_get_mouse_position_in_world_space(lua_State* L) {
	rglV2 pos;
	rglGetCursorPosInWorld(&pos);
	luargl_create_vector2(L, pos.x, pos.y);
	return 1;
}

int luargl_meta_index(lua_State* L) {
	return 0;
}

int luargl_meta_newindex(lua_State* L) {
	return 0;
}

void luargl_get_flag(char* str) {
	lua_getglobal(L, "rgl");

	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	lua_getfield(L, -1, "debug");
	lua_getfield(L, -1, str);
}

static const luaL_Reg lib[] = {
	{"make_window", luargl_make_window},
	{"draw_circle", luargl_draw_circle},
	// {"draw_rect", luargl_draw_circle},

	{"is_key_just_pressed", luargl_is_key_just_pressed},
	{"is_key_pressed", luargl_is_key_pressed},

	{"load_image_from_file", luargl_load_image_from_file},
	{"load_font_from_file", luargl_load_font_from_file},

	{"destroy_image", luargl_destroy_image},

	{"create_sprite", luargl_create_sprite},
	{"draw_sprite", luargl_draw_sprite},
	{"destroy_sprite", luargl_destroy_sprite},

	{"get_mouse_position", luargl_get_mouse_position },
	{"get_mouse_position_in_world_space", luargl_get_mouse_position_in_world_space },

	{"render_text", luargl_render_text },

	{"vector2", luargl_vector2 },

	{NULL, NULL},
};

static const luaL_Reg libmeta[] = {
	{"__index", luargl_meta_index},
	// {"__newindex", luargl_meta_newindex},
	{NULL, NULL},
};

LUALIB_API int luaopen_luargl(lua_State* _L) {
	L = _L;

	luaL_newmetatable(L, "Camera");
	luaL_setfuncs(L, luargl_camera_methods, 0);

	luaL_newmetatable(L, "Sprite");
	luaL_setfuncs(L, luargl_sprite_methods, 0);

	luaL_newmetatable(L, "Vector2");
	lua_pushcclosure(L, luargl_vector2_print, 0);
	lua_setfield(L, -2, "__tostring");

	lua_newtable(L);
	luaL_setfuncs(L, luargl_vector2_methods, 0);
	lua_setfield(L, -2, "__index");

	// create a table, put it inside the registry, register library functions inside it
	lua_newtable(L);
	library_reference = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	luaL_setfuncs(L, lib, 0);
	init_camera(L);

	// rgl.debug
	lua_newtable(L);
	lua_setfield(L, -2, "debug");

	// assigns a metatable to the luargl variable
	lua_newtable(L);
	luaL_setfuncs(L, libmeta, 0);
	lua_setmetatable(L, -2);

	return 1;
}
