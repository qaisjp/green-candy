/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare_shared.h
*  PURPOSE:     Shared renderware definitions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RenderWare_Shared_H_
#define _RenderWare_Shared_H_

// TODO: Remove the internal RW definitions, DIF
#include <CMatrix.h>

// RenderWare primitive types
struct RwV2d
{
    float x,y;
};
struct RwV3d
{
    float x,y,z;
};
typedef float RwV4d[4];
struct RwPlane
{
    RwV3d normal;
    float length;
};
struct RwBBox
{
    RwV3d max;
    RwV3d min;
};
struct RwSphere
{
    RwV3d position;
    float radius;
};
struct RwColorFloat
{
    float r,g,b,a;
};
struct RwColor
{
    unsigned char r, g, b, a;
};
class RwMatrix
{   // 16-byte padded
public:
    inline void Identity()
    {
        right.fX = 1; right.fY = 0; right.fZ = 0;
        up.fX = 0; up.fY = 1; up.fZ = 0;
        at.fX = 0; at.fY = 0; at.fZ = 1;

        pos.fX = (float)(1.15 * -0.25);
        pos.fY = 0;
        pos.fZ = 0;
    }

    RwMatrix()
    {
        Identity();
    }

    RwMatrix( const CMatrix& from )
    {
        right = from.vRight;
        up = from.vUp;
        at = from.vFront;
        pos = from.vPos;
    }

    inline void rotX( float radians )
    {
        float c = cos( radians );
        float s = sin( radians );

        right.fX = 1;
        right.fY = 0;
        right.fZ = 0;

        up.fX = 0;
        up.fY = c;
        up.fZ = -s;

        at.fX = 0;
        at.fY = s;
        at.fZ = c;
    }

    inline void rotY( float radians )
    {
        float c = cos( radians );
        float s = sin( radians );

        right.fX = c;
        right.fY = 0;
        right.fZ = s;

        up.fX = 0;
        up.fY = 1;
        up.fZ = 0;

        at.fX = -s;
        at.fY = 0;
        at.fZ = c;
    }

    inline void rotZ( float radians )
    {
        float c = cos( radians );
        float s = sin( radians );

        right.fX = c;
        right.fY = -s;
        right.fZ = 0;

        up.fX = s;
        up.fY = c;
        up.fZ = 0;

        at.fX = 0;
        at.fY = 0;
        at.fZ = 1;
    }

    // I hope this works :3
    inline void Multiply( const RwMatrix mat, RwMatrix dst )
    {
	    __asm
	    {
		    mov eax,this
		    mov edx,[mat]
		    mov esi,[dst]

		    movups xmm4,[edx]
		    movups xmm5,[edx+0x10]
		    movups xmm6,[edx+0x20]
    		
		    movss xmm3,[eax+0x10]
		    movss xmm7,[eax+0x20]

		    // X
		    movss xmm0,[eax]
		    movss xmm1,[eax+4]
		    movss xmm2,[eax+8]

		    shufps xmm0,xmm0,0x40
		    shufps xmm1,xmm1,0x40
		    shufps xmm2,xmm2,0x40
    		
		    // Do cache vars
		    shufps xmm3,xmm3,0x40
		    shufps xmm7,xmm7,0x40

		    mulps xmm0,xmm4
		    mulps xmm1,xmm5
		    mulps xmm2,xmm6

		    addps xmm0,xmm1
		    addps xmm0,xmm2

		    movups [esi],xmm0

		    // Y
		    movss xmm1,[eax+0x14]
		    movss xmm2,[eax+0x18]

		    shufps xmm1,xmm1,0x40
		    shufps xmm2,xmm2,0x40

		    mulps xmm3,xmm4
		    mulps xmm1,xmm5
		    mulps xmm2,xmm6

		    addps xmm3,xmm1
		    addps xmm3,xmm2

		    movups [esi+0x10],xmm3

		    // Z
		    movss xmm1,[eax+0x24]
		    movss xmm2,[eax+0x28]

		    shufps xmm1,xmm1,0x40
		    shufps xmm2,xmm2,0x40

		    mulps xmm7,xmm4
		    mulps xmm1,xmm5
		    mulps xmm2,xmm6

		    addps xmm7,xmm1
		    addps xmm7,xmm2

		    movups [esi+0x20],xmm7
	    }
    }

    float & operator [] ( unsigned int i )
    {
        return ((float*)(this))[i];
    }

    float operator [] ( unsigned int i ) const
    {
        return ((float*)(this))[i];
    }

    CVector         right;
    unsigned int    a;
    CVector         up;
    unsigned int    b;
    CVector         at;
    unsigned int    c;
    CVector         pos;
    unsigned int    d;
};

#endif