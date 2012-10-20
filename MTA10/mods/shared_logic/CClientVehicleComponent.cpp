/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicleComponent.cpp
*  PURPOSE:     Vehicle component class export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( getName )
{
    lua_pushstring( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetName() );
    return 1;
}

static LUA_DECLARE( setPosition )
{
    CVector pos;

    LUA_ARGS_BEGIN;
    argStream.ReadVector( pos );
    LUA_ARGS_END;

    ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->SetPosition( pos );
    LUA_SUCCESS;
}

static LUA_DECLARE( getPosition )
{
    const CVector& pos = ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetPosition();

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static LUA_DECLARE( getWorldPosition )
{
    const CVector& pos = ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetWorldPosition();

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static LUA_DECLARE( setMatrix )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->SetMatrix( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getMatrix )
{
    lua_creatematrix( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetMatrix() );
    return 1;
}

static LUA_DECLARE( getWorldMatrix )
{
    lua_creatematrix( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetWorldMatrix() );
    return 1;
}

static LUA_DECLARE( setActive )
{
    bool active;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( active );
    LUA_ARGS_END;

    ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->SetActive( active );
    LUA_SUCCESS;
}

static LUA_DECLARE( isActive )
{
    lua_pushboolean( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->IsActive() );
    return 1;
}

static LUA_DECLARE( addAtomic )
{
    CClientAtomic *atomic;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( atomic, LUACLASS_ATOMIC );
    LUA_ARGS_END;

    lua_pushnumber( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->AddAtomic( atomic ) );
    return 1;
}

static LUA_DECLARE( getAtomicCount )
{
    lua_pushnumber( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetAtomicCount() );
    return 1;
}

static LUA_DECLARE( cloneAtomic )
{
    unsigned int idx;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( idx, 0 );
    LUA_ARGS_END;

    CRpAtomic *inst = ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->CloneAtomic( idx );

    LUA_CHECK( inst );

    CClientAtomic *atom = new CClientAtomic( L, NULL, *inst );
    atom->SetRoot( g_pClientGame->GetRootEntity() );

    atom->PushStack( L );
    return 1;
}

static LUA_DECLARE( removeAtomic )
{
    unsigned int idx;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( idx );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->RemoveAtomic( idx ) );
    return 1;
}

static luaL_Reg component_interface[] =
{
    LUA_METHOD( getName ),
    LUA_METHOD( setPosition ),
    LUA_METHOD( getPosition ),
    LUA_METHOD( getWorldPosition ),
    LUA_METHOD( setMatrix ),
    LUA_METHOD( getMatrix ),
    LUA_METHOD( getWorldMatrix ),
    LUA_METHOD( setActive ),
    LUA_METHOD( isActive ),
    LUA_METHOD( addAtomic ),
    LUA_METHOD( getAtomicCount ),
    LUA_METHOD( cloneAtomic ),
    LUA_METHOD( removeAtomic ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_component )
{
    CClientVehicleComponent *comp = (CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_VEHICLECOMPONENT, comp );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, component_interface, 1 );

    lua_pushlstring( L, "vehicle-component", 17 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientVehicleComponent::CClientVehicleComponent( CClientVehicle *veh, CVehicleComponent *comp ) : LuaElement( veh->GetVM() )
{
    // Lua instancing
    lua_State *L = veh->GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_component, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_vehicle = veh;
    m_component = comp;

    // Link to vehicle
    veh->m_compContainer[comp->GetName()] = this;
}

CClientVehicleComponent::~CClientVehicleComponent()
{
    // Unlink from vehicle
    GetVehicle()->m_compContainer.erase( m_component->GetName() );

    delete m_component;

    // Clear resources
    for ( atomics_t::const_iterator iter = m_atomics.begin(); iter != m_atomics.end(); iter++ )
        (*iter).atomic->DecrementMethodStack();
}

unsigned int CClientVehicleComponent::AddAtomic( CClientAtomic *atom )
{
    unsigned int idx = m_component->AddAtomic( &atom->m_atomic );

    // CClientAtomic is a template structure for any cloned atomics
    // We have to reference it, so that resources associated with the atomic will not
    // get corrupted by destruction (textures, mainly).
    atom->IncrementMethodStack();

    // Atomics usually are inserted at the front of the clump and frames
    // We should update the indices; this should not break the other case though
    OffsetIndex( idx, 1 );

    atomicInfo info;
    info.atomic = atom;
    info.idx = idx;
    
    // Sort the list correctly
    m_atomics.insert( m_atomics.begin() + min( m_atomics.size(), idx ), info );
    return idx;
}

unsigned int CClientVehicleComponent::GetAtomicCount() const
{
    return m_component->GetAtomicCount();
}

bool CClientVehicleComponent::RemoveAtomic( unsigned int idx )
{
    if ( !m_component->RemoveAtomic( idx ) )
        return false;

    atomics_t::iterator iter = m_atomics.begin();

    for ( ; iter != m_atomics.end(); iter++ )
    {
        if ( (*iter).idx != idx )
            continue;

        // Dereference the residing atomic
        (*iter).atomic->DecrementMethodStack();

        m_atomics.erase( iter );
        goto success;
    }
    return true;

success:
    // Update the indices; this relies on a properly sorted list
    for ( ; iter != m_atomics.end(); iter++ )
    {
        if ( (*iter).idx < idx )
            continue;

        (*iter).idx--;
    }

    return true;
}

void CClientVehicleComponent::OffsetIndex( unsigned int start, int offset )
{
    for ( atomics_t::iterator iter = m_atomics.begin(); iter != m_atomics.end(); iter++ )
    {
        if ( (*iter).idx < start )
            continue;

        (*iter).idx += offset;
    }
}