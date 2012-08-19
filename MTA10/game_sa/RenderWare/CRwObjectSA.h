/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwObjectSA.h
*  PURPOSE:     RenderWare RwObject export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_OBJECT_SA_
#define _RW_OBJECT_SA_

class CRwFrameSA;

class CRwObjectSA : public virtual CRwObject
{
public:
                                    CRwObjectSA( RwObject *obj );
    virtual                         ~CRwObjectSA();

    inline RwObject*                GetObject()                         { return m_object; }
    inline const RwObject*          GetObject() const                   { return m_object; }

    virtual eRwType                 GetType() const = 0;
    virtual bool                    IsFrameExtension() const            { return false; }

    virtual const char*             GetName() const;
    virtual unsigned int            GetHash() const;

    CRwFrame*                       GetFrame();
    const CRwFrame*                 GetFrame() const;
    void                            SetFrame( CRwFrame *frame );

protected:
    RwObject*           m_object;

    CRwFrameSA*         m_frame;
};

#endif //_RW_OBJECT_SA_