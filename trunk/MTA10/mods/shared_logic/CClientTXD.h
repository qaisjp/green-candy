/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientTXD.h
*  PURPOSE:     TXD manager class header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CCLIENTTXD_H
#define __CCLIENTTXD_H

#define LUACLASS_TXD    20

class CClientGameTexture;

class CClientTXD : public CClientRwObject
{
    friend class CClientGameTexture;
public:
                                    CClientTXD( lua_State *L, CTexDictionary& txd );
                                    ~CClientTXD();

    bool                            Import( unsigned short usModelID );
    bool                            IsImported( unsigned short usModelID ) const;

    void                            Remove( unsigned short usModelID );
    void                            RemoveAll();

    CTexDictionary&         m_txd;

protected:
    RwList <CClientGameTexture>     m_textures;
};

#endif
