/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwFrameSA.h
*  PURPOSE:     RenderWare RwFrame export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_FRAME_SA_
#define _RW_FRAME_SA_

class CRwFrameSA : public virtual CRwFrame, public CRwObjectSA
{
    friend class CRwObjectSA;
public:
                                    CRwFrameSA( RwFrame *obj );
                                    ~CRwFrameSA();

    inline RwFrame*                 GetObject()                         { return (RwFrame*)m_object; }
    inline const RwFrame*           GetObject() const                   { return (const RwFrame*)m_object; }

    void                            SetName( const char *name )         { strncpy( GetObject()->m_nodeName, name, 31 ); }
    const char*                     GetName() const                     { return GetObject()->m_nodeName; }
    unsigned int                    GetHash() const;
    eRwType                         GetType() const                     { return RW_NULL; }

    void                            Link( CRwFrame *child );
    void                            Unlink();

    objectList_t&                   GetObjects()                        { return m_objects; }
    childList_t&                    GetChildren()                       { return m_children; }

    void                            SetLTM( const RwMatrix& mat )       { GetObject()->m_ltm = mat; }
    const RwMatrix&                 GetLTM() const                      { return GetObject()->m_ltm; }

    void                            SetModelling( const RwMatrix& mat ) { GetObject()->m_modelling = mat; }
    const RwMatrix&                 GetModelling() const                { return GetObject()->m_modelling; }

    void                            SetPosition( const CVector& pos )   { GetObject()->m_modelling.pos = pos; }
    const CVector&                  GetPosition() const                 { return GetObject()->m_modelling.pos; }

    childList_t                     m_children;

    // Static accessors
    static bool RwFrameObjectAssign( RwObject *obj, CRwFrameSA *parent );

private:
    objectList_t                    m_objects;
};

#endif //_RW_FRAME_SA_