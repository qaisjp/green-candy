/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRpAtomicSA.h
*  PURPOSE:     RenderWare RpAtomic export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_ATOMIC_SA_
#define _RW_ATOMIC_SA_

class CModelSA;

class CRpAtomicSA : public virtual CRpAtomic, public CRwObjectSA
{
    friend class CModelSA;
public:
                                    CRpAtomicSA( RpAtomic *atomic );
                                    ~CRpAtomicSA();

    inline RpAtomic*                GetObject()                     { return (RpAtomic*)m_object; }
    inline const RpAtomic*          GetObject() const               { return (const RpAtomic*)m_object; }

    CRpAtomic*                      Clone() const;
    RpAtomic*                       CreateInstance( unsigned short id ) const;

    void                            Render();

    void                            GetWorldSphere( RwSphere& out );

    eRwType                         GetType() const                 { return RW_ATOMIC; }
    bool                            IsFrameExtension() const        { return true; }

    void                            AddToModel( CModel *model );
    CModel*                         GetModel();
    void                            RemoveFromModel();

    bool                            Replace( unsigned short id );
    bool                            IsReplaced( unsigned short id ) const;
    bool                            Restore( unsigned short id );
    void                            RestoreAll();

    const imports_t&                GetImportList() const           { return m_imported; }

private:
    CModelSA*                       m_model;
    imports_t                       m_imported;
};

#endif //_RW_ATOMIC_SA_