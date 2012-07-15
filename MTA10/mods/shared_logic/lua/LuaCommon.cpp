/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/logic/lua/LuaCommon.cpp
*  PURPOSE:     Lua common functions
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Temporary until we change these funcs:
#include "../luadefs/CLuaDefs.h"
// End of temporary

// Prevent the warning issued when doing unsigned short -> void*
#pragma warning(disable:4312)

//#pragma message(__LOC__"Use RTTI/dynamic_casting here for safety?")
#pragma message(__LOC__"The_GTA: Convert this implementation to MTA:Lua!")


static inline CLuaMain& lua_readcontext( lua_State *L )
{
    return *lua_readuserdata <CLuaMain, LUA_REGISTRYINDEX, 2> ( L );
}

CClientEntity* lua_toelement( lua_State *L, int arg )
{
    if ( lua_type( L, arg ) != LUA_TLIGHTUSERDATA )
        return NULL;

    return CElementIDs::GetElement( TO_ELEMENTID( lua_touserdata( L, arg ) ) );
}

template <int el, class type>
static inline type* lua_getelement( lua_State *L, int arg )
{
    CClientEntity *element = lua_toelement( L, arg );

    if ( !element || element->GetType() != el )
        return NULL;

    return (type*)element;
}

// Lua push/pop macros for our datatypes
CClientRadarMarker* lua_toblip( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTRADARMARKER, CClientRadarMarker> ( L, arg );
}

CClientColModel* lua_tocolmodel( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTCOL, CClientColModel> ( L, arg );
}

// better name: detection area
CClientColShape* lua_tocolshape( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTCOLSHAPE, CClientColShape> ( L, arg );
}

CClientDFF* lua_todff( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTDFF, CClientDFF> ( L, arg );
}

CClientGUIElement* lua_toguielement( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTGUI, CClientGUIElement> ( L, arg );
}

CClientMarker* lua_tomarker( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTMARKER, CClientMarker> ( L, arg );
}

CClientObject* lua_toobject( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTOBJECT, CClientObject> ( L, arg );
}

CClientPed* lua_toped( lua_State *L, int arg )
{
    CClientEntity *entity = lua_toelement( L, arg );

    if ( !entity )
        return NULL;

    eClientEntityType eType = entity->GetType();

    if ( eType != CCLIENTPED && eType != CCLIENTPLAYER )
        return NULL;

    return (CClientPed*)entity;
}

CClientPickup* lua_topickup( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTPICKUP, CClientPickup> ( L, arg );
}

CClientPlayer* lua_toplayer( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTPLAYER, CClientPlayer> ( L, arg );
}

CClientProjectile* lua_toprojectile( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTPROJECTILE, CClientProjectile> ( L, arg );
}

LuaTimer* lua_totimer( lua_State *L, int idx )
{
    LuaTimer *timer;

    if ( lua_type( L, idx ) != LUA_TCLASS || !lua_refclass( L, idx )->GetTransmit( LUACLASS_TIMER, (void*&)timer ) )
        return NULL;

    return timer;
}

CResource* lua_toresource( lua_State *L, int arg )
{
    CResource *res = (CResource*)lua_touserdata( L, arg );

    if ( !CLuaFunctionDefs::m_pResourceManager->Exists( res ) )
        return NULL;

    return res;
}

CClientSound* lua_tosound( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTSOUND, CClientSound> ( L, arg );
}

CClientTeam* lua_toteam( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTTEAM, CClientTeam> ( L, arg );
}

CClientTXD* lua_totxd ( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTTXD, CClientTXD> ( L, arg );
}

CClientVehicle* lua_tovehicle( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTVEHICLE, CClientVehicle> ( L, arg );
}

CClientWater* lua_towater( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTWATER, CClientWater> ( L, arg );
}

CClientRadarArea* lua_toradararea( lua_State *L, int arg )
{
    return lua_getelement <CCLIENTRADARAREA, CClientRadarArea> ( L, arg );
}

CXMLNode* lua_toxmlnode( lua_State *L, int arg )
{
    return g_pCore->GetXML()->GetNodeFromID( (unsigned long)lua_touserdata( L, arg ) );
}

void lua_pushelement( lua_State *L, CClientEntity *entity )
{
    if ( !entity )
    {
        lua_pushnil( L );
        return;
    }

    ElementID ID = entity->GetID();

    if ( ID == INVALID_ELEMENT_ID )
        return;

    lua_pushlightuserdata( L, (void*)ID.Value() );
}

void lua_pushresource( lua_State *L, CResource *res )
{
    lua_pushlightuserdata( L, res );
}

void lua_pushxmlnode( lua_State *L, CXMLNode *xml )
{
    lua_pushlightuserdata( L, (void*)xml->GetID() );
}