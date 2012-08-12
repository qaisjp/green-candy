/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare/CRwFrame.h
*  PURPOSE:     RenderWare RwFrame modification export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_FRAME_
#define _RW_FRAME_

class CRwObject;

class CRwFrame abstract : public virtual CRwObject
{
public:
    virtual                         ~CRwFrame() {}

    virtual void                    SetName( const char *name ) = 0;
    virtual const char*             GetName() const = 0;

    typedef std::list <CRwObject*> objectList_t;

    virtual const objectList_t&     GetObjects() const = 0;
};

#endif //_RW_FRAME_