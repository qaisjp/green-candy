// Master include of the MTA:Lua module.
// Can be turned into a precompiled header for performance optimizations.

#define LUA_CORE
#include "lua.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif