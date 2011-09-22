/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTextureSA.h
*  PURPOSE:     Internal texture handler
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTextureSA_H_
#define _CTextureSA_H_

class CTextureSA
{
    friend class CTexDictionarySA;
    friend class CTextureManagerSA;
public:
                            CTextureSA( RwTexture *tex );
                            ~CTextureSA();

    const char*             GetName();
    unsigned int            GetHash();

    bool                    IsImported( unsigned short id );

protected:
    bool                    Import( unsigned short id );
    bool                    Remove( unsigned short id );

    typedef list <CBaseModelInfoSAInterface*> importList_t;

    CTexDictionarySA*       m_dictionary;
    RwTexture*              m_texture;
    
    importList_t            m_imported;
};

#endif