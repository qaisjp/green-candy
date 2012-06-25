/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaTimerManager.cpp
*  PURPOSE:     Lua timer manager class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Marcus Bauer <mabako@gmail.com>
*               Florian Busse <flobu@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

void LuaTimerManager::DoPulse( LuaMain *main )
{
    CTickCount curTime = CTickCount::Now();
    luaRefs refs;

    std::list <LuaTimer*>::iterator iter = m_list.begin();

    for ( ; iter != m_list.end(); iter++ )
    {
        LuaTimer *timer = *iter;

        // We have to reference the timer to keep it
        timer->Reference( refs );

        if ( curTime < ( timer->GetStartTime() + timer->GetDelay() ) )
            continue;

        timer->Execute( main );
        timer->SetStartTime( curTime );
    }

    // the "trash" will be taken out by lua here, in case the timer is not being used anywhere else
}

void LuaTimerManager::RemoveTimer( LuaTimer *timer )
{
    // This can either destroy it right away, or wait till unreferenced
    timer->Destroy();
}

void LuaTimerManager::RemoveAllTimers()
{
    luaRefs refs;
    std::list <LuaTimer*>::const_iterator iter = m_list.begin();

    for ( ; iter != m_list.end(); iter++ )
    {
        // We may not delete it right away, as it clears itself from list
        (*iter)->Reference( refs );
        (*iter)->Destroy();
    }
}

void LuaTimerManager::ResetTimer( LuaTimer *timer )
{
    timer->SetStartTime( CTickCount::Now() );
}

bool LuaTimerManager::Exists( LuaTimer *timer )
{
    return m_list.Contains( timer );
}

LuaTimer* LuaTimerManager::AddTimer( lua_State *L, const LuaFunctionRef& ref, CTickCount delay, unsigned int repCount, const LuaArguments& args )
{
    // Check for the minimum interval
    if ( delay.ToLongLong() < LUA_TIMER_MIN_INTERVAL )
        return NULL;

    if ( !VERIFY_FUNCTION( ref ) )
        return NULL;

    // Add the timer
    LuaTimer *timer = new LuaTimer( L, this, ref );
    timer->SetStartTime( CTickCount::Now () );
    timer->SetDelay( delay );
    timer->SetRepeats( repCount );
    m_list.push_back( timer );
    return timer;
}

void LuaTimerManager::GetTimers( CTickCount time, LuaMain *main )
{
    // We expect a table on the stack
    CTickCount curTime = CTickCount::Now();
    std::list <LuaTimer*>::const_iterator iter = m_list.begin();
    unsigned int n = 0;

    for ( ; iter != m_list.end(); iter++ )
    {
        // If the time left is less than the time specified, or the time specifed is 0
        if ( time.ToLongLong() != 0 && ( (*iter)->GetStartTime () + (*iter)->GetDelay () ) - curTime > time )
            continue;

        lua_State *L = main->GetVirtualMachine();
        lua_pushnumber( L, ++n );
        (*iter)->PushStack( L );
        lua_settable( L, -3 );
    }
}
