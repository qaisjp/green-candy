/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaTimer.h
*  PURPOSE:     Lua timed execution
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Florian Busse <flobu@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_TIMER_
#define _BASE_LUA_TIMER_

#define LUA_TIMER_MIN_INTERVAL      50

class LuaTimer : public LuaClass
{
public:
                            LuaTimer( lua_State *L, class LuaTimerManager *manager, const LuaFunctionRef& ref );
    virtual                 ~LuaTimer();

    CTickCount              GetStartTime() const                        { return m_startTime; };
    inline void             SetStartTime( CTickCount startTime )        { m_startTime = startTime; };

    CTickCount              GetDelay() const                            { return m_delay; };
    inline void             SetDelay( CTickCount delay )                { m_delay = delay; };

    inline unsigned int     GetRepeats() const                          { return m_repCount; };
    inline void             SetRepeats( unsigned int repCount )         { m_repCount = repCount; }

    void                    Execute( class LuaMain *main );

    CTickCount              GetTimeLeft();

private:
    LuaTimerManager*        m_manager;
    LuaFunctionRef          m_ref;
    LuaArguments*           m_args;
    CTickCount              m_startTime;
    CTickCount              m_delay;
    unsigned int            m_repCount;
};

#endif //_BASE_LUA_TIMER_