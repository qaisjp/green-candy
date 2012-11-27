/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CTextureSA.h
*  PURPOSE:     Internal texture handler
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTextureSA_H_
#define _CTextureSA_H_

class CTexDictionarySA;

class CTextureSA : public CTexture
{
    friend class CTexDictionarySA;
    friend class CTextureManagerSA;
public:
                            CTextureSA( RwTexture *tex );
                            CTextureSA( CTexDictionarySA *txd, RwTexture *tex );
                            ~CTextureSA();

    const char*             GetName() const;
    unsigned int            GetHash() const;

    void                    SetFiltering( bool filter )                     { BOOL_FLAG( m_texture->flags, 0x1102, filter ); }
    bool                    IsFiltering() const                             { return IS_FLAG( m_texture->flags, 0x1102 ); }

    void                    SetTXD( CTexDictionary *txd );
    void                    RemoveFromTXD();
    CTexDictionary*         GetTXD();

    void                    OnTxdLoad( RwTexDictionary& txd, unsigned short id );
    void                    OnTxdInvalidate( RwTexDictionary& txd, unsigned short id );

    bool                    Import( unsigned short id );
    bool                    ImportTXD( unsigned short id );
    bool                    Remove( unsigned short id );
    bool                    RemoveTXD( unsigned short id );

    void                    ClearImports();

    bool                    IsImported( unsigned short id ) const;
    bool                    IsImportedTXD( unsigned short id ) const;

protected:
    CTexDictionarySA*       m_dictionary;
    RwTexture*              m_texture;

    struct import
    {
        RwTexture *copy;
        RwTexture *original;
    };
    
    typedef std::map <unsigned short, import> importMap_t;

    importMap_t             m_imported;
};

#endif