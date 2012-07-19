/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/CMatrix_Pad.h
*  PURPOSE:     4x3 GTA padded matrix
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CMATRIX_PAD_H
#define __CMATRIX_PAD_H

#include "CVector.h"
#include "CMatrix.h"

/**
 * CVector Structure used internally by GTA.
 */
class CMatrix_Padded
{
public:
    CVector right; // 0            RIGHT
    DWORD    dwPadRoll; // 12
    CVector at; // 16   FOREWARDS
    DWORD    dwPadDirection; // 28
    CVector up; // 32          UP
    DWORD    dwPadWas; // 44    
    CVector pos;  // 48        TRANSLATE
    DWORD    dwPadPos; // 60

    CMatrix_Padded()
    {
        memset ( this, 0, sizeof ( CMatrix_Padded ) );
    }

    CMatrix_Padded ( const CMatrix& Matrix )
    {
        SetFromMatrix ( Matrix );
    }

    void ConvertToMatrix ( CMatrix& Matrix ) const
    {
        Matrix.pos = pos;
        Matrix.at = at;
        Matrix.up = up;
        Matrix.right = right;
    }

    void SetFromMatrix ( const CMatrix& Matrix )
    {
        pos = Matrix.pos;
        dwPadPos = 0;

        at = Matrix.at;
        dwPadDirection = 0;

        up = Matrix.up;
        dwPadWas = 0;

        right = Matrix.right;
        dwPadRoll = 0;
    }
};

#endif
