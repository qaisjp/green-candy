/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/logic/lua/CLuaTimerManager.h
*  PURPOSE:     Lua timer manager class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_TIMER_MANAGER_
#define _BASE_LUA_TIMER_MANAGER_

class LuaTimerManager
{
    friend class LuaTimer;
public:
    inline                      LuaTimerManager()                       {}
    inline                      ~LuaTimerManager()                      { RemoveAllTimers(); };

    void                        DoPulse( LuaMain& main );

    bool                        Exists( LuaTimer *timer );
    LuaTimer*                   GetTimer( unsigned int id );

    LuaTimer*                   AddTimer( lua_State *L, const LuaFunctionRef& ref, CTickCount delay, unsigned int repCount );
    void                        RemoveTimer( LuaTimer *timer );
    void                        RemoveAllTimers();
    unsigned long               GetTimerCount() const                   { return m_list.size(); }

    void                        ResetTimer( LuaTimer *timer );

    void                        GetTimers( CTickCount time, LuaMain *main );

private:
    CMappedList <LuaTimer*>     m_list;
};

#endif //_BASE_LUA_TIMER_MANAGER