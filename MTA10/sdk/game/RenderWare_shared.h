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

static const float negOne = -1.0f;

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
        right = from.right;
        up = from.up;
        at = from.at;
        pos = from.pos;
    }

    RwMatrix( const RwMatrix& mat )
    {
        right = mat.right;
        up = mat.up;
        at = mat.at;
        pos = mat.pos;
    }

    RwMatrix operator + ( const RwMatrix& mat )
    {
        float outmt[16];

        __asm
        {
            mov eax,mat
            mov ebx,this
            mov edx,outmt

            movups xmm0,[eax]
            movups xmm1,[eax+0x10]
            movups xmm2,[eax+0x20]
            movups xmm3,[eax+0x30]

            movups xmm4,[ebx]
            movups xmm5,[ebx+0x10]
            movups xmm6,[ebx+0x20]
            movups xmm7,[ebx+0x30]

            addps xmm0,xmm4
            addps xmm1,xmm5
            addps xmm2,xmm6
            addps xmm3,xmm7

            movups [edx],xmm0
            movups [edx+0x10],xmm1
            movups [edx+0x20],xmm2
            movups [edx+0x30],xmm3
        }

        return *(RwMatrix*)outmt;
    }

    RwMatrix operator - ( const RwMatrix& mat )
    {
        float outmt[16];

        __asm
        {
            mov eax,mat
            mov ebx,this
            mov edx,outmt

            movups xmm0,[eax]
            movups xmm1,[eax+0x10]
            movups xmm2,[eax+0x20]
            movups xmm3,[eax+0x30]

            movups xmm4,[ebx]
            movups xmm5,[ebx+0x10]
            movups xmm6,[ebx+0x20]
            movups xmm7,[ebx+0x30]

            subps xmm0,xmm4
            subps xmm1,xmm5
            subps xmm2,xmm6
            subps xmm3,xmm7

            movups [edx],xmm0
            movups [edx+0x10],xmm1
            movups [edx+0x20],xmm2
            movups [edx+0x30],xmm3
        }

        return *(RwMatrix*)outmt;
    }

    RwMatrix operator * ( const RwMatrix& mat )
    {
        RwMatrix outmt;
        Multiply( mat, outmt );
        return outmt;
    }

    RwMatrix operator / ( RwMatrix other )
    {
        other.Invert();

        return *this * other;
    }

    CVector operator * ( const CVector& vec )
    {
        return CVector(
            right.fX * vec.fX + at.fX * vec.fY + up.fX * vec.fZ,
            right.fY * vec.fX + at.fY * vec.fY + up.fY * vec.fZ,
            right.fZ * vec.fX + at.fZ * vec.fY + up.fZ * vec.fZ
        );
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
    inline void Multiply( const RwMatrix& mat, RwMatrix& dst )
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

    inline void Invert()
    {
        // Optimization to use SSE registers instead of stack space
        __asm
        {
            mov eax,this

            movups xmm0,[eax]
            movups xmm1,[eax+0x10]
            movups xmm2,[eax+0x20]
            movups xmm3,[eax+0x30]

            // Prepare for position invert
            movss xmm4,negOne

            movss [eax],xmm0
            movss [eax+0x04],xmm1
            movss [eax+0x08],xmm2

            // Left-shift the vectors
            shufps xmm0,xmm0,0x49
            shufps xmm1,xmm1,0x49
            shufps xmm2,xmm2,0x49

            // Pos invert prep
            shufps xmm4,xmm4,0x40

            movss [eax+0x10],xmm0
            movss [eax+0x14],xmm1
            movss [eax+0x18],xmm2

            // Left-shift the vectors
            shufps xmm0,xmm0,0x49
            shufps xmm1,xmm1,0x49
            shufps xmm2,xmm2,0x49

            movss [eax+0x20],xmm0
            movss [eax+0x24],xmm1
            movss [eax+0x28],xmm2

            // Invert the position
            mulps xmm3,xmm4
            movups [eax+0x30],xmm3
        }
    }

    float& operator [] ( unsigned int i )
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