/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/RenderWare_shared.h
*  PURPOSE:     Shared renderware definitions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RenderWare_Shared_H_
#define _RenderWare_Shared_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <rwlist.hpp>

// TODO: Remove the internal RW definitions, DIF
#include <CVector.h>

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
    CVector pos;
    float radius;

    bool operator ==( const RwSphere& right ) const
    {
        return pos == right.pos && radius == right.radius;
    }
};
struct RwColorFloat
{
    RwColorFloat()  {}

    RwColorFloat( float red, float green, float blue, float alpha )
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    float r,g,b,a;
};
struct RwColor
{
    RwColor()
    {
        r = g = b = a = 0;
    }
    
    RwColor( unsigned int color )
    {
        *(unsigned int*)this = color;
    }

    unsigned char r, g, b, a;

    operator unsigned int ( void ) const
    {
        return *(unsigned int*)this;
    }

    operator unsigned int& ( void )
    {
        return *(unsigned int*)this;
    }
};
enum eRwType : unsigned char
{
    RW_NULL,
    RW_ATOMIC,
    RW_CLUMP,
    RW_LIGHT,
    RW_CAMERA,
    RW_TXD = 6,
    RW_SCENE,
    RW_GEOMETRY
};
enum RpLightType : unsigned int
{
    LIGHT_TYPE_NULL = 0,
    
    LIGHT_TYPE_DIRECTIONAL = 1,
    LIGHT_TYPE_AMBIENT = 2,
    
    LIGHT_TYPE_POINT = 0x80,
    LIGHT_TYPE_SPOT_1 = 0x81,
    LIGHT_TYPE_SPOT_2 = 0x82
};

static const float negOne = -1.0f;

class RwMatrix
{   // 16-byte padded
public:
    inline void IdentityRotation()
    {
        right.fX = 1; right.fY = 0; right.fZ = 0;
        at.fX = 0; at.fY = 1; at.fZ = 0;
        up.fX = 0; up.fY = 0; up.fZ = 1;
    }

    inline void Identity()
    {
        IdentityRotation();

        pos.fX = 0;//(float)(1.15 * -0.25);
        pos.fY = 0;
        pos.fZ = 0;
    }

    RwMatrix()
    {
        Identity();
    }

    inline void assign( const RwMatrix& mat )
    {
        right = mat.right;
        up = mat.up;
        at = mat.at;
        pos = mat.pos;
    }

