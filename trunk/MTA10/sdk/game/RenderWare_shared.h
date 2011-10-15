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
    RwMatrix()
    {
        Identity();
    }

    inline void Identity()
    {
        right.x = 1; right.y = 0; right.z = 0;
        up.x = 0; up.y = 1; up.z = 0;
        at.x = 0; at.y = 0; at.z = 1;

        pos.x = 1.15 * -0.25;
        pos.y = 0;
        pos.z = 0;
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