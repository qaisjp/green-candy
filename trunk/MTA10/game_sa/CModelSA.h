/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelSA.h
*  PURPOSE:     DFF model management entity
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODEL_RW_
#define _MODEL_RW_

class CModelSA : public CModel
{
public:
                            CModelSA( RpClump *clump );
                            ~CModelSA();

    const char*             GetName() const;
    unsigned int            GetHash() const;

    std::vector <unsigned short>    GetImportList() const;

    bool                    Replace( unsigned short id );
    bool                    IsReplaced( unsigned short id ) const;
    bool                    Restore( unsigned short id );

    void                    RestoreAll();

protected:
    RpClump*        m_clump;

    struct import
    {
        RpClump *original;
    };

    typedef std::map <unsigned short, import> importMap_t;
    importMap_t     m_imported;
};

#endif //_MODEL_RW_