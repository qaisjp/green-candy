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

class CClientTXD : public LuaElement
{
public:
                                    CClientTXD( lua_State *L, CTexDictionary& txd );
                                    ~CClientTXD();

    const std::string               GetName() const                 { return m_txd.GetName(); }
    unsigned int                    GetHash() const                 { return m_txd.GetHash(); }
    unsigned short                  GetID() const                   { return m_txd.GetID(); }

    bool                            LoadTXD( CFile *file, bool filtering = true );

    bool                            Import( unsigned short usModelID );
    bool                            IsImported( unsigned short usModelID ) const;

    void                            Remove( unsigned short usModelID );
    void                            RemoveAll();

    CTexDictionary&         m_txd;
};

#endif
