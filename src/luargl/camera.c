#include "camera.h"
#include "vector2.h"

// takes the current camera and assigns the camera variable to the table on the stack
int init_camera(lua_State* state) {
	lua_newtable(state);

	rglCamera* camera = lua_newuserdata(state, sizeof(rglCamera));
	lua_setfield(state, -2, "__camera");
	luaL_getmetatable(state, "Camera");
	lua_setmetatable(state, -2);
	lua_setfield(state, -2, "camera");

	// broken
	// rglV2 pos = { 0, 0 };
	// rglCameraCreate(camera, pos, 1);
	// _rgl_camera = camera;

	return 0;
}

int luargl_camera_index(lua_State* state) {
	lua_getfield(state, 1, "__camera");
	rglCamera* camera = (rglCamera*)lua_touserdata(state, -1);

	const char* index = lua_tostring(state, 2);
	if (strcmp(index, "position") == 0)
		luargl_create_vector2(state, _rgl_camera->position.x, _rgl_camera->position.y);
	else if (strcmp(index, "zoom") == 0) {
		lua_pushnumber(state, _rgl_camera->zoom);
	}

	return 1;
};

// setting _rgl_camera to camera above and editing that instead is broken
int luargl_camera_newindex(lua_State* state) {
	lua_getfield(state, 1, "__camera");
	rglCamera* camera = (rglCamera*)lua_touserdata(state, -1);

	const char* index = lua_tostring(state, 2);
	if (strcmp(index, "position") == 0) {

		lua_rawgeti(state, 3, 1);
		f32 x = (f32)lua_tonumber(state, -1);

		lua_rawgeti(state, 3, 2);
		f32 y = (f32)lua_tonumber(state, -1);

		_rgl_camera->position.x = x;
		_rgl_camera->position.y = y;
	}
	else if (strcmp(index, "zoom") == 0) {
		f32 zoom = (f32)lua_tonumber(state, 3);
		_rgl_camera->zoom = zoom;
	}

	return 0;
};

const luaL_Reg luargl_camera_methods[] = {
	{ "__index", luargl_camera_index},
	{ "__newindex", luargl_camera_newindex},

	{ NULL, NULL }
};