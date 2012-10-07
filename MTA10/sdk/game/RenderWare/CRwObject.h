/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare/CRwObject.h
*  PURPOSE:     RenderWare RwObject modification export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_OBJECT_
#define _RW_OBJECT_

class CRwFrame;

class CRwObject abstract
{
public:
    virtual                         ~CRwObject()    {}

    virtual eRwType                 GetType() const = 0;
    virtual bool                    IsFrameExtension() const = 0;

    virtual CRwFrame*               GetFrame() = 0;
    virtual const CRwFrame*         GetFrame() const = 0;
    virtual void                    SetFrame( CRwFrame *frame ) = 0;
};

#endif //_RW_OBJECT_