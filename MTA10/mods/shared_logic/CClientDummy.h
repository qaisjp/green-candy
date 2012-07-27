/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDummy.h
*  PURPOSE:     Dummy entity class
*  DEVELOPERS:  Chris McArthur <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef CClientDummy_H
#define CClientDummy_H

#include "CClientEntity.h"

#define LUACLASS_DUMMY  45

class CClientDummy : public CClientEntity
{
public:
                                CClientDummy            ( class CClientManager* pManager, ElementID ID, const char *szTypeName, LuaClass& root, bool system );
                                ~CClientDummy           ( void );
    
    void                        Unlink                  ( void );

    inline eClientEntityType    GetType                 ( void ) const                       { return CCLIENTDUMMY; }
    inline const CVector&       GetPosition             ( void )                             { return m_vecPosition; };
    inline void                 GetPosition             ( CVector& vecPosition ) const       { vecPosition = m_vecPosition; };
    inline void                 SetPosition             ( const CVector& vecPosition )       { m_vecPosition = vecPosition; };

private:
    class CClientGroups*        m_pGroups;
    CVector                     m_vecPosition;
};

#endif
