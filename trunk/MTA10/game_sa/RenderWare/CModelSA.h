/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CModelSA.h
*  PURPOSE:     DFF model management entity
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODEL_RW_
#define _MODEL_RW_

class CModelSA : public virtual CModel, public CRwObjectSA
{
    friend class CRpAtomicSA;
    friend class CRpLightSA;
    friend class CRwCameraSA;
    friend class CStreamingSA;
public:
                                    CModelSA( RpClump *clump, CColModelSA *col, unsigned short txdID );
                                    CModelSA( RpClump *clump );
                                    ~CModelSA();

    inline RpClump*                 GetObject()                     { return (RpClump*)m_object; }
    inline const RpClump*           GetObject() const               { return (const RpClump*)m_object; }

    inline CColModelSA*             GetCollision()                  { return m_col; }

    eRwType                         GetType() const                 { return RW_CLUMP; }

    const char*                     GetName() const;
    unsigned int                    GetHash() const;

    void                            Render();

    std::vector <unsigned short>    GetImportList() const;

    bool                            Replace( unsigned short id );
    bool                            IsReplaced( unsigned short id ) const;
    bool                            Restore( unsigned short id );

    void                            RestoreAll();

    const atomicList_t&             GetAtomics() const              { return m_atomics; }
    const lightList_t&              GetLights() const               { return m_lights; }
    const cameraList_t&             GetCameras() const              { return m_cameras; }

    // Static accessors
    static void RpClumpAssignObjects( CRwObjectSA *obj, CModelSA *model );

protected:
    CColModelSA*                    m_col;
    unsigned short                  m_txdID;
    bool                            m_readOnly;

    typedef std::map <unsigned short, bool> importMap_t;
    importMap_t                     m_imported;

    atomicList_t                    m_atomics;
    lightList_t                     m_lights;
    cameraList_t                    m_cameras;
};

#endif //_MODEL_RW_