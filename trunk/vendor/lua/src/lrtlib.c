#include "luacore.h"

#include "lauxlib.h"
#include "lualib.h"
#include "lrtlib.h"

static int luaRT_create( lua_State *L )
{
    return 0;
}

static const luaL_Reg runtime_library[] =
{
    { "create", luaRT_create },
    { NULL, NULL }
};

LUAI_FUNC int luaopen_runtime( lua_State *L )
{
    luaL_register( L, LUA_RUNTIMELIBNAME, runtime_library );
    return 1;
}