    RwMatrix( const RwMatrix& mat )
    {
        assign( mat );
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
        float outmt[16];
        Multiply( mat, (RwMatrix&)outmt );
        return (RwMatrix&)outmt;
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

#define DEG2RAD(x)  ( M_PI * x / 180 )
#define RAD2DEG(x)  ( x / M_PI * 180 )

    inline void FromHeading( float heading )
    {
        float ch = cos( heading );
        float sh = sin( heading );

        right[0] = ch;  right[1] = sh;  right[2] = 0;
        at[0] = -sh;    at[1] = ch;     at[2] = 0;
        up[0] = 0;      up[1] = 0;      up[2] = 1.0f;
    }

    // I have done the homework for MTA
    inline void rotXY( float x, float y )
    {
        double ch = cos( x );
        double sh = sin( x );
        double cb = cos( y );
        double sb = sin( y );

        right[0] = (float)( cb );
        right[1] = 0;
        right[2] = (float)( sb );

        at[0] = (float)( sb * sh );
        at[1] = (float)( ch );
        at[2] = (float)( -sh * cb );

        up[0] = (float)( -sb * ch );
        up[1] = (float)( sh );
        up[2] = (float)( cb * ch );
    }

    inline void SetRotationRad( float x, float y, float z )
    {
        double ch = cos( x );
        double sh = sin( x );
        double cb = cos( y );
        double sb = sin( y );
        double ca = cos( z );
        double sa = sin( z );

        right[0] = (float)( ca * cb );
        right[1] = (float)( -sa * cb );
        right[2] = (float)( sb );

        at[0] = (float)( ca * sb * sh + sa * ch );
        at[1] = (float)( ca * ch - sa * sb * sh );
        at[2] = (float)( -sh * cb );

        up[0] = (float)( sa * sh - ca * sb * ch );
        up[1] = (float)( sa * sb * ch + ca * sh );
        up[2] = (float)( ch * cb );
    }

    inline void SetRotation( float x, float y, float z )
    {
        SetRotationRad( (float)DEG2RAD( x ), (float)DEG2RAD( y ), (float)DEG2RAD( z ) );
    }

    inline void GetRotationRad( float& x, float& y, float& z ) const
    {
        if ( right[2] == 1 )
        {
            y = (float)( M_PI / 2 );

            x = 0;
            z = (float)atan2( right[0], right[1] );
        }
        else if ( right[2] == -1 )
        {
            y = -(float)( M_PI / 2 );

            x = -0;
            z = (float)atan2( right[0], right[1] );
        }
        else
        {
            y = asin( right[2] );

            x = (float)atan2( -at[2], up[2] );
            z = (float)atan2( -right[1], right[0] );
        }
    }

    inline void GetRotation( float& x, float& y, float& z ) const
    {
        GetRotationRad( x, y, z );

        x = (float)RAD2DEG( x );
        y = (float)RAD2DEG( y );
        z = (float)RAD2DEG( z );
    }

    // I hope this works :3
    inline void __thiscall Multiply( const RwMatrix& mat, RwMatrix& dst ) const
    {
	    __asm
	    {
		    mov edx,[mat]
		    mov esi,[dst]

		    movups xmm4,[edx]
		    movups xmm5,[edx+0x10]
		    movups xmm6,[edx+0x20]
    		
		    movss xmm3,[ecx+0x10]
		    movss xmm7,[ecx+0x20]

		    // X
		    movss xmm0,[ecx]
		    movss xmm1,[ecx+4]
		    movss xmm2,[ecx+8]

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
		    movss xmm1,[ecx+0x14]
		    movss xmm2,[ecx+0x18]

		    shufps xmm1,xmm1,0x40
		    shufps xmm2,xmm2,0x40

		    mulps xmm3,xmm4
		    mulps xmm1,xmm5
		    mulps xmm2,xmm6

		    addps xmm3,xmm1
		    addps xmm3,xmm2

		    movups [esi+0x10],xmm3

		    // Z
		    movss xmm1,[ecx+0x24]
		    movss xmm2,[ecx+0x28]

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

    inline void __thiscall Invert()
    {
        // Optimization to use SSE registers instead of stack space
        __asm
        {
            movups xmm0,[ecx]
            movups xmm1,[ecx+0x10]
            movups xmm2,[ecx+0x20]
            movups xmm3,[ecx+0x30]

            // Prepare for position invert
            movss xmm4,negOne

            movss [ecx],xmm0
            movss [ecx+0x04],xmm1
            movss [ecx+0x08],xmm2

            // Left-shift the vectors
            shufps xmm0,xmm0,0x49
            shufps xmm1,xmm1,0x49
            shufps xmm2,xmm2,0x49

            // Pos invert prep
            shufps xmm4,xmm4,0x40

            movss [ecx+0x10],xmm0
            movss [ecx+0x14],xmm1
            movss [ecx+0x18],xmm2

            // Left-shift the vectors
            shufps xmm0,xmm0,0x49
            shufps xmm1,xmm1,0x49
            shufps xmm2,xmm2,0x49

            movss [ecx+0x20],xmm0
            movss [ecx+0x24],xmm1
            movss [ecx+0x28],xmm2

            // Invert the position
            mulps xmm3,xmm4
            movups [ecx+0x30],xmm3
        }
    }

    inline void __thiscall Transform( const CVector& point, CVector& vout ) const
    {
	    __asm
	    {
		    mov eax,[point]
		    mov esi,[vout]

            // __thiscall makes sure that our matrix is in ecx
		    movups xmm4,[ecx]RwMatrix.right
		    movups xmm5,[ecx]RwMatrix.at
		    movups xmm6,[ecx]RwMatrix.up
		    movups xmm3,[ecx]RwMatrix.pos

            // Read the offset parameters
		    movss xmm0,[eax]CVector.fX
		    movss xmm1,[eax]CVector.fY
		    movss xmm2,[eax]CVector.fZ

            // Expand to vectors
		    shufps xmm0,xmm0,0x40
		    shufps xmm1,xmm1,0x40
		    shufps xmm2,xmm2,0x40

		    mulps xmm0,xmm4
		    mulps xmm1,xmm5
		    mulps xmm2,xmm6

		    addps xmm0,xmm1
		    addps xmm0,xmm2
		    addps xmm0,xmm3

            // Write the output
		    movss [esi]CVector.fX,xmm0
		    shufps xmm0,xmm0,0x49
		    movss [esi]CVector.fY,xmm0
		    shufps xmm0,xmm0,0x49
		    movss [esi]CVector.fZ,xmm0
	    }
    }

    float& operator [] ( unsigned int i )
    {
#ifdef _DEBUG
        assert( i < 0x10 );
#endif
        return ((float*)(this))[i];
    }

    float operator [] ( unsigned int i ) const
    {
#ifdef _DEBUG
        assert( i < 0x10 );
#endif
        return ((float*)(this))[i];
    }

    CVector         right;
    unsigned int    a;
    CVector         at;
    unsigned int    b;
    CVector         up;
    unsigned int    c;
    CVector         pos;
    unsigned int    d;
};

#endif