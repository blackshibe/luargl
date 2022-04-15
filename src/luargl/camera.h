#include "other/includes.h"

int init_camera(lua_State* state);
int luargl_camera_index(lua_State* state);
int luargl_camera_newindex(lua_State* state);

extern const luaL_Reg luargl_camera_methods[];