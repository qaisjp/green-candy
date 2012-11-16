/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwCameraSA.h
*  PURPOSE:     RenderWare RwCamera export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_CAMERA_SA_
#define _RW_CAMERA_SA_

class CRwCameraSA : public virtual CRwCamera, public CRwObjectSA
{
    friend class CModelSA;
public:
                                    CRwCameraSA( RwCamera *cam );
                                    ~CRwCameraSA();

    inline RwCamera*                GetObject()                                 { return (RwCamera*)m_object; }
    inline const RwCamera*          GetObject() const                           { return (const RwCamera*)m_object; }

    eRwType                         GetType() const                             { return RW_CAMERA; }
    bool                            IsFrameExtension() const                    { return true; }

    void                            AddToModel( CModel *model );
    CModel*                         GetModel();
    void                            RemoveFromModel();

    void                            SetRenderSize( int width, int height );
    void                            GetRenderSize( int& width, int& height ) const;

    bool                            BeginUpdate();
    bool                            IsRendering() const                         { return m_isRendering; }
    void                            EndUpdate();

private:
    void                            DestroyBuffers();

    CModelSA*   m_model;
    bool        m_isRendering;
};

#endif //_RW_CAMERA_SA_