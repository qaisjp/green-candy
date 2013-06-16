#include "StdInc.h"

static int matrix_offset( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    vec3_t pos;
    ((CMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetOffset( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ), pos );

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static LUA_DECLARE( setPosition )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    ((CMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_vecPos = CVector( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    LUA_SUCCESS;
}

static int matrix_destroy( lua_State *L )
{
    delete (CMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg matrix_interface[] =
{
    { "offset", matrix_offset },
    LUA_METHOD( setPosition ),
    { "destroy", matrix_destroy },
    { NULL, NULL }
};

static int luaconstructor_matrix( lua_State *L )
{
    CMatrix *matrix = (CMatrix*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_KFMATRIX, matrix );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, matrix_interface, 1 );

    lua_pushlstring( L, "matrix", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void lua_creatematrix( lua_State *L, CMatrix& matrix )
{
    lua_pushlightuserdata( L, new CMatrix( matrix ) );
    lua_pushcclosure( L, luaconstructor_matrix, 1 );
    lua_newclass( L );
}

void		CMatrix::GetOffset( float fOffX, float fOffY, float fOffZ, vec3_t vecOut ) const
{
	vecOut[0] =-m_vecRight[0] * fOffX - m_vecRight[1] * fOffY - m_vecRight[2] * fOffZ + m_vecPos[0];
	vecOut[1] = m_vecFront[0] * fOffX + m_vecFront[1] * fOffY + m_vecFront[2] * fOffZ + m_vecPos[1];
	vecOut[2] = m_vecUp[0] * fOffX + m_vecUp[1] * fOffY + m_vecUp[2] * fOffZ + m_vecPos[2];
}

void		CMatrix::SetRotation( const vec3_t vecRot )
{
	SetRotation(vecRot[0], vecRot[1], vecRot[2]);
}

void		CMatrix::SetRotation( float fRotX, float fRotY, float fRotZ )
{
	double ch = cos(DEG2RAD(fRotX));	// ATTITUDE
	double sh = sin(DEG2RAD(fRotX));
	double ca = cos(DEG2RAD(fRotZ));	// HEADING
	double sa = sin(DEG2RAD(fRotZ));
	double cb = cos(DEG2RAD(fRotY));	// BANK
	double sb = sin(DEG2RAD(fRotY));

	m_vecRight[0] = (float)(ch * ca);
	m_vecRight[1] = (float)(sh*sb - ch*sa*cb);
	m_vecRight[2] = (float)(ch*sa*sb + sh*cb);
	m_vecFront[0] = (float)(sa);
	m_vecFront[1] = (float)(ca*cb);
	m_vecFront[2] = (float)(-ca*sb);
	m_vecUp[0] = (float)(-sh*ca);
	m_vecUp[1] = (float)(sh*sa*cb + ch*sb);
	m_vecUp[2] = (float)(-sh*sa*sb + ch*cb);
}

void		CMatrix::GetRotation( vec3_t vecRot ) const
{
	vecRot[0] = (float)RAD2DEG(atan2(-m_vecUp[0], m_vecRight[0]));
	vecRot[2] = (float)RAD2DEG(asin(m_vecFront[0]));
	vecRot[1] = (float)RAD2DEG(atan2(-m_vecFront[2], m_vecFront[1]));
}