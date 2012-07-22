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
*****************************************************************************/

#ifndef __CCLIENTTXD_H
#define __CCLIENTTXD_H

#define LUACLASS_TXD    20

class CClientTXD : public LuaElement
{
public:
                                    CClientTXD( LuaClass& root, CTexDictionary& txd );
                                    ~CClientTXD();

    const std::string               GetName() const                 { return m_txd.GetName(); }
    unsigned int                    GetHash() const                 { return m_txd.GetHash(); }
    unsigned short                  GetID() const                   { return m_txd.GetID(); }

    bool                            LoadTXD( const char *path, bool filtering = true );
    void                            UnloadTXD();

    bool                            Import( unsigned short usModelID );
    bool                            IsImported( unsigned short usModelID ) const;

    void                            Remove( unsigned short usModelID );
    void                            RemoveAll();

    CTexDictionary&         m_txd;
};

#endif
