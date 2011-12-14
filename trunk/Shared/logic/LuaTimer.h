/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaTimer.h
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

#ifndef _BASE_LUA_TIMER_
#define _BASE_LUA_TIMER_

#define LUA_TIMER_MIN_INTERVAL      50

class LuaTimer
{
public:

                            LuaTimer                    ( const LuaFunctionRef& iLuaFunction, const CLuaArguments& Arguments );
                            ~LuaTimer                   ( void );

    CTickCount              GetStartTime                ( void ) const                  { return m_llStartTime; };
    inline void             SetStartTime                ( CTickCount llStartTime )      { m_llStartTime = llStartTime; };

    CTickCount              GetDelay                    ( void ) const                  { return m_llDelay; };
    inline void             SetDelay                    ( CTickCount llDelay )          { m_llDelay = llDelay; };

    inline unsigned int     GetRepeats                  ( void ) const                  { return m_uiRepeats; };
    inline void             SetRepeats                  ( unsigned int uiRepeats )      { m_uiRepeats = uiRepeats; }

    void                    ExecuteTimer                ( class CLuaMain* pLuaMain );

    CTickCount              GetTimeLeft                 ( void );

    inline bool             IsBeingDeleted              ( void )                        { return m_bBeingDeleted; }
    inline void             SetBeingDeleted             ( bool bBeingDeleted )          { m_bBeingDeleted = bBeingDeleted; }
private:
    CLuaFunctionRef         m_iLuaFunction;
    CLuaArguments           m_Arguments;
    CTickCount              m_llStartTime;
    CTickCount              m_llDelay;
    unsigned int            m_uiRepeats;
    bool                    m_bBeingDeleted;
};

#endif //_BASE_LUA_TIMER_