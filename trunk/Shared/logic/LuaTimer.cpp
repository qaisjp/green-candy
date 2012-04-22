/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaTimer.cpp
*  PURPOSE:     Lua timed execution
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

LuaTimer::LuaTimer( const LuaFunctionRef& ref )
{
    m_ref = ref;
    m_iLuaFunction = iLuaFunction;
}

LuaTimer::~LuaTimer()
{
}

void LuaTimer::ExecuteTimer( LuaMain *main )
{
    if ( !VERIFY_FUNCTION( m_ref ) )
        return;

    m_args->Call( main, m_ref );
}

CTickCount CLuaTimer::GetTimeLeft()
{
    return m_llStartTime + m_llDelay - CTickCount::Now();
}
