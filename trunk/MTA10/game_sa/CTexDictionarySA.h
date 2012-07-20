/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTexDictionarySA.h
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

class CTexDictionarySA : public CTexDictionary
{
    friend class CTextureManagerSA;
public:
                            CTexDictionarySA( const char *name );
                            CTexDictionarySA( const char *name, CTxdInstanceSA *txd );
                            ~CTexDictionarySA();

    bool                    Load( const char *filename, bool filtering );
    void                    Clear();

    const char*             GetName() const                 { return m_name.c_str(); }
    unsigned int            GetHash() const                 { return m_tex->m_hash; }

    bool                    Import( unsigned short id );
    bool                    ImportTXD( unsigned short id );
    bool                    Remove( unsigned short id );
    bool                    RemoveTXD( unsigned short id );

    bool                    IsImported( unsigned short id ) const;
    bool                    IsImportedTXD( unsigned short id ) const;

protected:
    std::string             m_name;

    typedef std::list <unsigned short> importList_t;
    importList_t            m_imported;

public:
    typedef std::list <CTextureSA*> textureList_t;

    CTxdInstanceSA*         m_tex;
    textureList_t           m_textures;
};

#endif