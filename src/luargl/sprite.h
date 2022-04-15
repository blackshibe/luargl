#include <rgl/rgl.h>
#include <lua.h>
#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"
#include "lua5.3/lua.h"

#include "math.h"
#include "../colors.h"

int luargl_sprite_index(lua_State* state);
int luargl_sprite_newindex(lua_State* state);

extern const luaL_Reg luargl_sprite_methods[];