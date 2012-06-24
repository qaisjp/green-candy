#ifndef	_MATRIX_
#define _MATRIX_

#define	DEG2RAD(deg)	(M_PI*deg/180)
#define RAD2DEG(rad)	(rad*180/M_PI)

class CMatrix	//size: 60bytes
{
public:
	vec4_t			m_vecRight;
	vec4_t			m_vecFront;
	vec4_t			m_vecUp;
	vec4_t			m_vecPos;

	void			GetOffset(float fOffX, float fOffY, float fOffZ, vec3_t vecOut);
	void			SetRotation(vec3_t vecRot);
	void			SetRotation(float fRotX, float fRotY, float fRotZ);
	void			GetRotation(vec3_t vecRot);
};

#endif //_MATRIX_