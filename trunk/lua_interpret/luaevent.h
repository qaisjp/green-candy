/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luaevent.h
*  PURPOSE:     Lua event handling classes
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_EVENT_SYSTEM_
#define _LUA_EVENT_SYSTEM_

#define LUACLASS_EVENTSYS   43

struct LuaEventHandle
{
    RwListEntry <LuaEventHandle> node;
    LuaFunctionRef ref;
};

struct LuaEvent
{
    RwListEntry <LuaEvent> node;
    RwList <LuaEventHandle> handlers;
    std::string name;
};

struct LuaEventSys : public LuaClass
{
    LuaEventSys( lua_State *L );
    LuaEventSys( lua_State *L, ILuaClass *j );
    ~LuaEventSys( void );

    LuaEvent*   Get( const char *name );

    RwList <LuaEvent> events;

    LuaEventSys *parent;
    RwListEntry <LuaEventSys> childNode;
    RwList <LuaEventSys> children;
};

void luaevent_extend( lua_State *L );

#endif //_LUA_EVENT_SYSTEM_