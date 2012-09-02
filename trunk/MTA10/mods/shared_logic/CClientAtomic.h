/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientAtomic.h
*  PURPOSE:     Atomic mesh class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CLIENT_ATOMIC_
#define _CLIENT_ATOMIC_

#define LUACLASS_ATOMIC     96

class CClientDFF;

class CClientAtomic : public CClientRwObject
{
public:
                                    CClientAtomic( LuaClass& root, CClientDFF *model, CRpAtomic& atom );
                                    ~CClientAtomic();

    const char*                     GetName() const         { return m_atomic.GetName(); }
    unsigned int                    GetHash() const         { return m_atomic.GetHash(); }

protected:
    CClientDFF* m_clump;

    CRpAtomic&  m_atomic;
};

#endif //_CLIENT_ATOMIC_