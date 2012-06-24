#include "StdInc.h"

void		CMatrix::GetOffset(float fOffX, float fOffY, float fOffZ, vec3_t vecOut)
{
	vecOut[0] =-m_vecRight[0] * fOffX - m_vecRight[1] * fOffY - m_vecRight[2] * fOffZ + m_vecPos[0];
	vecOut[1] = m_vecFront[0] * fOffX + m_vecFront[1] * fOffY + m_vecFront[2] * fOffZ + m_vecPos[1];
	vecOut[2] = m_vecUp[0] * fOffX + m_vecUp[1] * fOffY + m_vecUp[2] * fOffZ + m_vecPos[2];
}

void		CMatrix::SetRotation(vec3_t vecRot)
{
	SetRotation(vecRot[0], vecRot[1], vecRot[2]);
}

void		CMatrix::SetRotation(float fRotX, float fRotY, float fRotZ)
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

void		CMatrix::GetRotation(vec3_t vecRot)
{
	vecRot[0] = (float)RAD2DEG(atan2(-m_vecUp[0], m_vecRight[0]));
	vecRot[2] = (float)RAD2DEG(asin(m_vecFront[0]));
	vecRot[1] = (float)RAD2DEG(atan2(-m_vecFront[2], m_vecFront[1]));
}