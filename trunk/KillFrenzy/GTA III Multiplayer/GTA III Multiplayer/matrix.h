#ifndef	_MATRIX_
#define _MATRIX_

#define LUACLASS_KFMATRIX 20

class CMatrix	//size: 60bytes
{
public:
	CVector			m_vecRight;
    int             a;
	CVector		    m_vecFront;
    int             b;
	CVector         m_vecUp;
    int             c;
	CVector		    m_vecPos;
    int             d;

	void			GetOffset( float fOffX, float fOffY, float fOffZ, vec3_t vecOut ) const;
	void			SetRotation( const vec3_t vecRot );
	void			SetRotation( float fRotX, float fRotY, float fRotZ );
	void			GetRotation( vec3_t vecRot ) const;
};

void lua_creatematrix( lua_State *L, CMatrix& matrix );

#endif //_MATRIX_