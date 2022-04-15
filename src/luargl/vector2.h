#include <rgl/rgl.h>
#include <lua.h>
#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"
#include "lua5.3/lua.h"

#include "math.h"
#include "../colors.h"

int luargl_vector2_test(lua_State* state);
void luargl_create_vector2(lua_State* state, f32 x, f32 y);
int luargl_vector2_print(lua_State* state);

extern const luaL_Reg luargl_vector2_methods[];