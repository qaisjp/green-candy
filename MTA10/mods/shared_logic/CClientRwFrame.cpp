/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientRwFrame.cpp
*  PURPOSE:     RenderWare frame management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( setName )
{
    const char *name;

    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    LUA_ARGS_END;

    ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->SetName( name );
    LUA_SUCCESS;
}

static LUA_DECLARE( getName )
{
    lua_pushstring( L, ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetName() );
    return 1;
}

static LUA_DECLARE( getHash )
{
    lua_pushnumber( L, ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetHash() );
    return 1;
}

static LUA_DECLARE( setLTM )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetObject().SetLTM( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getLTM )
{
    lua_creatematrix( L, ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetObject().GetLTM() );
    return 1;
}

static LUA_DECLARE( setModelling )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetObject().SetModelling( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getModelling )
{
    lua_creatematrix( L, ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetObject().GetModelling() );
    return 1;
}

static LUA_DECLARE( isValidChild )
{
    if ( lua_type( L, 1 ) != LUA_TCLASS )
        return 0;

    ILuaClass& j = *lua_refclass( L, 1 );

    if ( j.IsTransmit( LUACLASS_RWFRAME ) )
    {
        lua_pushboolean( L, true );
        return 1;
    }

    CClientRwObject *obj;

    lua_pushboolean( L, j.GetTransmit( LUACLASS_RWOBJECT, obj ) && obj->GetObject().IsFrameExtension() );
    return 1;
}

LUA_DECLARE( CClientRwFrame::unlinkParentFrame )
{
    CClientRwFrame::children_t& list = ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_children;
    CClientRwFrame *child = ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 2 ) ));

    // Untie internally and in our API
    child->m_frame.Unlink();
    list.erase( std::find( list.begin(), list.end(), child ) );
    return 0;
}

LUA_DECLARE( CClientRwFrame::unlinkParentObject )
{
    CClientRwFrame::objects_t& list = ((CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_objects;
    CClientRwObject *obj = (CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 2 ) );

    // Untie stuff
    obj->GetObject().SetFrame( NULL );
    list.erase( std::find( list.begin(), list.end(), obj ) );
    return 0;
}

LUA_DECLARE( CClientRwFrame::setChild )
{
    ILuaClass& j = *lua_refclass( L, 1 );
    CClientRwFrame *frame = (CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // Make sure we end up with child(1), childAPI(2) stack values!
    lua_settop( L, 1 );
    lua_pushvalue( L, 1 );

    // Grab the childAPI; this will check the first argument for class validity
    lua_getfield( L, LUA_ENVIRONINDEX, "super" );
    lua_pushvalue( L, 1 );
    lua_call( L, 1, 1 );

    union
    {
        CClientRwFrame *child;
        CClientRwObject *obj;
    };

    if ( j.GetTransmit( LUACLASS_RWFRAME, child ) )
    {
        // frame holds child (CClientRwFrame -> CClientRwFrame)
        frame->m_children.insert( frame->m_children.begin(), child );
        frame->m_frame.Link( &child->m_frame );

        lua_pushlightuserdata( L, frame );
        lua_pushlightuserdata( L, child );
        lua_pushcclosure( L, unlinkParentFrame, 2 );
    }
    else if ( j.GetTransmit( LUACLASS_RWOBJECT, obj ) )
    {
        // frame holds obj (CClientRwFrame -> CClientRwObjectEx)
        frame->m_objects.insert( frame->m_objects.begin(), obj );
        obj->GetObject().SetFrame( &frame->m_frame );
        
        lua_pushlightuserdata( L, frame );
        lua_pushlightuserdata( L, obj );
        lua_pushcclosure( L, unlinkParentObject, 2 );
    }
    else
        assert( 0 );

    // A disconnector has to be provided above
    lua_setfield( L, 2, "notifyDestroy" );

    // <3 unions are lovely!
    obj->m_parent = frame;
    return 1;
}

static const luaL_Reg rwframe_interface[] =
{
    LUA_METHOD( getName ),
    LUA_METHOD( setName ),
    LUA_METHOD( getHash ),
    LUA_METHOD( setLTM ),
    LUA_METHOD( getLTM ),
    LUA_METHOD( setModelling ),
    LUA_METHOD( getModelling ),
    LUA_METHOD( isValidChild ),
    LUA_METHOD( setChild ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_rwframe )
{
    CClientRwFrame *frame = (CClientRwFrame*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_RWFRAME, frame );

    lua_pushvalue( L, LUA_ENVIRONINDEX );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, rwframe_interface, 1 );

    lua_pushlstring( L, "rwframe", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static inline void RwFrameAssignObject( CRwObject *obj, CClientRwFrame *parent )
{
    if ( obj->GetType() == RW_ATOMIC )
    {
        CClientAtomic *atom = new CClientAtomic( parent->GetVM(), NULL, *(CRpAtomic*)obj );

        atom->SetRoot( parent );
        return;
    }

    assert( 0 );
}

static inline void RwFrameAssignChild( CRwFrame *child, CClientRwFrame *parent )
{
    lua_State *L = parent->GetVM();
    CClientRwFrame *frame = new CClientRwFrame( L, *child );

    // Establish the connection; this should do all the magic
    frame->PushMethod( L, "setParent" );
    parent->PushStack( L );
    lua_call( L, 1, 0 );
}

CClientRwFrame::CClientRwFrame( lua_State *L, CRwFrame& frame ) : CClientRwObject( L, frame )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_rwframe, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    // Obtain all children...
    {
        const CRwFrame::childList_t& list = frame.GetChildren();

        for ( CRwFrame::childList_t::iterator iter = list.begin(); iter != list.end(); iter++ )
            RwFrameAssignChild( *iter, this );
    }

    // ... and objects
    {
        const CRwFrame::objectList_t& list = frame.GetObjects();

        for ( CRwFrame::childList_t::iterator iter = list.begin(); iter != list.end(); iter++ )
            RwFrameAssignObject( *iter, this );
    }
}

CClientRwFrame::~CClientRwFrame()
{
}