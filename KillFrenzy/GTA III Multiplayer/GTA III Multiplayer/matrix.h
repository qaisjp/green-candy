#ifndef	_MATRIX_
#define _MATRIX_

#define LUACLASS_MATRIX 20

#define	DEG2RAD(deg)	(M_PI*deg/180)
#define RAD2DEG(rad)	(rad*180/M_PI)

class CMatrix	//size: 60bytes
{
public:
	vec4_t			m_vecRight;
	vec4_t			m_vecFront;
	vec4_t			m_vecUp;
	vec4_t			m_vecPos;

	void			GetOffset( float fOffX, float fOffY, float fOffZ, vec3_t vecOut ) const;
	void			SetRotation( const vec3_t vecRot );
	void			SetRotation( float fRotX, float fRotY, float fRotZ );
	void			GetRotation( vec3_t vecRot ) const;
};

void lua_creatematrix( lua_State *L, CMatrix& matrix );

#endif //_MATRIX_