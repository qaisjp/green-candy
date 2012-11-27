/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGameTexture.h
*  PURPOSE:     RenderWare texture entity
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CLIENT_GAME_TEXTURE_
#define _CLIENT_GAME_TEXTURE_

#define LUACLASS_TEXTURE    21

class CClientTXD;

class CClientGameTexture : public LuaElement
{
    friend class CClientGameTexture;
public:
                                    CClientGameTexture( lua_State *L, CTexture& tex );
                                    ~CClientGameTexture();

    const std::string               GetName()               { return m_tex.GetName(); }
    unsigned int                    GetHash()               { return m_tex.GetHash(); }

    void                            SetTXD( CClientTXD *txd );
    void                            RemoveFromTXD();
    CClientTXD*                     GetTXD()                { return m_txd; }

    bool                            Import( unsigned short id );
    bool                            IsImported( unsigned short id ) const;
    void                            Remove( unsigned short id );

public:
    CTexture&               m_tex;

    CClientTXD*             m_txd;
    RwListEntry <CClientGameTexture>    m_textures;
};

#endif //_CLIENT_GAME_TEXTURE_