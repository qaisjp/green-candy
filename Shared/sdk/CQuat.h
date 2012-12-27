/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/CQuat.h
*  PURPOSE:     Quaternion class
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CQuat;

#ifndef __CQUAT_H
#define __CQUAT_H

#include <float.h>
#include "SharedUtil.h"

#define EPSILON         0.001f

#ifndef PI
#define PI 3.14159265358979323846264338327950f
#endif

class CQuat
{
public:
    // SSE optimize this some time!
    // Important documents on optimization:
    //      http://cache-www.intel.com/cd/00/00/29/37/293748_293748.pdf
    //      http://cache-www.intel.com/cd/00/00/29/37/293747_293747.pdf

    CQuat()
    {
        x = y = z = w = 0;
    }

    CQuat( RwMatrix& m )
    {
        w = sqrt( SharedUtil::Max( (float)0, 1.0f + m.right.fX + m.at.fY + m.up.fZ ) ) * 0.5f;
        x = sqrt( SharedUtil::Max( (float)0, 1.0f + m.right.fX - m.at.fY - m.up.fZ ) ) * 0.5f;
        y = sqrt( SharedUtil::Max( (float)0, 1.0f - m.right.fX + m.at.fY - m.up.fZ ) ) * 0.5f;
        z = sqrt( SharedUtil::Max( (float)0, 1.0f - m.right.fX - m.at.fY + m.up.fZ ) ) * 0.5f;
        
        x = static_cast < float > ( _copysign( x, m.up.fY - m.at.fZ ) );
        y = static_cast < float > ( _copysign( y, m.right.fZ - m.up.fX ) );
        z = static_cast < float > ( _copysign( z, m.at.fX - m.right.fY ) );
    }

    static void ToMatrix(const CQuat& q, RwMatrix& m)
    {
        float xx = q.x * q.x;
        float xy = q.x * q.y;
        float xz = q.x * q.z;
        float xw = q.x * q.w;

        float yy = q.y * q.y;
        float yz = q.y * q.z;
        float yw = q.y * q.w;

        float zz = q.z * q.z;
        float zw = q.z * q.w;

        m.right.fX =       1.0f -  2.0f * ( yy + zz );
        m.right.fY =               2.0f * ( xy - zw );
        m.right.fZ =               2.0f * ( xz + yw );

        m.at.fX =                  2.0f * ( xy + zw );
        m.at.fY =          1.0f -  2.0f * ( xx + zz );
        m.at.fZ =                  2.0f * ( yz - xw );

        m.up.fX =                  2.0f * ( xz - yw );
        m.up.fY =                  2.0f * ( yz + xw );
        m.up.fZ =          1.0f -  2.0f * ( xx + yy );
    }

    // Linear interpolation
    static void LERP(const CQuat& qa, const CQuat& qb, CQuat& qm, float t)
    {
        float cosom, scale0, scale1, s;
        cosom = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;
        scale0 = 1.0f - t;
        scale1 = ( cosom >= 0.0f ) ? t : -t;
        qm.x = scale0 * qa.x + scale1 * qb.x;
        qm.y = scale0 * qa.y + scale1 * qb.y;
        qm.z = scale0 * qa.z + scale1 * qb.z;
        qm.w = scale0 * qa.w + scale1 * qb.w;
        s = 1.0f / sqrt( qm.x * qm.x + qm.y * qm.y + qm.z * qm.z + qm.w * qm.w );
        qm.x *= s;
        qm.y *= s;
        qm.z *= s;
        qm.w *= s;
    }

    // Spherical linear interpolation
    static void SLERP(const CQuat& qa, const CQuat& qb, CQuat& qm, float t)
    {
        // Calculate angle between them.
        float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
        // if qa=qb or qa=-qb then theta = 0 and we can return qa
        if (fabs(cosHalfTheta) >= 1.0f){
            qm.w = qa.w;qm.x = qa.x;qm.y = qa.y;qm.z = qa.z;
            return;
        }
        // Calculate temporary values.
        float halfTheta = acos(cosHalfTheta);
        float sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);
        // if theta = 180 degrees then result is not fully defined
        // we could rotate around any axis normal to qa or qb
        if (fabs(sinHalfTheta) < 0.001f){ // fabs is floating point absolute
            qm.w = (qa.w * 0.5f + qb.w * 0.5f);
            qm.x = (qa.x * 0.5f + qb.x * 0.5f);
            qm.y = (qa.y * 0.5f + qb.y * 0.5f);
            qm.z = (qa.z * 0.5f + qb.z * 0.5f);
            return;
        }
        float ratioA = sin((1.0f - t) * halfTheta) / sinHalfTheta;
        float ratioB = sin(t * halfTheta) / sinHalfTheta; 
        //calculate Quaternion.
        qm.w = (qa.w * ratioA + qb.w * ratioB);
        qm.x = (qa.x * ratioA + qb.x * ratioB);
        qm.y = (qa.y * ratioA + qb.y * ratioB);
        qm.z = (qa.z * ratioA + qb.z * ratioB);
    }

    float x, y, z, w;
};

#endif
