/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaMatrix.cpp
*  PURPOSE:     Lua RwMatrix instancing
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

inline static void lua_pushvector3D( lua_State *L, CVector& vec )
{
    lua_createtable( L, 3, 0 );
    lua_pushnumber( L, vec[0] );
    lua_pushnumber( L, vec[1] );
    lua_pushnumber( L, vec[2] );
    lua_rawseti( L, -4, 1 );
    lua_rawseti( L, -3, 2 );
    lua_rawseti( L, -2, 3 );
}

static int matrix_index( lua_State *L )
{
    if ( !lua_isnumber( L, 2 ) )
    {
        lua_getfield( L, LUA_ENVIRONINDEX, "super" );
        lua_insert( L, 1 );
        lua_call( L, 2, 1 );
        return 1;
    }

    RwMatrix *mat = (RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    switch( lua_tointeger( L, 2 ) )
    {
    case 1:
        lua_pushvector3D( L, mat->right );
        break;
    case 2:
        lua_pushvector3D( L, mat->at );
        break;
    case 3:
        lua_pushvector3D( L, mat->up );
        break;
    case 4:
        lua_pushvector3D( L, mat->pos );
        break;
    default:
        return 0;
    }

    return 1;
}

static int matrix_setPosition( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->pos = CVector( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    return 0;
}

static int matrix_getPosition( lua_State *L )
{
    CVector& pos = ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->pos;

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static int matrix_setEulerAngles( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->SetRotation( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    return 0;
}

static int matrix_setEulerAnglesRad( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->SetRotationRad( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    return 0;
}

static int matrix_getEulerAngles( lua_State *L )
{
    float x, y, z;

    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetRotation( x, y, z );

    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_pushnumber( L, z );
    return 3;
}

static int matrix_getEulerAnglesRad( lua_State *L )
{
    float x, y, z;

    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetRotationRad( x, y, z );

    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_pushnumber( L, z );
    return 3;
}

static int matrix_fromQuat( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 4 );

    CQuat quat;
    quat.x = (float)lua_tonumber( L, 1 );
    quat.y = (float)lua_tonumber( L, 2 );
    quat.z = (float)lua_tonumber( L, 3 );
    quat.w = (float)lua_tonumber( L, 4 );

    CQuat::ToMatrix( quat, *(RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ) );
    return 0;
}

static int matrix_offset( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    CVector pos;
    ((RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetOffset( CVector( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) ), pos );

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static int matrix_destroy( lua_State *L )
{
    delete (RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg matrix_interface_light[] =
{
    { "__index", matrix_index },
    { "setPosition", matrix_setPosition },
    { "getPosition", matrix_getPosition },
    { "setEulerAngles", matrix_setEulerAngles },
    { "setEulerAnglesRad", matrix_setEulerAnglesRad },
    { "getEulerAngles", matrix_getEulerAngles },
    { "getEulerAnglesRad", matrix_getEulerAnglesRad },
    { "offset", matrix_offset },
    { "destroy", matrix_destroy },
    { NULL, NULL }
};

static int luaconstructor_matrix( lua_State *L )
{
    RwMatrix *matrix = (RwMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_MATRIX, matrix );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_basicprotect( L );

    j.RegisterLightInterface( L, matrix_interface_light, matrix );

    lua_pushlstring( L, "matrix", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void lua_creatematrix( lua_State *L, const RwMatrix& matrix )
{
    lua_pushlightuserdata( L, new RwMatrix( matrix ) );
    lua_pushcclosure( L, luaconstructor_matrix, 1 );
    lua_newclass( L );
}