/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CMapEventManager.cpp
*  PURPOSE:     Map event manager class
*  DEVELOPERS:  Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Chris McArthur <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CMapEventManager::CMapEventManager( CClientEntity *owner )
{
    m_owner = owner;
}

CMapEventManager::~CMapEventManager()
{
    // Delete all eventhandlers
    DeleteAll ();
}

bool CMapEventManager::Add( CLuaMain *main, const char *name, const LuaFunctionRef& ref, bool propagated )
{
    // Check for max name length
    if ( strlen( name ) > MAPEVENT_MAX_LENGTH_NAME )
        return false;

    // Make a new event
    CMapEvent *pEvent = new CMapEvent( main, *this, name, ref, propagated );
    pEvent->SetRoot( main->GetResource() );

    m_Events.push_back( pEvent );
    return true;
}

bool CMapEventManager::Delete( CLuaMain *main, const char *name, const LuaFunctionRef *ref )
{
    luaRefs refs;

    bool bRemovedSomeone = false;
    events_t::iterator iter = m_Events.begin();

    for ( ; iter != m_Events.end(); iter++ )
    {
        CMapEvent *pMapEvent = *iter;

        if ( main != pMapEvent->GetVM() )
            continue;

        // If we have a name, delete only events which match it
        if ( name && strcmp( pMapEvent->GetName(), name ) != 0 )
            continue;

        if ( ref && *ref != pMapEvent->GetLuaFunction() )
            continue;

        // Delete it
        pMapEvent->Reference( refs );
        pMapEvent->Delete();

        // Remember that we deleted something
        bRemovedSomeone = true;
    }

    // Return whether we actually destroyed someone or not
    return bRemovedSomeone;
}

void CMapEventManager::DeleteAll()
{
    luaRefs refs;

    // Delete all the events
    events_t::iterator iter = m_Events.begin();
    
    for ( ; iter != m_Events.end(); iter++ )
    {
        (*iter)->Reference( refs );
        (*iter)->Delete();
    }
}

CMapEvent* CMapEventManager::Get( const char *name )
{
    // Return it if we find it in the list
    events_t::const_iterator iter = m_Events.begin();

    for ( ; iter != m_Events.end(); iter++ )
    {
        // Compare the names
        if ( strcmp( (*iter)->GetName(), name ) == 0 )
            return *iter;
    }

    return NULL;
}

bool CMapEventManager::Call( lua_State *callee, unsigned int argCount, const char *name, CClientEntity *source )
{
    luaRefs refs;

    // Call all the events with matching names
    bool bCalled = false;
    events_t::const_iterator iter = m_Events.begin();

    for ( ; iter != m_Events.end(); iter++ )
    {
        CMapEvent *pMapEvent = *iter;

        // Compare the names
        if ( strcmp( pMapEvent->GetName(), name ) == 0 )
        {
            // Call if propagated?
            if ( source == m_owner || pMapEvent->IsPropagated() )
            {
                // Grab the event's executive environment
                CLuaMain *main = pMapEvent->GetVM();
                lua_State *L = **main;
                int top = lua_gettop( L );

                TIMEUS startTime = GetTimeUs();

                // Make sure our stack has got enough size
                lua_checkstack( L, 6 + argCount );

                // Store globals
                lua_getfield( L, LUA_GLOBALSINDEX, "source" );
                lua_getfield( L, LUA_GLOBALSINDEX, "this" );
                lua_getfield( L, LUA_GLOBALSINDEX, "sourceResource" );
                lua_getfield( L, LUA_GLOBALSINDEX, "sourceResourceRoot" );
                lua_getfield( L, LUA_GLOBALSINDEX, "eventName" );

                // Init globals
                source->PushStack( L );                                                             lua_setglobal( L, "source" );
                m_owner->PushStack( L );                                                            lua_setglobal( L, "this" );
                main->GetResource()->PushStack( L );                                                lua_setglobal( L, "sourceResource" );
                main->GetResource()->GetResourceDynamicEntity()->PushStack( L );                    lua_setglobal( L, "sourceResourceRoot" );
                lua_pushstring( L, name );                                                          lua_setglobal( L, "eventName" );

                // Reference it to prevent destruction
                pMapEvent->Reference( refs );

                // Get it's function
                main->PushReference( pMapEvent->m_funcRef );

                // Grab the arguments
                if ( callee != L )
                    lua_xcopy( callee, L, argCount );   // perform a quick copy
                else
                {
                    // We need to snatch the arguments from below
                    unsigned int n = argCount;

                    while ( n-- )
                        lua_pushvalue( L, top - n );
                }

                // Call it
                main->PCallStackVoid( argCount );
                bCalled = true;

                // Reset globals
                lua_setfield( L, LUA_GLOBALSINDEX, "eventName" );
                lua_setfield( L, LUA_GLOBALSINDEX, "sourceResourceRoot" );            
                lua_setfield( L, LUA_GLOBALSINDEX, "sourceResource" );
                lua_setfield( L, LUA_GLOBALSINDEX, "this" );
                lua_setfield( L, LUA_GLOBALSINDEX, "source" );

#if MTA_DEBUG
                assert( lua_gettop( L ) == top );
#endif

                CClientPerfStatLuaTiming::GetSingleton()->UpdateLuaTiming( main, name, GetTimeUs() - startTime );
            }
        }
    }

    // Return whether we called atleast one func or not
    return bCalled;
}