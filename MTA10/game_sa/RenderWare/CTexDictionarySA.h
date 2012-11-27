/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CTexDictionarySA.h
*  PURPOSE:     Internal texture dictionary handler
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTexDictionarySA_H_
#define _CTexDictionarySA_H_

class CTextureSA;
class CTxdInstanceSA;

class CTexDictionarySA : public virtual CTexDictionary, public CRwObjectSA
{
    friend class CTextureManagerSA;
public:
                            CTexDictionarySA( RwTexDictionary *txd );
                            ~CTexDictionarySA();

    RwTexDictionary*        GetObject()                     { return (RwTexDictionary*)m_object; }
    const RwTexDictionary*  GetObject() const               { return (const RwTexDictionary*)m_object; }

    eRwType                 GetType() const                 { return RW_TXD; }

    std::list <CTexture*>&  GetTextures()                   { return (std::list <CTexture*>&)m_textures; }
    void                    Clear();

    void                    SetGlobalEmitter();

    bool                    Import( unsigned short id );
    bool                    ImportTXD( unsigned short id );
    bool                    Remove( unsigned short id );
    bool                    RemoveTXD( unsigned short id );

    void                    ClearImports();

    bool                    IsImported( unsigned short id ) const;
    bool                    IsImportedTXD( unsigned short id ) const;

    const importList_t&     GetImportedList() const         { return m_imported; }

protected:
    importList_t            m_imported;

public:
    typedef std::list <CTextureSA*> textureList_t;

    RwTexDictionary*        m_txd;
    textureList_t           m_textures;

    RwListEntry <CTexDictionarySA>  m_dicts;
};

#endif //_CTexDictionarySA_H_