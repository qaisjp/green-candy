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

class CTexDictionarySA
{
    friend class CTextureManagerSA;
public:
                            ~CTexDictionarySA();

    const char*             GetName();

    bool                    IsImported( unsigned short id );

protected:
    bool                    Import( unsigned short id );
    bool                    Remove( unsigned short id );

    typedef list <CTextureSA*> textureList_t;

    textureList_t           m_textures;
};

#endif