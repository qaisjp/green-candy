/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CObject.h
*  PURPOSE:     Object entity interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_OBJECT
#define __CGAME_OBJECT

#include "CPhysical.h"

class CVehicle;
class CPed;
class CObjectSAInterface;

class CObject : public virtual CPhysical
{
public:
    virtual                         ~CObject() {};

    virtual unsigned int            GetPoolIndex() const = 0;

    virtual void                    Explode() = 0;
    virtual void                    Break() = 0;
    virtual void                    SetScale( float fScale ) = 0;
    virtual void                    SetHealth( float fHealth ) = 0;
    virtual float                   GetHealth() const = 0;
    virtual void                    SetModelIndex( unsigned short ulModel ) = 0;

    virtual void                    SetAlpha( unsigned char ucAlpha ) = 0;
    virtual unsigned char           GetAlpha() const = 0;

    virtual bool                    IsAGangTag() const = 0;
};

#endif
