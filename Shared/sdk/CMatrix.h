/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/CMatrix.h
*  PURPOSE:     4x3 GTA matrix class
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CMatrix_H
#define __CMatrix_H

#include "CVector.h"

/**
 * Contains full positional data for a point
 */
class CMatrix
{
public:
    CMatrix ( )
    {
        // Load the identity matrix
        right = CVector ( 1.0f, 0.0f, 0.0f );
        at = CVector ( 0.0f, 1.0f, 0.0f );
        up    = CVector ( 0.0f, 0.0f, 1.0f );
        pos   = CVector ( 0.0f, 0.0f, 0.0f );
    }

    CMatrix operator+ ( const CMatrix& other ) const
    {
        CMatrix matResult;
        matResult.right = right + other.right;
        matResult.at = at + other.at;
        matResult.up    = up    + other.up;
        matResult.pos   = pos   + other.pos;
        return matResult;
    }

    CMatrix operator- ( const CMatrix& other ) const
    {
        CMatrix matResult;
        matResult.right = right - other.right;
        matResult.at = at - other.at;
        matResult.up    = up    - other.up;
        matResult.pos   = pos   - other.pos;
        return matResult;
    }

    CMatrix operator* ( const CMatrix& other ) const
    {
        CMatrix matResult;
        matResult.right = (*this) * other.right;
        matResult.at = (*this) * other.at;
        matResult.up    = (*this) * other.up;
        matResult.pos   = (*this) * other.pos;
        return matResult;
    }

    CMatrix operator/ ( CMatrix other ) const
    {
        other.Invert ();
        return (*this) * other;
    }

    CVector operator* ( const CVector& vec ) const
    {
        return CVector (
            right.fX*vec.fX + at.fX*vec.fY + up.fX*vec.fZ,
            right.fY*vec.fX + at.fY*vec.fY + up.fY*vec.fZ,
            right.fZ*vec.fX + at.fZ*vec.fY + up.fZ*vec.fZ
        );
    }

    void Invert ()
    {
        // Transpose the rotation matrix and negate the position
        CVector vOldRight = right, vOldFront = at, vOldUp = up;
        right = CVector ( vOldRight.fX, vOldFront.fX, vOldUp.fX );
        at = CVector ( vOldRight.fY, vOldFront.fY, vOldUp.fY );
        up    = CVector ( vOldRight.fZ, vOldFront.fZ, vOldUp.fZ );
        pos  *= -1.0f;
    }

    CVector right;
    CVector at;
    CVector up;
    CVector pos;
};

#endif
